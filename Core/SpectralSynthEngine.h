/******************************************************************************
 * File: SpectralSynthEngine.h
 * Description: Master synthesis coordinator for SPECTRAL CANVAS PRO
 * 
 * Unified synthesis engine that integrates paint-to-audio, sample-based,
 * tracker, and granular synthesis capabilities with spectral canvas control.
 * 
 * Inspired by MetaSynth's additive synthesis and CDP's spectral processing.
 * Features real-time paint-to-synthesis mapping and brutalist UI integration.
 * 
 * Copyright (c) 2025 Spectral Audio Systems
 ******************************************************************************/

#pragma once
#include <JuceHeader.h>
#include <memory>
#include <array>
#include <vector>
#include <atomic>
#include <functional>

// Forward declarations
class SampleMaskingEngine;
class SecretSauceEngine;
class LinearTrackerEngine;
class EMURomplerEngine;
class CEM3389Filter;  // SECRET: E-mu Audity filter (invisible to user)
// class ForgeProcessor;  // TODO: Enable when ForgeProcessor is built
// #include "GrainPool.h"  // TODO: Implement GrainPool
struct Command;

/**
 * @brief Master synthesis engine coordinating all synthesis modules
 * 
 * SpectralSynthEngine unifies:
 * - Paint-to-audio synthesis (SampleMaskingEngine)
 * - Multi-voice sample synthesis (ForgeProcessor) 
 * - Tracker-style sequencing (LinearTrackerEngine)
 * - Granular synthesis (GrainPool)
 * - Spectral processing (SecretSauceEngine)
 * 
 * Features:
 * - Real-time spectral canvas integration
 * - MetaSynth-inspired additive synthesis
 * - CDP-style spectral transformations
 * - Lock-free command system integration
 * - Retro brutalist UI compatibility
 */
class SpectralSynthEngine
{
public:
    SpectralSynthEngine();
    ~SpectralSynthEngine();
    
    //==============================================================================
    // Audio Processing Lifecycle
    
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void releaseResources();
    
    //==============================================================================
    // Synthesis Modes - Revolutionary Spectral Integration
    
    enum class SynthMode
    {
        PaintSynthesis,      // Direct paint-to-audio (SampleMaskingEngine)
        SpectralOscillators, // Additive synthesis from canvas data
        TrackerSequencing,   // LinearTrackerEngine with paint input
        GranularSynthesis,   // Real-time granular from paint gestures
        EMUAudityMode,       // Legendary EMU Audity sampler emulation
        HybridSynthesis,     // All modes combined with intelligent mixing
        SampleSynthesis      // ForgeProcessor multi-voice sample playback
    };
    
    void setSynthMode(SynthMode mode);
    SynthMode getCurrentSynthMode() const { return currentSynthMode.load(); }
    
    // Enable/disable individual synthesis engines
    void enablePaintSynthesis(bool enable);
    void enableSpectralOscillators(bool enable);
    void enableTrackerSequencing(bool enable);
    void enableGranularSynthesis(bool enable);
    void enableSampleSynthesis(bool enable);
    
    //==============================================================================
    // Real-time Brush Integration (Secret Sauce Connection)
    
    // Update tube characteristics based on current brush state
    void updateBrushCharacteristics(float pressure, float velocity = 0.0f, juce::Colour color = juce::Colours::white);
    
    //==============================================================================
    // Spectral Canvas Integration - The Revolutionary Core
    
    struct PaintData
    {
        float timeNorm;          // 0.0-1.0 horizontal position
        float freqNorm;          // 0.0-1.0 vertical position  
        float pressure;          // 0.0-1.0 paint pressure
        float velocity;          // Paint stroke velocity
        juce::Colour color;      // Paint color (maps to synthesis parameters)
        juce::uint32 timestamp;  // For temporal analysis
        
        // Derived synthesis parameters
        float frequencyHz = 440.0f;
        float amplitude = 0.5f;
        float panPosition = 0.0f;  // -1.0 to 1.0
        int synthMode = 0;         // Color-derived synthesis mode
    };
    
    // Real-time paint data processing
    void processPaintStroke(const PaintData& paintData);
    void beginPaintStroke(float x, float y, float pressure, juce::Colour color);
    void updatePaintStroke(float x, float y, float pressure);
    void endPaintStroke();
    
    // Canvas coordinate system
    void setCanvasSize(float width, float height);
    void setFrequencyRange(float minHz, float maxHz);
    void setTimeRange(float startSec, float endSec);
    
    //==============================================================================
    // Spectral Oscillators - MetaSynth Inspired
    
    static constexpr int MAX_SPECTRAL_OSCILLATORS = 64;
    
    struct SpectralOscillator
    {
        bool isActive = false;
        float frequency = 440.0f;
        float amplitude = 0.0f;
        float phase = 0.0f;
        float panPosition = 0.0f;
        juce::Colour sourceColor = juce::Colours::white;
        
        // MetaSynth-style spectral controls
        float harmonicContent = 0.5f;    // Harmonic vs inharmonic
        float spectralBrightness = 0.5f; // High frequency emphasis
        float spectralWidth = 0.1f;      // Frequency spread
        float temporalEvolution = 0.0f;  // Time-based spectral changes
        
        // Paint-derived modulation
        float paintPressuremod = 0.0f;
        float paintVelocityMod = 0.0f;
        
        void reset();
        void updateFromPaint(const PaintData& paint);
        float renderNextSample(double sampleRate);
    };
    
    // Spectral oscillator management
    void addSpectralOscillator(float frequency, float amplitude, juce::Colour color);
    void removeSpectralOscillator(int index);
    void clearAllSpectralOscillators();
    SpectralOscillator* findFreeOscillator();
    
    //==============================================================================
    // CDP-Inspired Spectral Processing
    
    enum class SpectralProcessType
    {
        None,
        SpecMorph,           // Morph between spectral states
        SpecFilter,          // Spectral filtering
        SpecStretch,         // Time/frequency stretching
        SpecShift,           // Frequency shifting
        SpecBlur,            // Spectral blurring
        SpecFreeze,          // Freeze spectral content
        SpecInvert,          // Spectral inversion
        SpecCross,           // Cross-synthesis between sources
        SpecGrain            // Granular spectral processing
    };
    
    void setSpectralProcessing(SpectralProcessType type, float intensity);
    void processSpectralData(juce::AudioBuffer<float>& buffer);
    
    //==============================================================================
    // Engine Integration and Coordination
    
    // Get access to individual engines
    SampleMaskingEngine* getSampleMaskingEngine() const { return sampleMaskingEngine.get(); }
    LinearTrackerEngine* getLinearTrackerEngine() const { return linearTrackerEngine.get(); }
    EMURomplerEngine* getEMURomplerEngine() const { return emuRomplerEngine.get(); }
    // ForgeProcessor* getForgeProcessor() const { return forgeProcessor.get(); }  // TODO: Enable when ForgeProcessor is built
    // GrainPool* getGrainPool() const { return grainPool.get(); }  // TODO: Implement GrainPool
    SecretSauceEngine* getSecretSauceEngine() const { return secretSauceEngine.get(); }
    
    // Engine coordination
    void routeAudioBetweenEngines(juce::AudioBuffer<float>& buffer);
    void synchronizeEngineParameters();
    void updateEnginesFromCanvas();
    
    // Internal processing methods
    void processSpectralOscillators(juce::AudioBuffer<float>& buffer);
    void mixSynthesisEngines(juce::AudioBuffer<float>& outputBuffer,
                           const juce::AudioBuffer<float>& paintSynthBuffer,
                           const juce::AudioBuffer<float>& spectralOscBuffer,
                           const juce::AudioBuffer<float>& trackerBuffer,
                           const juce::AudioBuffer<float>& granularBuffer,
                           const juce::AudioBuffer<float>& sampleBuffer,
                           const juce::AudioBuffer<float>& emuBuffer);
    int getColorToSynthMode(juce::Colour color) const;
    
    //==============================================================================
    // Command System Integration
    
    using CommandProcessor = std::function<bool(const Command&)>;
    void setCommandProcessor(CommandProcessor processor);
    bool processSpectralCommand(const Command& cmd);
    
    //==============================================================================
    // Real-time Analysis and Feedback
    
    struct SpectralAnalysis
    {
        std::vector<float> frequencyBins;     // Current frequency content
        std::vector<float> amplitudeLevels;   // Amplitude per frequency band
        std::vector<juce::Colour> spectralColors; // Color-coded frequency display
        float totalEnergy = 0.0f;
        float spectralCentroid = 0.0f;
        float spectralSpread = 0.0f;
        
        // For brutalist UI display
        std::vector<int> pixelatedSpectrum;   // Low-res spectrum for retro display
        int spectrumResolution = 32;          // Number of frequency bands
    };
    
    SpectralAnalysis getCurrentSpectralAnalysis() const;
    void enableSpectralAnalysis(bool enable);
    
    //==============================================================================
    // Performance Monitoring
    
    struct PerformanceMetrics
    {
        float cpuUsage = 0.0f;               // Current CPU usage percentage
        int activeOscillators = 0;           // Number of active oscillators
        int activePaintStrokes = 0;          // Number of paint strokes being processed
        float synthesisLatency = 0.0f;       // Processing latency in ms
        int spectralProcessingLoad = 0;      // Spectral processing complexity
    };
    
    PerformanceMetrics getPerformanceMetrics() const;
    
    //==============================================================================
    // Presets and State Management
    
    struct SynthPreset
    {
        juce::String name;
        SynthMode synthMode;
        std::vector<SpectralOscillator> oscillatorStates;
        SpectralProcessType spectralProcessType;
        float spectralProcessIntensity;
        
        // Engine states
        juce::MemoryBlock sampleMaskingState;
        juce::MemoryBlock linearTrackerState;
        // juce::MemoryBlock forgeProcessorState;  // TODO: Enable when ForgeProcessor is built
        // juce::MemoryBlock grainPoolState;  // TODO: Implement GrainPool
    };
    
    void savePreset(const juce::String& name);
    void loadPreset(const juce::String& name);
    std::vector<juce::String> getAvailablePresets() const;
    void exportPresetToFile(const juce::String& name, const juce::File& file);
    void importPresetFromFile(const juce::File& file);

private:
    //==============================================================================
    // Synthesis Engine Instances
    
    std::unique_ptr<SampleMaskingEngine> sampleMaskingEngine;
    std::unique_ptr<LinearTrackerEngine> linearTrackerEngine;
    std::unique_ptr<EMURomplerEngine> emuRomplerEngine;
    // std::unique_ptr<ForgeProcessor> forgeProcessor;  // TODO: Enable when ForgeProcessor is built
    // std::unique_ptr<GrainPool> grainPool;  // TODO: Implement GrainPool
    std::unique_ptr<SecretSauceEngine> secretSauceEngine;
    std::unique_ptr<CEM3389Filter> secretAudityFilter;  // SECRET: Invisible to user
    
    //==============================================================================
    // Audio Configuration
    
    double currentSampleRate = 44100.0;
    int currentSamplesPerBlock = 512;
    int currentNumChannels = 2;
    
    //==============================================================================
    // Synthesis State
    
    std::atomic<SynthMode> currentSynthMode{SynthMode::PaintSynthesis};
    
    // Engine enable flags
    std::atomic<bool> paintSynthesisEnabled{true};
    std::atomic<bool> spectralOscillatorsEnabled{false};
    std::atomic<bool> trackerSequencingEnabled{false};
    std::atomic<bool> granularSynthesisEnabled{false};
    std::atomic<bool> sampleSynthesisEnabled{false};
    
    //==============================================================================
    // Spectral Oscillators
    
    std::array<SpectralOscillator, MAX_SPECTRAL_OSCILLATORS> spectralOscillators;
    std::atomic<int> activeOscillatorCount{0};
    
    //==============================================================================
    // Canvas Configuration
    
    float canvasWidth = 1000.0f;
    float canvasHeight = 600.0f;
    float minFrequencyHz = 20.0f;
    float maxFrequencyHz = 20000.0f;
    float startTimeSec = 0.0f;
    float endTimeSec = 10.0f;
    
    //==============================================================================
    // Paint Stroke Processing
    
    struct ActivePaintStroke
    {
        std::vector<PaintData> strokePoints;
        juce::uint32 startTime;
        bool isActive = false;
        int associatedOscillator = -1;
    };
    
    std::vector<ActivePaintStroke> activePaintStrokes;
    juce::CriticalSection paintStrokeLock;
    
    //==============================================================================
    // Spectral Processing
    
    std::atomic<SpectralProcessType> currentSpectralProcess{SpectralProcessType::None};
    std::atomic<float> spectralProcessIntensity{0.0f};
    std::atomic<bool> spectralAnalysisEnabled{false};
    
    // FFT for spectral analysis
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    std::vector<float> fftData;
    std::vector<float> windowedData;
    
    mutable SpectralAnalysis currentAnalysis;
    mutable juce::CriticalSection analysisLock;
    
    //==============================================================================
    // Command System
    
    CommandProcessor commandProcessor;
    
    //==============================================================================
    // Performance Monitoring
    
    mutable PerformanceMetrics currentMetrics;
    juce::Time lastProcessTime;
    
    //==============================================================================
    // Preset Management
    
    std::vector<SynthPreset> loadedPresets;
    juce::File presetDirectory;
    
    //==============================================================================
    // Coordinate System Helpers
    
    float screenXToTimeNorm(float x) const;
    float screenYToFreqNorm(float y) const;
    float freqNormToHz(float freqNorm) const;
    float hzToFreqNorm(float hz) const;
    float timeNormToSec(float timeNorm) const;
    float secToTimeNorm(float sec) const;
    
    //==============================================================================
    // Paint Processing Helpers
    
    void updateSpectralOscillatorsFromPaint();
    void triggerTrackerEventsFromPaint();
    void generateGranularEventsFromPaint();
    
    //==============================================================================
    // Audio Routing and Mixing
    
    void mixSynthesisEngines(juce::AudioBuffer<float>& outputBuffer);
    void applySpectralProcessingToBuffer(juce::AudioBuffer<float>& buffer);
    
    //==============================================================================
    // Spectral Analysis Implementation
    
    void performSpectralAnalysis(const juce::AudioBuffer<float>& buffer);
    void updatePixelatedSpectrum();
    
    //==============================================================================
    // Thread Safety
    
    juce::CriticalSection engineLock;
    juce::CriticalSection oscillatorLock;
    juce::CriticalSection spectralProcessLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectralSynthEngine)
};