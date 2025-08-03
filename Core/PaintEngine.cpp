#include "PaintEngine.h"
#include <algorithm>
#include <cmath>

//==============================================================================
// PaintEngine Implementation

PaintEngine::PaintEngine()
{
    // RELIABILITY FIX: Initialize double-buffered oscillator pools
    oscillatorPools[0].resize(MAX_OSCILLATORS);
    oscillatorPools[1].resize(MAX_OSCILLATORS);
    
    // PHASE 1 OPTIMIZATION: Initialize enhanced oscillator states
    oscillatorStates.resize(MAX_OSCILLATORS);
    freeOscillatorIndices.reserve(MAX_OSCILLATORS);
    
    // Initially all oscillators are free
    for (int i = 0; i < MAX_OSCILLATORS; ++i) {
        freeOscillatorIndices.push_back(i);
    }
    
    // Set default canvas bounds for typical musical range
    setFrequencyRange(20.0f, 20000.0f);
    setCanvasRegion(-100.0f, 100.0f, -50.0f, 50.0f);
    
    // Initialize spatial grid
    const float canvasWidth = canvasRight - canvasLeft;
    const float canvasHeight = canvasTop - canvasBottom;
    spatialGrid.initialize(canvasWidth, canvasHeight);
    
    DBG("SpectralCanvas PaintEngine initialized with Phase 1 optimizations");
}

PaintEngine::~PaintEngine()
{
    releaseResources();
}

void PaintEngine::prepareToPlay(double sr, int samplesPerBlock_)
{
    sampleRate = sr;
    samplesPerBlock = samplesPerBlock_;
    
    // Initialize smoothed values
    masterGain.reset(sampleRate, 0.01); // 10ms smoothing
    masterGain.setCurrentAndTargetValue(0.7f);
    
    // RELIABILITY FIX: Reset both oscillator pools
    for (auto& pool : oscillatorPools)
    {
        for (auto& osc : pool)
        {
            osc = Oscillator{}; // Reset to default state
        }
    }
    activeOscillators.store(0);
    
    DBG("PaintEngine prepared: " << sampleRate << "Hz, " << samplesPerBlock_ << " samples");
}

void PaintEngine::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (!isActive.load())
    {
        buffer.clear();
        return;
    }
    
    const auto startTime = juce::Time::getMillisecondCounterHiRes();
    
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Clear buffer
    buffer.clear();
    
    // Get buffer pointers for efficient access
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;
    
    // RELIABILITY FIX: Lock-free buffer swap check (audio thread only)
    swapBuffersIfPending();
    
    // Update canvas oscillators based on current playhead position
    updateCanvasOscillators();
    
    // RELIABILITY FIX: Use lock-free front buffer instead of mutex
    auto& currentOscillatorPool = getFrontBuffer();
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float leftSample = 0.0f;
        float rightSample = 0.0f;
        
        int activeOscCount = 0;
        
        // Process active oscillators with enhanced states
        for (int i = 0; i < MAX_OSCILLATORS; ++i)
        {
            auto& oscState = oscillatorStates[i];
            auto& osc = currentOscillatorPool[i];
            
            if (oscState.isActive())
            {
                // Update envelope for smooth activation/deactivation
                oscState.updateEnvelope(static_cast<float>(sampleRate));
                
                // Apply parameter smoothing to prevent clicks/pops
                osc.smoothParameters();
                
                // Generate oscillator sample
                const float oscSample = osc.getSample();
                
                // Apply envelope to prevent clicks when oscillators start/stop
                const float envelopedSample = oscSample * oscState.envelopeValue;
                
                if (usePanning.load() && rightChannel != nullptr)
                {
                    // Stereo panning with smoothed pan value
                    const float panValue = 0.5f; // TODO: Use oscState.targetPan with smoothing
                    leftSample += envelopedSample * (1.0f - panValue);
                    rightSample += envelopedSample * panValue;
                }
                else
                {
                    // Mono or no panning
                    leftSample += envelopedSample;
                }
                
                osc.updatePhase(static_cast<float>(sampleRate));
                ++activeOscCount;
                
                // Update last used time for age-based replacement
                oscState.lastUsedTime = static_cast<float>(juce::Time::getMillisecondCounterHiRes());
            }
            else if (oscState.envelopePhase == EnhancedOscillatorState::EnvelopePhase::Inactive && 
                     oscState.inUse)
            {
                // PHASE 1 OPTIMIZATION: Return finished oscillators to free pool
                oscState.inUse = false;
                freeOscillatorIndices.push_back(i);
            }
        }
        
        // Apply master gain
        const float currentGain = masterGain.getNextValue();
        leftChannel[sample] = leftSample * currentGain;
        
        if (rightChannel != nullptr)
        {
            rightChannel[sample] = usePanning.load() ? (rightSample * currentGain) : leftChannel[sample];
        }
        
        // Update active oscillator count periodically
        if (sample == 0)
        {
            activeOscillators.store(activeOscCount);
        }
    }
    
    // Update performance metrics
    const auto endTime = juce::Time::getMillisecondCounterHiRes();
    const float processingTime = static_cast<float>(endTime - startTime);
    const float blockDuration = static_cast<float>(numSamples) / static_cast<float>(sampleRate) * 1000.0f;
    cpuLoad.store(processingTime / blockDuration);
    
    // Periodically optimize oscillator pool
    static int blockCounter = 0;
    if (++blockCounter % 100 == 0) // Every ~2 seconds at 44.1kHz/512 samples
    {
        optimizeOscillatorPool();
    }
}

void PaintEngine::releaseResources()
{
    // RELIABILITY FIX: No mutex needed with lock-free design
    currentStroke.reset();
    canvasRegions.clear();
    
    // Reset both oscillator pools
    for (auto& pool : oscillatorPools)
    {
        for (auto& osc : pool)
        {
            osc = Oscillator{};
        }
    }
    
    activeOscillators.store(0);
}

//==============================================================================
// Stroke Interaction API

void PaintEngine::beginStroke(Point position, float pressure, juce::Colour color)
{
    if (currentStroke != nullptr)
    {
        // End previous stroke if one was active
        endStroke();
    }
    
    currentStroke = std::make_unique<Stroke>(nextStrokeId++);
    
    StrokePoint point(position, pressure, color);
    currentStroke->addPoint(point);
    
    DBG("Stroke started at (" << position.x << ", " << position.y << ") pressure=" << pressure);
}

void PaintEngine::updateStroke(Point position, float pressure)
{
    if (currentStroke == nullptr)
    {
        // Auto-start stroke if none active
        beginStroke(position, pressure);
        return;
    }
    
    StrokePoint point(position, pressure, juce::Colours::white);
    currentStroke->addPoint(point);
    
    // PHASE 1 OPTIMIZATION: Use incremental updates instead of full recalculation
    updateOscillatorsIncremental(point);
}

void PaintEngine::endStroke()
{
    if (currentStroke == nullptr)
        return;
    
    currentStroke->finalize();
    
    // Add stroke to appropriate canvas regions
    const auto& points = currentStroke->getPoints();
    if (!points.empty())
    {
        for (const auto& point : points)
        {
            auto* region = getOrCreateRegion(point.position.x, point.position.y);
            if (region != nullptr)
            {
                region->addStroke(std::move(currentStroke));
                break; // Only add to first region for now
            }
        }
    }
    
    currentStroke.reset();
    
    DBG("Stroke ended and added to canvas");
}

//==============================================================================
// Canvas Control

void PaintEngine::setPlayheadPosition(float normalisedPosition)
{
    playheadPosition = juce::jlimit(0.0f, 1.0f, normalisedPosition);
}

void PaintEngine::setCanvasRegion(float leftX, float rightX, float bottomY, float topY)
{
    canvasLeft = leftX;
    canvasRight = rightX;
    canvasBottom = bottomY;
    canvasTop = topY;
}

void PaintEngine::clearCanvas()
{
    // RELIABILITY FIX: Use lock-free buffer swap for clearing
    auto& backBuffer = getBackBuffer();
    
    currentStroke.reset();
    canvasRegions.clear();
    
    // Reset all oscillators in back buffer
    for (auto& osc : backBuffer)
    {
        osc.reset();
    }
    
    activeOscillators.store(0);
    
    // Request buffer swap to apply changes
    requestBufferSwap();
    
    DBG("Canvas cleared");
}

void PaintEngine::clearRegion(const juce::Rectangle<float>& region)
{
    // TODO: Implement selective region clearing
    juce::ignoreUnused(region);
}

void PaintEngine::setMasterGain(float gain)
{
    masterGain.setTargetValue(juce::jlimit(0.0f, 2.0f, gain));
}

void PaintEngine::setFrequencyRange(float minHz, float maxHz)
{
    minFrequency = juce::jlimit(1.0f, 20000.0f, minHz);
    maxFrequency = juce::jlimit(minFrequency + 1.0f, 22000.0f, maxHz);
}

//==============================================================================
// Canvas Mapping Functions

float PaintEngine::canvasYToFrequency(float y) const
{
    // Normalize Y to 0-1 range
    const float normalizedY = (y - canvasBottom) / (canvasTop - canvasBottom);
    const float clampedY = juce::jlimit(0.0f, 1.0f, normalizedY);
    
    if (useLogFrequencyScale)
    {
        // Logarithmic frequency mapping (more musical)
        const float logMin = std::log(minFrequency);
        const float logMax = std::log(maxFrequency);
        return std::exp(logMin + clampedY * (logMax - logMin));
    }
    else
    {
        // Linear frequency mapping
        return minFrequency + clampedY * (maxFrequency - minFrequency);
    }
}

float PaintEngine::frequencyToCanvasY(float frequency) const
{
    const float clampedFreq = juce::jlimit(minFrequency, maxFrequency, frequency);
    
    float normalizedY;
    if (useLogFrequencyScale)
    {
        const float logMin = std::log(minFrequency);
        const float logMax = std::log(maxFrequency);
        const float logFreq = std::log(clampedFreq);
        normalizedY = (logFreq - logMin) / (logMax - logMin);
    }
    else
    {
        normalizedY = (clampedFreq - minFrequency) / (maxFrequency - minFrequency);
    }
    
    return canvasBottom + normalizedY * (canvasTop - canvasBottom);
}

float PaintEngine::canvasXToTime(float x) const
{
    // Simple linear mapping for now
    const float normalizedX = (x - canvasLeft) / (canvasRight - canvasLeft);
    return juce::jlimit(0.0f, 1.0f, normalizedX);
}

float PaintEngine::timeToCanvasX(float time) const
{
    const float clampedTime = juce::jlimit(0.0f, 1.0f, time);
    return canvasLeft + clampedTime * (canvasRight - canvasLeft);
}

//==============================================================================
// Private Methods

void PaintEngine::updateCanvasOscillators()
{
    // Update oscillators based on current playhead position and active strokes
    const float currentTime = canvasXToTime(playheadPosition * (canvasRight - canvasLeft) + canvasLeft);
    
    // RELIABILITY FIX: Use lock-free front buffer
    auto& currentOscillatorPool = getFrontBuffer();
    
    // Process current stroke if active
    if (currentStroke != nullptr)
    {
        currentStroke->updateOscillators(currentTime, currentOscillatorPool);
    }
    
    // Process stored canvas regions
    for (auto& [key, region] : canvasRegions)
    {
        if (region != nullptr && !region->isEmpty())
        {
            region->updateOscillators(currentTime, currentOscillatorPool);
        }
    }
}

juce::int64 PaintEngine::getRegionKey(int regionX, int regionY) const
{
    return (static_cast<juce::int64>(regionX) << 32) | static_cast<juce::int64>(regionY);
}

PaintEngine::CanvasRegion* PaintEngine::getOrCreateRegion(float canvasX, float canvasY)
{
    const int regionX = static_cast<int>(std::floor(canvasX / CanvasRegion::REGION_SIZE));
    const int regionY = static_cast<int>(std::floor(canvasY / CanvasRegion::REGION_SIZE));
    const juce::int64 key = getRegionKey(regionX, regionY);
    
    auto it = canvasRegions.find(key);
    if (it != canvasRegions.end())
    {
        return it->second.get();
    }
    
    // Create new region
    auto newRegion = std::make_unique<CanvasRegion>(regionX, regionY);
    auto* regionPtr = newRegion.get();
    canvasRegions[key] = std::move(newRegion);
    
    return regionPtr;
}

void PaintEngine::cullInactiveRegions()
{
    auto it = canvasRegions.begin();
    while (it != canvasRegions.end())
    {
        if (it->second->isEmpty())
        {
            it = canvasRegions.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

PaintEngine::AudioParams PaintEngine::strokePointToAudioParams(const StrokePoint& point) const
{
    AudioParams params;
    
    params.frequency = canvasYToFrequency(point.position.y);
    params.amplitude = point.pressure; // Direct mapping for now
    params.time = canvasXToTime(point.position.x);
    
    // Extract pan from color hue
    if (point.color != juce::Colours::transparentBlack)
    {
        params.pan = point.color.getHue();
    }
    else
    {
        params.pan = 0.5f; // Center
    }
    
    return params;
}

void PaintEngine::updateCPULoad()
{
    // CPU load is updated in processBlock()
}

void PaintEngine::optimizeOscillatorPool()
{
    // RELIABILITY FIX: Optimize using back buffer and swap
    auto& backBuffer = getBackBuffer();
    
    // Reset inactive oscillators to improve cache performance
    int compactIndex = 0;
    for (int i = 0; i < MAX_OSCILLATORS; ++i)
    {
        if (backBuffer[i].isActive())
        {
            if (compactIndex != i)
            {
                backBuffer[compactIndex] = backBuffer[i];
                backBuffer[i].reset();
            }
            ++compactIndex;
        }
    }
    
    requestBufferSwap();
}

//==============================================================================
// Oscillator Implementation

void PaintEngine::Oscillator::setParameters(const AudioParams& params)
{
    frequency = params.frequency;
    targetAmplitude = juce::jlimit(0.0f, 1.0f, params.amplitude);
    targetPan = juce::jlimit(0.0f, 1.0f, params.pan);
}

void PaintEngine::Oscillator::updatePhase(float sampleRate)
{
    phaseIncrement = frequency / sampleRate;
    phase += phaseIncrement;
    
    // Wrap phase to [0, 1)
    if (phase >= 1.0f)
        phase -= std::floor(phase);
}

float PaintEngine::Oscillator::getSample() const
{
    return std::sin(phase * juce::MathConstants<float>::twoPi) * amplitude;
}

void PaintEngine::Oscillator::smoothParameters(float smoothingFactor)
{
    // PHASE 1 OPTIMIZATION: Enhanced parameter smoothing with individual rates
    // Use the smoothing factor as a base, but apply individual smoothing rates
    amplitude += (targetAmplitude - amplitude) * (smoothingFactor * 2.0f); // Faster amplitude changes
    pan += (targetPan - pan) * (smoothingFactor * 1.0f); // Moderate pan changes
    
    // Smooth frequency changes to prevent clicks (slower than amplitude)  
    // Note: We need to add targetFrequency to the Oscillator class for this to work
    // For now, frequency changes are handled in the enhanced state
}

//==============================================================================
// Stroke Implementation

PaintEngine::Stroke::Stroke(juce::uint32 id) 
    : strokeId(id)
{
    points.reserve(256); // Reserve space for typical stroke
}

void PaintEngine::Stroke::addPoint(const StrokePoint& point)
{
    points.push_back(point);
    updateBounds();
}

void PaintEngine::Stroke::finalize()
{
    isFinalized = true;
    updateBounds();
}

void PaintEngine::Stroke::updateOscillators(float currentTime, std::vector<Oscillator>& oscillatorPool)
{
    // Simple implementation: activate oscillators for visible points
    juce::ignoreUnused(currentTime, oscillatorPool);
    
    // TODO: Implement sophisticated stroke-to-oscillator mapping
}

void PaintEngine::Stroke::updateBounds()
{
    if (points.empty())
    {
        bounds = juce::Rectangle<float>();
        return;
    }
    
    float minX = points[0].position.x;
    float maxX = minX;
    float minY = points[0].position.y;
    float maxY = minY;
    
    for (const auto& point : points)
    {
        minX = std::min(minX, point.position.x);
        maxX = std::max(maxX, point.position.x);
        minY = std::min(minY, point.position.y);
        maxY = std::max(maxY, point.position.y);
    }
    
    bounds = juce::Rectangle<float>(minX, minY, maxX - minX, maxY - minY);
}

bool PaintEngine::Stroke::hasActiveOscillators() const
{
    // TODO: Implement proper oscillator tracking
    return !points.empty() && !isFinalized;
}

//==============================================================================
// CanvasRegion Implementation

PaintEngine::CanvasRegion::CanvasRegion(int regionX_, int regionY_)
    : regionX(regionX_), regionY(regionY_)
{
    strokes.reserve(16); // Reserve space for typical region
}

void PaintEngine::CanvasRegion::addStroke(std::shared_ptr<Stroke> stroke)
{
    if (stroke != nullptr)
    {
        strokes.push_back(std::move(stroke));
    }
}

void PaintEngine::CanvasRegion::removeStroke(juce::uint32 strokeId)
{
    strokes.erase(
        std::remove_if(strokes.begin(), strokes.end(),
            [strokeId](const std::shared_ptr<Stroke>& stroke) {
                return stroke->getId() == strokeId;
            }),
        strokes.end());
}

void PaintEngine::CanvasRegion::updateOscillators(float currentTime, std::vector<Oscillator>& oscillatorPool)
{
    for (auto& stroke : strokes)
    {
        if (stroke != nullptr && stroke->isActive())
        {
            stroke->updateOscillators(currentTime, oscillatorPool);
        }
    }
}

//==============================================================================
// PHASE 1 OPTIMIZATIONS: Sub-10ms Latency Paint-to-Audio Pipeline

void PaintEngine::updateOscillatorsIncremental(const StrokePoint& newPoint)
{
    // Convert stroke point to audio parameters
    AudioParams params = strokePointToAudioParams(newPoint);
    
    // Use spatial grid to find nearby oscillators that should be affected
    auto nearbyOscillators = spatialGrid.getNearbyOscillators(
        newPoint.position.x, newPoint.position.y, canvasLeft, canvasBottom);
    
    // If no nearby oscillators or we need a new one, allocate one
    if (nearbyOscillators.empty() || shouldAllocateNewOscillator(newPoint))
    {
        int oscillatorIndex = allocateOscillator();
        if (oscillatorIndex >= 0)
        {
            activateOscillator(oscillatorIndex, params);
            assignOscillatorToGrid(oscillatorIndex, newPoint.position.x, newPoint.position.y);
        }
    }
    else
    {
        // Update nearby oscillators with influence based on distance
        for (int oscIndex : nearbyOscillators)
        {
            updateOscillatorWithInfluence(oscIndex, newPoint, params);
        }
    }
}

bool PaintEngine::shouldAllocateNewOscillator(const StrokePoint& newPoint) const
{
    // Allocate new oscillators for points with significant pressure or 
    // when we have free oscillators available
    return (newPoint.pressure > 0.1f && !freeOscillatorIndices.empty()) ||
           (newPoint.pressure > 0.5f); // Always allocate for high pressure
}

int PaintEngine::allocateOscillator()
{
    // Try to get a free oscillator first
    if (!freeOscillatorIndices.empty())
    {
        int index = freeOscillatorIndices.back();
        freeOscillatorIndices.pop_back();
        return index;
    }
    
    // If pool is full, find the best oscillator to replace (age-based)
    return findBestOscillatorForReplacement();
}

int PaintEngine::findBestOscillatorForReplacement() const
{
    int oldestIndex = -1;
    float oldestTime = std::numeric_limits<float>::max();
    
    for (int i = 0; i < MAX_OSCILLATORS; ++i)
    {
        const auto& state = oscillatorStates[i];
        
        // Prefer oscillators in release phase or inactive
        if (state.envelopePhase == EnhancedOscillatorState::EnvelopePhase::Release ||
            state.envelopePhase == EnhancedOscillatorState::EnvelopePhase::Inactive)
        {
            return i;
        }
        
        // Otherwise find the least recently used
        if (state.lastUsedTime < oldestTime)
        {
            oldestTime = state.lastUsedTime;
            oldestIndex = i;
        }
    }
    
    return oldestIndex;
}

void PaintEngine::activateOscillator(int index, const AudioParams& params)
{
    if (index < 0 || index >= MAX_OSCILLATORS) return;
    
    auto& state = oscillatorStates[index];
    auto& osc = getBackBuffer()[index];
    
    // Set up enhanced state
    state.activate();
    state.targetFrequency = params.frequency;
    state.targetAmplitude = params.amplitude;
    state.targetPan = params.pan;
    state.lastUsedTime = static_cast<float>(juce::Time::getMillisecondCounterHiRes());
    
    // Set oscillator parameters with smoothing
    osc.setParameters(params);
    
    DBG("SpectralCanvas: Activated oscillator " << index << " freq=" << params.frequency << "Hz");
}

void PaintEngine::releaseOscillator(int index)
{
    if (index < 0 || index >= MAX_OSCILLATORS) return;
    
    auto& state = oscillatorStates[index];
    state.release();
    
    // Don't immediately return to free pool - let envelope finish
    // The envelope system will mark it as free when release is complete
}

void PaintEngine::updateOscillatorWithInfluence(int oscillatorIndex, const StrokePoint& newPoint, const AudioParams& params)
{
    if (oscillatorIndex < 0 || oscillatorIndex >= MAX_OSCILLATORS) return;
    
    auto& state = oscillatorStates[oscillatorIndex];
    auto& osc = getBackBuffer()[oscillatorIndex];
    
    if (!state.isActive()) return;
    
    // Calculate influence based on distance and pressure
    float distance = calculateDistance(oscillatorIndex, newPoint.position);
    float influence = calculateInfluence(distance, newPoint.pressure);
    
    if (influence < 0.01f) return; // Skip very small influences
    
    // Blend parameters based on influence
    state.targetFrequency = juce::jlimit(20.0f, 20000.0f, 
        state.targetFrequency * (1.0f - influence) + params.frequency * influence);
    
    state.targetAmplitude = juce::jlimit(0.0f, 1.0f,
        state.targetAmplitude * (1.0f - influence) + params.amplitude * influence);
    
    state.targetPan = juce::jlimit(0.0f, 1.0f,
        state.targetPan * (1.0f - influence) + params.pan * influence);
    
    // Update oscillator with smoothed parameters
    AudioParams blendedParams;
    blendedParams.frequency = state.targetFrequency;
    blendedParams.amplitude = state.targetAmplitude;
    blendedParams.pan = state.targetPan;
    
    osc.setParameters(blendedParams);
    state.lastUsedTime = static_cast<float>(juce::Time::getMillisecondCounterHiRes());
}

float PaintEngine::calculateDistance(int oscillatorIndex, const Point& position) const
{
    // For now, estimate oscillator position from its frequency
    // TODO: Store actual oscillator positions for accurate distance calculation
    
    const auto& osc = getFrontBuffer()[oscillatorIndex];
    float oscY = frequencyToCanvasY(oscillatorStates[oscillatorIndex].targetFrequency);
    
    // Simple 2D distance in canvas space
    float dx = position.x; // Assume oscillator is at time=0 for simplicity
    float dy = position.y - oscY;
    
    return std::sqrt(dx * dx + dy * dy);
}

float PaintEngine::calculateInfluence(float distance, float pressure) const
{
    // Influence decreases with distance, increases with pressure
    float normalizedDistance = distance / INFLUENCE_RADIUS;
    float influence = pressure * std::exp(-normalizedDistance * normalizedDistance);
    
    return juce::jlimit(0.0f, 1.0f, influence);
}

void PaintEngine::assignOscillatorToGrid(int oscillatorIndex, float x, float y)
{
    int cellIndex = spatialGrid.getCellIndex(x, y, canvasLeft, canvasBottom);
    spatialGrid.oscillatorIndices[cellIndex].push_back(oscillatorIndex);
}

void PaintEngine::rebuildSpatialGrid()
{
    spatialGrid.clearGrid();
    
    for (int i = 0; i < MAX_OSCILLATORS; ++i)
    {
        if (oscillatorStates[i].isActive())
        {
            // Estimate position from frequency (simplified)
            float y = frequencyToCanvasY(oscillatorStates[i].targetFrequency);
            float x = canvasLeft; // Simplified - assume all at left edge
            
            assignOscillatorToGrid(i, x, y);
        }
    }
    
    DBG("SpectralCanvas: Rebuilt spatial grid with active oscillators");
}

//==============================================================================
// RELIABILITY FIX: Lock-free buffer management implementation

void PaintEngine::swapBuffersIfPending()
{
    // CRITICAL: Called from audio thread only - must be lock-free!
    if (bufferSwapPending.load(std::memory_order_acquire))
    {
        // Swap the buffer indices atomically
        int currentFront = frontBufferIndex.load(std::memory_order_relaxed);
        int currentBack = backBufferIndex.load(std::memory_order_relaxed);
        
        // Perform the swap with memory barriers for reliability
        std::atomic_thread_fence(std::memory_order_seq_cst);
        frontBufferIndex.store(currentBack, std::memory_order_release);
        backBufferIndex.store(currentFront, std::memory_order_release);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        
        // Clear the swap pending flag
        bufferSwapPending.store(false, std::memory_order_release);
        
        DBG("PaintEngine: Buffer swap completed (audio thread)");
    }
}

void PaintEngine::requestBufferSwap()
{
    // CRITICAL: Called from GUI thread to request buffer swap
    // Only request if no swap is already pending to prevent conflicts
    bool expected = false;
    if (bufferSwapPending.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
    {
        DBG("PaintEngine: Buffer swap requested (GUI thread)");
    }
    else
    {
        DBG("PaintEngine: Buffer swap already pending, skipping request");
    }
}