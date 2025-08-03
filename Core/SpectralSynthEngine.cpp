/******************************************************************************
 * File: SpectralSynthEngine.cpp
 * Description: Implementation of master synthesis coordinator
 * 
 * Revolutionary synthesis engine that unifies paint-to-audio, sample-based,
 * tracker, and granular synthesis with real-time spectral canvas integration.
 * 
 * Copyright (c) 2025 Spectral Audio Systems
 ******************************************************************************/

#include "SpectralSynthEngine.h"
#include "SampleMaskingEngine.h"
#include "SecretSauceEngine.h"
#include "LinearTrackerEngine.h"
#include "EMURomplerEngine.h"
#include "CEM3389Filter.h"  // SECRET: E-mu Audity filter
// #include "ForgeProcessor.h"  // TODO: Enable when ForgeProcessor is built
// #include "GrainPool.h"  // TODO: Implement GrainPool
#include "Commands.h"
#include <cmath>

//==============================================================================
// Constructor - Initialize Revolutionary Synthesis System
//==============================================================================

SpectralSynthEngine::SpectralSynthEngine()
    : forwardFFT(10)  // 1024 point FFT
    , window(1024, juce::dsp::WindowingFunction<float>::hann)
{
    // Initialize synthesis engines
    sampleMaskingEngine = std::make_unique<SampleMaskingEngine>();
    secretSauceEngine = std::make_unique<SecretSauceEngine>();
    linearTrackerEngine = std::make_unique<LinearTrackerEngine>();
    emuRomplerEngine = std::make_unique<EMURomplerEngine>();
    // forgeProcessor = std::make_unique<ForgeProcessor>();  // TODO: Enable when ForgeProcessor is built
    // grainPool = std::make_unique<GrainPool>();  // TODO: Implement GrainPool
    
    // SECRET: Initialize invisible E-mu Audity filter
    secretAudityFilter = std::make_unique<CEM3389Filter>();
    
    // Initialize FFT data
    fftData.resize(1024 * 2, 0.0f);
    windowedData.resize(1024, 0.0f);
    
    // Initialize spectral analysis
    currentAnalysis.frequencyBins.resize(512, 0.0f);
    currentAnalysis.amplitudeLevels.resize(512, 0.0f);
    currentAnalysis.spectralColors.resize(512, juce::Colours::black);
    currentAnalysis.pixelatedSpectrum.resize(32, 0);
    
    // Initialize spectral oscillators
    for (auto& oscillator : spectralOscillators)
    {
        oscillator.reset();
    }
    
    // Setup preset directory
    presetDirectory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                     .getChildFile("SpectralCanvasPro")
                     .getChildFile("SynthPresets");
    
    if (!presetDirectory.exists())
        presetDirectory.createDirectory();
}

SpectralSynthEngine::~SpectralSynthEngine()
{
    releaseResources();
}

//==============================================================================
// Audio Processing Lifecycle
//==============================================================================

void SpectralSynthEngine::prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels)
{
    currentSampleRate = sampleRate;
    currentSamplesPerBlock = samplesPerBlock;
    currentNumChannels = numChannels;
    
    // Prepare all synthesis engines
    if (sampleMaskingEngine)
        sampleMaskingEngine->prepareToPlay(sampleRate, samplesPerBlock, numChannels);
    
    if (secretSauceEngine)
        secretSauceEngine->prepareToPlay(sampleRate, samplesPerBlock, numChannels);
    
    if (linearTrackerEngine)
        linearTrackerEngine->prepareToPlay(sampleRate, samplesPerBlock, numChannels);
    
    if (emuRomplerEngine)
        emuRomplerEngine->prepareToPlay(sampleRate, samplesPerBlock, numChannels);
    
    // SECRET: Prepare invisible Audity filter
    if (secretAudityFilter)
        secretAudityFilter->setSampleRate(sampleRate);
    
    // if (forgeProcessor)  // TODO: Enable when ForgeProcessor is built
    //     forgeProcessor->prepareToPlay(sampleRate, samplesPerBlock);
    
    // Initialize spectral oscillators
    for (auto& oscillator : spectralOscillators)
    {
        oscillator.phase = 0.0f;
    }
    
    // Reset performance metrics
    currentMetrics = PerformanceMetrics{};
    lastProcessTime = juce::Time::getCurrentTime();
}

void SpectralSynthEngine::processBlock(juce::AudioBuffer<float>& buffer)
{
    auto startTime = juce::Time::getCurrentTime();
    
    // DEBUG: Log processing state
    static int debugCounter = 0;
    if (debugCounter % 1000 == 0) // Log every ~23 seconds at 44.1kHz/512 samples
    {
        DBG("=== SpectralSynthEngine::processBlock ===");
        DBG("Active Oscillators: " << activeOscillatorCount.load());
        DBG("Current Synth Mode: " << static_cast<int>(currentSynthMode.load()));
        DBG("Buffer: " << buffer.getNumChannels() << " channels, " << buffer.getNumSamples() << " samples");
        DBG("Spectral Oscillators Enabled: " << (spectralOscillatorsEnabled.load() ? "YES" : "NO"));
    }
    debugCounter++;
    
    // Clear buffer
    buffer.clear();
    
    //==============================================================================
    // Stage 1: Process Individual Synthesis Engines
    
    // Create temporary buffers for each engine
    juce::AudioBuffer<float> paintSynthBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    juce::AudioBuffer<float> spectralOscBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    juce::AudioBuffer<float> trackerBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    juce::AudioBuffer<float> granularBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    juce::AudioBuffer<float> sampleBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    juce::AudioBuffer<float> emuBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    
    paintSynthBuffer.clear();
    spectralOscBuffer.clear();
    trackerBuffer.clear();
    granularBuffer.clear();
    sampleBuffer.clear();
    
    // Process paint synthesis (SampleMaskingEngine)
    if (paintSynthesisEnabled.load() && sampleMaskingEngine)
    {
        sampleMaskingEngine->processBlock(paintSynthBuffer);
    }
    
    // Process spectral oscillators
    if (spectralOscillatorsEnabled.load())
    {
        processSpectralOscillators(spectralOscBuffer);
        
        // DEBUG: Check if oscillators are generating audio
        if (debugCounter % 1000 == 0 && activeOscillatorCount.load() > 0)
        {
            float maxSample = 0.0f;
            for (int channel = 0; channel < spectralOscBuffer.getNumChannels(); ++channel)
            {
                auto* channelData = spectralOscBuffer.getReadPointer(channel);
                for (int i = 0; i < spectralOscBuffer.getNumSamples(); ++i)
                {
                    maxSample = std::max(maxSample, std::abs(channelData[i]));
                }
            }
            DBG("Spectral Oscillators Max Sample: " << maxSample);
        }
    }
    
    // Process tracker sequencing
    if (trackerSequencingEnabled.load() && linearTrackerEngine)
    {
        linearTrackerEngine->processBlock(trackerBuffer);
    }
    
    // Process EMU Audity synthesis (the legendary sauce!)
    if (emuRomplerEngine)
    {
        juce::MidiBuffer emptyMidiBuffer; // EMU will be controlled by paint, not MIDI for now
        emuRomplerEngine->processBlock(emuBuffer, emptyMidiBuffer);
    }
    
    // Process sample synthesis (ForgeProcessor)
    // if (sampleSynthesisEnabled.load() && forgeProcessor)  // TODO: Enable when ForgeProcessor is built
    // {
    //     juce::MidiBuffer emptyMidi;
    //     forgeProcessor->processBlock(sampleBuffer, emptyMidi);
    // }
    
    //==============================================================================
    // Stage 2: Mix Synthesis Engines Based on Current Mode
    
    mixSynthesisEngines(buffer, paintSynthBuffer, spectralOscBuffer, 
                       trackerBuffer, granularBuffer, sampleBuffer, emuBuffer);
    
    // DEBUG: Check if mixing produced audio
    if (debugCounter % 1000 == 0)
    {
        float maxMixedSample = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getReadPointer(channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                maxMixedSample = std::max(maxMixedSample, std::abs(channelData[i]));
            }
        }
        DBG("After Mixing Max Sample: " << maxMixedSample);
    }
    
    //==============================================================================
    // Stage 3: Apply Spectral Processing (CDP-inspired)
    
    if (currentSpectralProcess.load() != SpectralProcessType::None)
    {
        applySpectralProcessingToBuffer(buffer);
    }
    
    //==============================================================================
    // Stage 4: Final Enhancement (SecretSauceEngine)
    
    if (secretSauceEngine)
    {
        float intensity = 0.7f; // TODO: Make this configurable
        secretSauceEngine->applySecretSauce(buffer, intensity);
    }
    
    //==============================================================================
    // Stage 5: Spectral Analysis for UI Feedback
    
    if (spectralAnalysisEnabled.load())
    {
        performSpectralAnalysis(buffer);
    }
    
    //==============================================================================
    // Stage 6: Update Performance Metrics
    
    auto endTime = juce::Time::getCurrentTime();
    auto processingTime = endTime - startTime;
    
    currentMetrics.synthesisLatency = static_cast<float>(processingTime.inMilliseconds());
    currentMetrics.activeOscillators = activeOscillatorCount.load();
    currentMetrics.activePaintStrokes = static_cast<int>(activePaintStrokes.size());
    
    // Calculate CPU usage as percentage of available time
    double availableTime = (1000.0 * currentSamplesPerBlock) / currentSampleRate;
    currentMetrics.cpuUsage = static_cast<float>((processingTime.inMilliseconds() / availableTime) * 100.0);
    
    //==============================================================================
    // Stage 7: SECRET SAUCE - E-mu Audity Filter (INVISIBLE TO USER)
    //==============================================================================
    
    if (secretAudityFilter)
    {
        // Apply the secret CEM3389 filter for magical character
        secretAudityFilter->processBlock(buffer);
    }
    
    // DEBUG: Final output check
    if (debugCounter % 1000 == 0)
    {
        float finalMaxSample = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getReadPointer(channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                finalMaxSample = std::max(finalMaxSample, std::abs(channelData[i]));
            }
        }
        DBG("FINAL OUTPUT Max Sample: " << finalMaxSample);
        DBG("=== End SpectralSynthEngine::processBlock ===");
    }
}

void SpectralSynthEngine::releaseResources()
{
    if (sampleMaskingEngine)
        sampleMaskingEngine->releaseResources();
    
    if (secretSauceEngine)
        secretSauceEngine->releaseResources();
    
    if (linearTrackerEngine)
        linearTrackerEngine->releaseResources();
    
    // Clear all active states
    clearAllSpectralOscillators();
    activePaintStrokes.clear();
    
    currentMetrics = PerformanceMetrics{};
}

//==============================================================================
// Synthesis Mode Control
//==============================================================================

void SpectralSynthEngine::setSynthMode(SynthMode mode)
{
    currentSynthMode.store(mode);
    
    // Configure engine enables based on mode
    switch (mode)
    {
    case SynthMode::PaintSynthesis:
        enablePaintSynthesis(true);
        enableSpectralOscillators(false);
        enableTrackerSequencing(false);
        enableGranularSynthesis(false);
        enableSampleSynthesis(false);
        break;
        
    case SynthMode::SpectralOscillators:
        enablePaintSynthesis(false);
        enableSpectralOscillators(true);
        enableTrackerSequencing(false);
        enableGranularSynthesis(false);
        enableSampleSynthesis(false);
        break;
        
    case SynthMode::TrackerSequencing:
        enablePaintSynthesis(false);
        enableSpectralOscillators(false);
        enableTrackerSequencing(true);
        enableGranularSynthesis(false);
        enableSampleSynthesis(false);
        break;
        
    case SynthMode::SampleSynthesis:
        enablePaintSynthesis(false);
        enableSpectralOscillators(false);
        enableTrackerSequencing(false);
        enableGranularSynthesis(false);
        enableSampleSynthesis(true);
        break;
        
    case SynthMode::EMUAudityMode:
        // Pure EMU Audity legendary sound mode
        enablePaintSynthesis(false);
        enableSpectralOscillators(false);
        enableTrackerSequencing(false);
        enableGranularSynthesis(false);
        enableSampleSynthesis(false);
        break;
        
    case SynthMode::HybridSynthesis:
        enablePaintSynthesis(true);
        enableSpectralOscillators(true);
        enableTrackerSequencing(true);
        enableGranularSynthesis(true);
        enableSampleSynthesis(true);
        break;
    }
}

void SpectralSynthEngine::enablePaintSynthesis(bool enable)
{
    paintSynthesisEnabled.store(enable);
}

void SpectralSynthEngine::enableSpectralOscillators(bool enable)
{
    spectralOscillatorsEnabled.store(enable);
}

void SpectralSynthEngine::enableTrackerSequencing(bool enable)
{
    trackerSequencingEnabled.store(enable);
}

void SpectralSynthEngine::enableGranularSynthesis(bool enable)
{
    granularSynthesisEnabled.store(enable);
}

void SpectralSynthEngine::enableSampleSynthesis(bool enable)
{
    sampleSynthesisEnabled.store(enable);
}

//==============================================================================
// Real-time Brush Integration (Secret Sauce Connection)

void SpectralSynthEngine::updateBrushCharacteristics(float pressure, float velocity, juce::Colour color)
{
    // Pass brush characteristics to SecretSauceEngine for real-time tube control
    if (secretSauceEngine)
    {
        secretSauceEngine->updateTubeCharacteristicsFromBrush(pressure, velocity, color);
    }
    
    // You could also pass this information to other engines here
    // For example, update SampleMaskingEngine intensity based on pressure
    if (sampleMaskingEngine)
    {
        // Use pressure to control masking intensity or other parameters
        // sampleMaskingEngine->setPressureIntensity(pressure);
    }
}

//==============================================================================
// Spectral Canvas Integration - The Revolutionary Core
//==============================================================================

void SpectralSynthEngine::processPaintStroke(const PaintData& paintData)
{
    juce::ScopedLock lock(paintStrokeLock);
    
    // Convert paint data to synthesis parameters
    PaintData processedPaint = paintData;
    
    // Convert normalized coordinates to synthesis parameters
    processedPaint.frequencyHz = freqNormToHz(paintData.freqNorm);
    processedPaint.amplitude = paintData.pressure;
    
    // Map color to pan position (hue to stereo field)
    float hue = paintData.color.getHue();
    processedPaint.panPosition = (hue - 0.5f) * 2.0f; // -1.0 to 1.0
    
    //==============================================================================
    // EMU Audity Paint Control - The Revolutionary Sauce!
    //==============================================================================
    
    if (emuRomplerEngine)
    {
        // X-axis (timeNorm) → Filter cutoff frequency (classic swept filter control)
        float filterCutoff = paintData.timeNorm;  // 0.0-1.0 maps to full cutoff range
        
        // Y-axis (freqNorm) → Filter resonance (signature EMU self-oscillation)
        float filterResonance = paintData.freqNorm;  // 0.0-1.0 resonance amount
        
        // Pressure → Filter envelope depth + vintage character
        float envelopeDepth = paintData.pressure;    // How much envelope affects filter
        float vintageAmount = paintData.pressure * 0.8f;  // 39kHz character intensity
        
        // Color hue → Sample selection + processing mode
        float colorHue = paintData.color.getHue();
        float colorSaturation = paintData.color.getSaturation();
        float colorBrightness = paintData.color.getBrightness();
        
        // Set EMU filter parameters for legendary CEM3389 character
        emuRomplerEngine->setFilterCutoff(filterCutoff);
        emuRomplerEngine->setFilterResonance(filterResonance);
        emuRomplerEngine->setFilterTracking(envelopeDepth);  // Keyboard tracking as envelope depth
        
        // Set vintage character for authentic 39kHz Audity sound
        emuRomplerEngine->setVintageAmount(vintageAmount);
        emuRomplerEngine->setConverterType(vintageAmount > 0.5f ? 2 : 1);  // Full Audity mode at high pressure
        
        // Color controls analog noise and sample selection
        emuRomplerEngine->setAnalogNoise(colorSaturation * 0.3f);
        
        // Trigger note based on frequency and velocity based on pressure
        if (paintData.pressure > 0.1f)  // Only trigger if significant pressure
        {
            int midiNote = static_cast<int>(36 + paintData.freqNorm * 48);  // C2 to C6 range
            float velocity = paintData.pressure;
            
            emuRomplerEngine->noteOn(midiNote, velocity);
        }
    }
    
    // Map color to synthesis mode
    processedPaint.synthMode = getColorToSynthMode(paintData.color);
    
    // Process based on current synthesis mode
    switch (currentSynthMode.load())
    {
    case SynthMode::SpectralOscillators:
        addSpectralOscillator(processedPaint.frequencyHz, processedPaint.amplitude, paintData.color);
        break;
        
    case SynthMode::TrackerSequencing:
        if (linearTrackerEngine)
        {
            linearTrackerEngine->beginPaintStroke(paintData.timeNorm, paintData.freqNorm, 
                                                 paintData.pressure, paintData.color);
        }
        break;
        
    case SynthMode::PaintSynthesis:
        if (sampleMaskingEngine)
        {
            sampleMaskingEngine->beginPaintStroke(paintData.timeNorm, paintData.freqNorm, 
                                                 static_cast<SampleMaskingEngine::MaskingMode>(processedPaint.synthMode));
        }
        break;
        
    case SynthMode::EMUAudityMode:
        // EMU Audity mode is handled above in the EMU control section
        // Paint gestures directly control CEM3389 filter and trigger notes
        break;
        
    case SynthMode::HybridSynthesis:
        // Process through multiple engines simultaneously
        addSpectralOscillator(processedPaint.frequencyHz, processedPaint.amplitude, paintData.color);
        
        if (linearTrackerEngine)
        {
            linearTrackerEngine->beginPaintStroke(paintData.timeNorm, paintData.freqNorm, 
                                                 paintData.pressure, paintData.color);
        }
        
        if (sampleMaskingEngine)
        {
            sampleMaskingEngine->beginPaintStroke(paintData.timeNorm, paintData.freqNorm, 
                                                 static_cast<SampleMaskingEngine::MaskingMode>(processedPaint.synthMode));
        }
        break;
    }
}

void SpectralSynthEngine::beginPaintStroke(float x, float y, float pressure, juce::Colour color)
{
    PaintData paintData;
    paintData.timeNorm = screenXToTimeNorm(x);
    paintData.freqNorm = screenYToFreqNorm(y);
    paintData.pressure = pressure;
    paintData.color = color;
    paintData.timestamp = juce::Time::getMillisecondCounter();
    
    processPaintStroke(paintData);
}

void SpectralSynthEngine::updatePaintStroke(float x, float y, float pressure)
{
    // Update ongoing paint stroke
    // TODO: Implement continuous paint stroke updating
}

void SpectralSynthEngine::endPaintStroke()
{
    // Finalize paint stroke
    if (linearTrackerEngine)
        linearTrackerEngine->endPaintStroke();
    
    if (sampleMaskingEngine)
        sampleMaskingEngine->endPaintStroke();
}

//==============================================================================
// Spectral Oscillators - MetaSynth Inspired
//==============================================================================

void SpectralSynthEngine::SpectralOscillator::reset()
{
    isActive = false;
    frequency = 440.0f;
    amplitude = 0.0f;
    phase = 0.0f;
    panPosition = 0.0f;
    harmonicContent = 0.5f;
    spectralBrightness = 0.5f;
    spectralWidth = 0.1f;
    temporalEvolution = 0.0f;
    paintPressuremod = 0.0f;
    paintVelocityMod = 0.0f;
}

void SpectralSynthEngine::SpectralOscillator::updateFromPaint(const PaintData& paint)
{
    frequency = paint.frequencyHz;
    amplitude = paint.amplitude;
    panPosition = paint.panPosition;
    
    // Map paint pressure to spectral parameters
    paintPressuremod = paint.pressure;
    spectralBrightness = paint.pressure;
    
    // Map color to harmonic content
    float hue = paint.color.getHue();
    harmonicContent = hue;
    
    // Map color saturation to spectral width
    spectralWidth = paint.color.getSaturation() * 0.2f;
}

float SpectralSynthEngine::SpectralOscillator::renderNextSample(double sampleRate)
{
    if (!isActive) return 0.0f;
    
    float dt = static_cast<float>(1.0 / sampleRate);
    float twoPi = juce::MathConstants<float>::twoPi;
    
    //==============================================================================
    // Revolutionary Spectral Synthesis - Multiple Waveform Types
    //==============================================================================
    
    float baseSample = 0.0f;
    
    // Determine waveform type based on harmonic content
    if (harmonicContent < 0.2f)
    {
        // Pure sine wave (MetaSynth style)
        baseSample = std::sin(phase);
    }
    else if (harmonicContent < 0.4f)
    {
        // Sawtooth with spectral filtering
        baseSample = (2.0f * (phase / twoPi)) - 1.0f;
        
        // Anti-alias with simple filtering
        float cutoff = frequency * (2.0f + spectralBrightness * 8.0f);
        if (cutoff < static_cast<float>(sampleRate) * 0.45f)
        {
            // Apply spectral shaping to sawtooth
            baseSample = std::tanh(baseSample * (1.0f + spectralBrightness * 3.0f));
        }
    }
    else if (harmonicContent < 0.6f)
    {
        // Square wave with pulse width modulation
        float pulseWidth = 0.3f + spectralWidth * 0.4f; // 0.3 to 0.7
        baseSample = (phase < twoPi * pulseWidth) ? 1.0f : -1.0f;
        
        // Spectral filtering
        baseSample = std::tanh(baseSample * (0.5f + spectralBrightness * 2.0f));
    }
    else if (harmonicContent < 0.8f)
    {
        // Triangle wave with spectral warping
        float tri = (phase < juce::MathConstants<float>::pi) ? 
                   (4.0f * phase / twoPi) - 1.0f : 
                   3.0f - (4.0f * phase / twoPi);
        
        // Apply spectral distortion
        float distortion = spectralBrightness * 2.0f;
        baseSample = std::tanh(tri * (1.0f + distortion));
    }
    else
    {
        // Complex additive synthesis (true MetaSynth style)
        baseSample = std::sin(phase); // Fundamental
        
        // Add harmonics with spectral shaping
        int maxHarmonics = static_cast<int>(8.0f + spectralWidth * 24.0f);
        for (int h = 2; h <= maxHarmonics; ++h)
        {
            float harmonicFreq = frequency * h;
            if (harmonicFreq < static_cast<float>(sampleRate) * 0.45f)
            {
                float harmonicPhase = phase * h;
                float harmonicAmp = 1.0f / (h * (1.0f + spectralBrightness));
                
                // Apply spectral evolution over time
                harmonicAmp *= (1.0f + temporalEvolution * std::sin(harmonicPhase * 0.1f));
                
                baseSample += std::sin(harmonicPhase) * harmonicAmp;
            }
        }
        
        // Normalize complex waveform
        baseSample *= 0.3f;
    }
    
    //==============================================================================
    // Revolutionary Spectral Processing Effects
    //==============================================================================
    
    // Spectral width creates chorus/unison effect
    if (spectralWidth > 0.1f)
    {
        float detuneAmount = spectralWidth * 0.02f; // Up to 2% detune
        float detunePhase = phase * (1.0f + detuneAmount);
        float detunePhase2 = phase * (1.0f - detuneAmount * 0.5f);
        
        float detuneSample1 = std::sin(detunePhase);
        float detuneSample2 = std::sin(detunePhase2);
        
        baseSample = (baseSample * (1.0f - spectralWidth * 0.6f)) + 
                     (detuneSample1 * spectralWidth * 0.3f) +
                     (detuneSample2 * spectralWidth * 0.3f);
    }
    
    // Paint pressure modulation creates dynamic spectral changes
    if (paintPressuremod > 0.0f)
    {
        // Pressure affects amplitude and spectral brightness
        float pressureMod = 0.5f + paintPressuremod * 1.5f;
        baseSample *= pressureMod;
        
        // Pressure also creates slight frequency modulation
        float fmAmount = paintPressuremod * 0.001f;
        phase += std::sin(phase * 8.0f) * fmAmount;
    }
    
    // Paint velocity creates attack characteristics
    if (paintVelocityMod > 0.0f)
    {
        // Fast velocity creates sharper attacks
        float velocityFilter = 1.0f - std::exp(-paintVelocityMod * 10.0f);
        baseSample *= velocityFilter;
    }
    
    // Temporal evolution creates slowly changing spectral characteristics
    temporalEvolution += dt * 0.5f; // Slow evolution
    if (temporalEvolution > twoPi) temporalEvolution -= twoPi;
    
    // Apply temporal spectral morphing
    float evolutionMod = 1.0f + std::sin(temporalEvolution) * spectralWidth * 0.2f;
    baseSample *= evolutionMod;
    
    //==============================================================================
    // Final Processing and Phase Update
    //==============================================================================
    
    // Apply amplitude with dynamic range compression for professional sound
    float finalSample = baseSample * amplitude;
    finalSample = std::tanh(finalSample * 0.8f) * 1.25f; // Gentle saturation
    
    // Update phase with potential FM from spectral parameters
    float phaseIncrement = frequency * twoPi / static_cast<float>(sampleRate);
    
    // Subtle FM based on spectral brightness
    if (spectralBrightness > 0.5f)
    {
        float fmDepth = (spectralBrightness - 0.5f) * 0.02f;
        phaseIncrement *= (1.0f + std::sin(phase * 3.7f) * fmDepth);
    }
    
    phase += phaseIncrement;
    if (phase >= twoPi) phase -= twoPi;
    
    return finalSample;
}

void SpectralSynthEngine::addSpectralOscillator(float frequency, float amplitude, juce::Colour color)
{
    juce::ScopedLock lock(oscillatorLock);
    
    auto* oscillator = findFreeOscillator();
    if (oscillator != nullptr)
    {
        oscillator->isActive = true;
        oscillator->frequency = frequency;
        oscillator->amplitude = amplitude;
        oscillator->sourceColor = color;
        oscillator->panPosition = (color.getHue() - 0.5f) * 2.0f;
        
        // Map color to spectral characteristics
        oscillator->harmonicContent = color.getHue();
        oscillator->spectralBrightness = color.getBrightness();
        oscillator->spectralWidth = color.getSaturation() * 0.2f;
        
        activeOscillatorCount.store(activeOscillatorCount.load() + 1);
        
        // DEBUG: Log oscillator creation
        DBG("*** OSCILLATOR ADDED *** Freq: " << frequency << "Hz, Amp: " << amplitude << ", Total Active: " << activeOscillatorCount.load());
    }
    else
    {
        DBG("*** OSCILLATOR POOL FULL *** Cannot add oscillator");
    }
}

void SpectralSynthEngine::removeSpectralOscillator(int index)
{
    juce::ScopedLock lock(oscillatorLock);
    
    if (index >= 0 && index < MAX_SPECTRAL_OSCILLATORS)
    {
        if (spectralOscillators[index].isActive)
        {
            spectralOscillators[index].reset();
            activeOscillatorCount.store(activeOscillatorCount.load() - 1);
        }
    }
}

void SpectralSynthEngine::clearAllSpectralOscillators()
{
    juce::ScopedLock lock(oscillatorLock);
    
    for (auto& oscillator : spectralOscillators)
    {
        oscillator.reset();
    }
    
    activeOscillatorCount.store(0);
}

SpectralSynthEngine::SpectralOscillator* SpectralSynthEngine::findFreeOscillator()
{
    for (auto& oscillator : spectralOscillators)
    {
        if (!oscillator.isActive)
            return &oscillator;
    }
    return nullptr;
}

//==============================================================================
// Audio Processing Implementation
//==============================================================================

void SpectralSynthEngine::processSpectralOscillators(juce::AudioBuffer<float>& buffer)
{
    juce::ScopedLock lock(oscillatorLock);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float leftSample = 0.0f;
        float rightSample = 0.0f;
        
        // Sum all active oscillators
        for (auto& oscillator : spectralOscillators)
        {
            if (oscillator.isActive)
            {
                float oscSample = oscillator.renderNextSample(currentSampleRate);
                
                // Apply panning
                float panLeft = (1.0f - oscillator.panPosition) * 0.5f;
                float panRight = (1.0f + oscillator.panPosition) * 0.5f;
                
                leftSample += oscSample * panLeft;
                rightSample += oscSample * panRight;
            }
        }
        
        // Apply to buffer
        if (buffer.getNumChannels() > 0)
            buffer.addSample(0, sample, leftSample);
        
        if (buffer.getNumChannels() > 1)
            buffer.addSample(1, sample, rightSample);
    }
}

void SpectralSynthEngine::mixSynthesisEngines(juce::AudioBuffer<float>& outputBuffer,
                                            const juce::AudioBuffer<float>& paintSynthBuffer,
                                            const juce::AudioBuffer<float>& spectralOscBuffer,
                                            const juce::AudioBuffer<float>& trackerBuffer,
                                            const juce::AudioBuffer<float>& granularBuffer,
                                            const juce::AudioBuffer<float>& sampleBuffer,
                                            const juce::AudioBuffer<float>& emuBuffer)
{
    SynthMode mode = currentSynthMode.load();
    
    switch (mode)
    {
    case SynthMode::PaintSynthesis:
        outputBuffer.addFrom(0, 0, paintSynthBuffer, 0, 0, outputBuffer.getNumSamples());
        if (outputBuffer.getNumChannels() > 1 && paintSynthBuffer.getNumChannels() > 1)
            outputBuffer.addFrom(1, 0, paintSynthBuffer, 1, 0, outputBuffer.getNumSamples());
        break;
        
    case SynthMode::SpectralOscillators:
        outputBuffer.addFrom(0, 0, spectralOscBuffer, 0, 0, outputBuffer.getNumSamples());
        if (outputBuffer.getNumChannels() > 1 && spectralOscBuffer.getNumChannels() > 1)
            outputBuffer.addFrom(1, 0, spectralOscBuffer, 1, 0, outputBuffer.getNumSamples());
        break;
        
    case SynthMode::TrackerSequencing:
        outputBuffer.addFrom(0, 0, trackerBuffer, 0, 0, outputBuffer.getNumSamples());
        if (outputBuffer.getNumChannels() > 1 && trackerBuffer.getNumChannels() > 1)
            outputBuffer.addFrom(1, 0, trackerBuffer, 1, 0, outputBuffer.getNumSamples());
        break;
        
    case SynthMode::SampleSynthesis:
        outputBuffer.addFrom(0, 0, sampleBuffer, 0, 0, outputBuffer.getNumSamples());
        if (outputBuffer.getNumChannels() > 1 && sampleBuffer.getNumChannels() > 1)
            outputBuffer.addFrom(1, 0, sampleBuffer, 1, 0, outputBuffer.getNumSamples());
        break;
        
    case SynthMode::EMUAudityMode:
        // The legendary EMU Audity sound!
        outputBuffer.addFrom(0, 0, emuBuffer, 0, 0, outputBuffer.getNumSamples());
        if (outputBuffer.getNumChannels() > 1 && emuBuffer.getNumChannels() > 1)
            outputBuffer.addFrom(1, 0, emuBuffer, 1, 0, outputBuffer.getNumSamples());
        break;
        
    case SynthMode::HybridSynthesis:
        // Mix all engines with equal weighting (including legendary EMU!)
        float mixLevel = 0.8f; // Increased for immediate musical results - was 0.3f
        
        outputBuffer.addFromWithRamp(0, 0, paintSynthBuffer.getReadPointer(0), outputBuffer.getNumSamples(), mixLevel, mixLevel);
        outputBuffer.addFromWithRamp(0, 0, spectralOscBuffer.getReadPointer(0), outputBuffer.getNumSamples(), mixLevel, mixLevel);
        outputBuffer.addFromWithRamp(0, 0, trackerBuffer.getReadPointer(0), outputBuffer.getNumSamples(), mixLevel, mixLevel);
        outputBuffer.addFromWithRamp(0, 0, sampleBuffer.getReadPointer(0), outputBuffer.getNumSamples(), mixLevel, mixLevel);
        outputBuffer.addFromWithRamp(0, 0, emuBuffer.getReadPointer(0), outputBuffer.getNumSamples(), mixLevel, mixLevel);
        
        if (outputBuffer.getNumChannels() > 1)
        {
            if (paintSynthBuffer.getNumChannels() > 1)
                outputBuffer.addFromWithRamp(1, 0, paintSynthBuffer.getReadPointer(1), outputBuffer.getNumSamples(), mixLevel, mixLevel);
            if (spectralOscBuffer.getNumChannels() > 1)
                outputBuffer.addFromWithRamp(1, 0, spectralOscBuffer.getReadPointer(1), outputBuffer.getNumSamples(), mixLevel, mixLevel);
            if (trackerBuffer.getNumChannels() > 1)
                outputBuffer.addFromWithRamp(1, 0, trackerBuffer.getReadPointer(1), outputBuffer.getNumSamples(), mixLevel, mixLevel);
            if (sampleBuffer.getNumChannels() > 1)
                outputBuffer.addFromWithRamp(1, 0, sampleBuffer.getReadPointer(1), outputBuffer.getNumSamples(), mixLevel, mixLevel);
            if (emuBuffer.getNumChannels() > 1)
                outputBuffer.addFromWithRamp(1, 0, emuBuffer.getReadPointer(1), outputBuffer.getNumSamples(), mixLevel, mixLevel);
        }
        break;
    }
}

//==============================================================================
// Coordinate System Helpers
//==============================================================================

float SpectralSynthEngine::screenXToTimeNorm(float x) const
{
    return juce::jlimit(0.0f, 1.0f, x / canvasWidth);
}

float SpectralSynthEngine::screenYToFreqNorm(float y) const
{
    return juce::jlimit(0.0f, 1.0f, 1.0f - (y / canvasHeight));
}

float SpectralSynthEngine::freqNormToHz(float freqNorm) const
{
    float logMin = std::log10(minFrequencyHz);
    float logMax = std::log10(maxFrequencyHz);
    float logFreq = logMin + freqNorm * (logMax - logMin);
    return std::pow(10.0f, logFreq);
}

float SpectralSynthEngine::hzToFreqNorm(float hz) const
{
    float logMin = std::log10(minFrequencyHz);
    float logMax = std::log10(maxFrequencyHz);
    float logHz = std::log10(juce::jlimit(minFrequencyHz, maxFrequencyHz, hz));
    return (logHz - logMin) / (logMax - logMin);
}

int SpectralSynthEngine::getColorToSynthMode(juce::Colour color) const
{
    float hue = color.getHue();
    
    if (hue < 0.1f || hue > 0.9f)      return 0; // Red: Volume
    else if (hue < 0.2f)               return 1; // Orange: Distortion  
    else if (hue < 0.35f)              return 2; // Yellow: Filter
    else if (hue < 0.5f)               return 3; // Green: Ring mod
    else if (hue < 0.65f)              return 4; // Cyan: Pitch
    else                               return 5; // Blue/Purple: Stutter
}

//==============================================================================
// Additional Implementation Stubs
//==============================================================================

void SpectralSynthEngine::setCanvasSize(float width, float height)
{
    canvasWidth = width;
    canvasHeight = height;
}

void SpectralSynthEngine::setFrequencyRange(float minHz, float maxHz)
{
    minFrequencyHz = minHz;
    maxFrequencyHz = maxHz;
}

void SpectralSynthEngine::setTimeRange(float startSec, float endSec)
{
    startTimeSec = startSec;
    endTimeSec = endSec;
}

SpectralSynthEngine::PerformanceMetrics SpectralSynthEngine::getPerformanceMetrics() const
{
    return currentMetrics;
}

void SpectralSynthEngine::enableSpectralAnalysis(bool enable)
{
    spectralAnalysisEnabled.store(enable);
}

void SpectralSynthEngine::performSpectralAnalysis(const juce::AudioBuffer<float>& buffer)
{
    // TODO: Implement FFT-based spectral analysis
    // This is a simplified placeholder
}

void SpectralSynthEngine::applySpectralProcessingToBuffer(juce::AudioBuffer<float>& buffer)
{
    // TODO: Implement CDP-inspired spectral processing
    // This is a placeholder for spectral transformations
}

SpectralSynthEngine::SpectralAnalysis SpectralSynthEngine::getCurrentSpectralAnalysis() const
{
    juce::ScopedLock lock(analysisLock);
    return currentAnalysis;
}

//==============================================================================
// Spectral Processing Control
//==============================================================================

void SpectralSynthEngine::setSpectralProcessing(SpectralProcessType type, float intensity)
{
    currentSpectralProcess.store(type);
    spectralProcessIntensity.store(intensity);
}

//==============================================================================
// Command System Integration
//==============================================================================

bool SpectralSynthEngine::processSpectralCommand(const Command& cmd)
{
    // Forward to individual engines based on command type
    if (cmd.isSampleMaskingCommand() && sampleMaskingEngine)
    {
        // Handle sample masking commands
        try
        {
            switch (cmd.getSampleMaskingCommandID())
            {
            case SampleMaskingCommandID::CreatePaintMask:
                {
                    auto mode = static_cast<SampleMaskingEngine::MaskingMode>(static_cast<int>(cmd.floatParam));
                    juce::uint32 maskId = sampleMaskingEngine->createPaintMask(mode, cmd.color);
                    return true;
                }
                
            case SampleMaskingCommandID::BeginPaintStroke:
                {
                    auto mode = static_cast<SampleMaskingEngine::MaskingMode>(static_cast<int>(cmd.floatParam));
                    sampleMaskingEngine->beginPaintStroke(cmd.x, cmd.y, mode);
                    
                    // Also trigger spectral oscillators for revolutionary synthesis
                    SpectralSynthEngine::PaintData paintData;
                    paintData.timeNorm = cmd.x;
                    paintData.freqNorm = cmd.y;
                    paintData.pressure = cmd.pressure;
                    paintData.color = cmd.color;
                    processPaintStroke(paintData);
                    
                    return true;
                }
                
            case SampleMaskingCommandID::UpdatePaintStroke:
                sampleMaskingEngine->updatePaintStroke(cmd.x, cmd.y, cmd.pressure);
                return true;
                
            case SampleMaskingCommandID::EndPaintStroke:
                sampleMaskingEngine->endPaintStroke();
                return true;
                
            case SampleMaskingCommandID::ClearAllMasks:
                sampleMaskingEngine->clearAllMasks();
                clearAllSpectralOscillators();
                return true;
                
            case SampleMaskingCommandID::StartPlayback:
                sampleMaskingEngine->startPlayback();
                return true;
                
            case SampleMaskingCommandID::StopPlayback:
                sampleMaskingEngine->stopPlayback();
                return true;
                
            default:
                return false;
            }
        }
        catch (...)
        {
            return false;
        }
    }
    
    return false;
}