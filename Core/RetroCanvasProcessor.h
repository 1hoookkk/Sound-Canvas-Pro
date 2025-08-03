#pragma once
#include <JuceHeader.h>
#include "CommandQueue.h"
#include "Commands.h"
#include <memory>
#include <atomic>

// Forward declarations
class EMURomplerEngine;
class TubeWarmthEngine;
class SignaturePhaserEngine;
class SampleMaskingEngine;
class LinearTrackerEngine;
class SmartMacroSystem;
class AICreativeAssistant;
class GPUAccelerationEngine;
class CollaborativeManager;
class HardwareControllerManager;
class VisualFeedbackEngine;

/**
 * RetroCanvas Pro - Revolutionary Music Production Paradigm
 * 
 * PARADIGM SHIFT: Paint interface as universal audio control language
 * 
 * Core Revolutionary Engines:
 * - Sample Masking Engine: Paint over samples for infinite variations
 * - Linear Tracker Engine: Frequency-separated sequencing with AI
 * - EMU Rompler Engine: Vintage sample library with modern AI
 * - Tube Warmth Engine: Analog character processing
 * - Signature Phaser Engine: Advanced modulation effects
 * 
 * AI Integration: Intelligent creative assistance and collaboration
 * GPU Acceleration: Real-time complex processing pipeline
 * Cloud Platform: Collaborative music creation and community features
 * Hardware Integration: Dedicated controllers and touch interfaces
 * 
 * Vision: Change how music is created through visual expression
 */
class RetroCanvasProcessor : public juce::AudioProcessor
{
public:
    RetroCanvasProcessor();
    ~RetroCanvasProcessor() override;
    
    //==============================================================================
    // AudioProcessor Interface
    
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    
    const juce::String getName() const override;
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override;
    
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    //==============================================================================
    // Paint Interface API
    
    struct PaintPoint
    {
        float x = 0.0f;           // Canvas X coordinate
        float y = 0.0f;           // Canvas Y coordinate  
        float pressure = 1.0f;    // Brush pressure (0.0-1.0)
        float velocity = 0.0f;    // Brush velocity (derived from movement speed)
        juce::Colour color = juce::Colours::white;  // Brush color
        juce::uint32 timestamp = 0;  // Creation timestamp
        
        PaintPoint() = default;
        PaintPoint(float x_, float y_, float pressure_ = 1.0f, juce::Colour color_ = juce::Colours::white)
            : x(x_), y(y_), pressure(pressure_), color(color_)
            , timestamp(juce::Time::getMillisecondCounter()) {}
    };
    
    // Paint stroke control (thread-safe via command queue)
    void beginPaintStroke(const PaintPoint& startPoint);
    void updatePaintStroke(const PaintPoint& point);
    void endPaintStroke();
    
    // Canvas control
    void clearCanvas();
    void setCanvasRegion(float left, float right, float bottom, float top);
    void setPlayheadPosition(float normalizedPosition);
    
    //==============================================================================
    // Revolutionary Engine Access & Control
    
    // Core Audio Engines
    EMURomplerEngine& getEMUEngine() { return *emuEngine; }
    TubeWarmthEngine& getTubeEngine() { return *tubeEngine; }
    SignaturePhaserEngine& getPhaserEngine() { return *phaserEngine; }
    
    // Revolutionary Engines - The Game Changers
    SampleMaskingEngine& getMaskingEngine() { return *maskingEngine; }
    LinearTrackerEngine& getTrackerEngine() { return *trackerEngine; }
    
    // Intelligence & Automation
    AICreativeAssistant& getAIAssistant() { return *aiAssistant; }
    SmartMacroSystem& getMacroSystem() { return *macroSystem; }
    
    // Performance & Visualization
    GPUAccelerationEngine& getGPUEngine() { return *gpuEngine; }
    VisualFeedbackEngine& getVisualEngine() { return *visualEngine; }
    
    // Collaboration & Hardware
    CollaborativeManager& getCollaborativeManager() { return *collaborativeManager; }
    HardwareControllerManager& getHardwareManager() { return *hardwareManager; }
    
    // Revolutionary Processing Modes
    enum class ProcessingMode
    {
        // Traditional Modes
        EMUOnly = 0,            // Just the EMU rompler
        EMUWithTube,            // EMU + Tube warmth
        EMUWithPhaser,          // EMU + Phaser
        FullChain,              // EMU → Tube → Phaser (classic mode)
        
        // Revolutionary Modes - The Game Changers
        SampleMasking,          // Sample Masking Engine primary
        LinearTracker,          // Linear Tracker Engine primary
        MaskingTracker,         // Masking + Tracker combined
        
        // AI-Enhanced Modes
        AIAssisted,             // AI suggests and enhances any mode
        AICollaborative,        // AI + human collaborative mode
        
        // Performance Modes
        GPUAccelerated,         // GPU-powered real-time processing
        HardwareControl,        // Dedicated hardware controller mode
        TouchInterface,         // Optimized for touch/tablet control
        
        // Specialized Modes
        EducationalMode,        // Learning and tutorial mode
        CollaborativeSession,   // Multi-user real-time collaboration
        LivePerformance         // Optimized for live use
    };
    
    void setProcessingMode(ProcessingMode mode) { currentMode.store(static_cast<int>(mode)); }
    ProcessingMode getProcessingMode() const { return static_cast<ProcessingMode>(currentMode.load()); }
    
    //==============================================================================
    // Performance Monitoring
    
    struct PerformanceStats
    {
        float cpuUsagePercent = 0.0f;
        float memoryUsageMB = 0.0f;
        int activeVoices = 0;
        int droppedSamples = 0;
        float latencyMs = 0.0f;
    };
    
    PerformanceStats getPerformanceStats() const;
    
    //==============================================================================
    // Producer Features
    
    // A/B Comparison
    void saveStateToSlot(int slot);  // 0 = A, 1 = B
    void loadStateFromSlot(int slot);
    void morphBetweenSlots(float morphAmount); // 0.0 = A, 1.0 = B
    
    // Smart Randomization  
    void randomizeParameters(bool lockEMU = false, bool lockTube = false, bool lockPhaser = false);
    
    // Preset Management
    void savePreset(const juce::String& name, const juce::String& category = "User");
    void loadPreset(const juce::String& name);
    juce::StringArray getPresetList(const juce::String& category = "") const;
    
private:
    //==============================================================================
    // Revolutionary Architecture Foundation
    
    // Core Audio Processing Engines (signal flow order)
    std::unique_ptr<EMURomplerEngine> emuEngine;
    std::unique_ptr<TubeWarmthEngine> tubeEngine;
    std::unique_ptr<SignaturePhaserEngine> phaserEngine;
    
    // Revolutionary Game-Changing Engines
    std::unique_ptr<SampleMaskingEngine> maskingEngine;     // Paint-over-samples magic
    std::unique_ptr<LinearTrackerEngine> trackerEngine;     // Frequency-intelligent sequencing
    
    // Intelligence & Creativity Systems
    std::unique_ptr<AICreativeAssistant> aiAssistant;       // AI creative partner
    std::unique_ptr<SmartMacroSystem> macroSystem;          // Intelligent macro control
    
    // Performance & Visualization Infrastructure
    std::unique_ptr<GPUAccelerationEngine> gpuEngine;       // GPU-powered processing
    std::unique_ptr<VisualFeedbackEngine> visualEngine;     // Advanced visual feedback
    
    // Collaboration & Hardware Integration
    std::unique_ptr<CollaborativeManager> collaborativeManager;  // Real-time collaboration
    std::unique_ptr<HardwareControllerManager> hardwareManager;  // Hardware integration
    
    // Command & Communication Systems
    CommandQueue<1024> commandQueue;  // Increased size for complex workflows
    
    // Processing control
    std::atomic<int> currentMode{static_cast<int>(ProcessingMode::FullChain)};
    std::atomic<bool> isActive{true};
    
    // Audio processing buffers (to avoid allocation in processBlock)
    juce::AudioBuffer<float> emuBuffer;
    juce::AudioBuffer<float> tubeBuffer;
    juce::AudioBuffer<float> phaserBuffer;
    juce::AudioBuffer<float> tempBuffer;
    
    //==============================================================================
    // Paint System
    
    struct PaintStroke
    {
        std::vector<PaintPoint> points;
        juce::uint32 strokeId;
        bool isActive = false;
        float totalLength = 0.0f;
        juce::Rectangle<float> bounds;
        
        void addPoint(const PaintPoint& point);
        void finalize();
        float getAgeInSeconds() const;
    };
    
    std::vector<std::unique_ptr<PaintStroke>> activeStrokes;
    std::unique_ptr<PaintStroke> currentStroke;
    juce::uint32 nextStrokeId{1};
    
    // Canvas bounds and mapping
    float canvasLeft = -100.0f;
    float canvasRight = 100.0f;
    float canvasBottom = -50.0f;
    float canvasTop = 50.0f;
    float playheadPosition = 0.0f;
    
    //==============================================================================
    // Command Processing
    
    void processCommands();
    void processCommand(const Command& cmd);
    void processPaintCommand(const Command& cmd);
    void processEngineCommand(const Command& cmd);
    void processMacroCommand(const Command& cmd);
    
    //==============================================================================
    // Paint-to-Parameter Mapping
    
    struct ParameterMapping
    {
        enum class Parameter
        {
            // EMU Engine
            EMU_SampleSelect,
            EMU_Pitch,
            EMU_FilterCutoff,
            EMU_FilterResonance,
            EMU_Attack,
            EMU_Release,
            EMU_Volume,
            
            // Tube Engine  
            Tube_Glow,
            Tube_Sag,
            Tube_Air,
            Tube_Drive,
            
            // Phaser Engine
            Phaser_Rate,
            Phaser_Depth,
            Phaser_Feedback,
            Phaser_Stages,
            Phaser_Frequency
        };
        
        Parameter parameter;
        float minValue;
        float maxValue;
        bool useLogScale = false;
        
        float mapPaintValue(float paintValue) const;
    };
    
    void setupDefaultParameterMappings();
    void applyPaintToParameters(const PaintPoint& point);
    
    std::array<ParameterMapping, 16> xAxisMappings;  // X-axis parameter mappings
    std::array<ParameterMapping, 16> yAxisMappings;  // Y-axis parameter mappings
    std::array<ParameterMapping, 8> pressureMappings; // Pressure mappings
    std::array<ParameterMapping, 4> colorMappings;    // Color-based mappings
    
    //==============================================================================
    // Performance & Memory Management
    
    // Performance monitoring
    juce::Time lastProcessTime;
    std::atomic<float> currentCPUUsage{0.0f};
    std::atomic<int> droppedSampleCount{0};
    
    // Memory management
    static constexpr int MAX_ACTIVE_STROKES = 50;
    void cullOldStrokes();
    void optimizeMemoryUsage();
    
    // A/B State management
    juce::MemoryBlock slotA, slotB;
    std::atomic<bool> hasSlotA{false};
    std::atomic<bool> hasSlotB{false};
    
    //==============================================================================
    // Thread Safety
    
    juce::CriticalSection strokeLock;
    juce::CriticalSection parameterLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RetroCanvasProcessor)
};

//==============================================================================
// Extended Command IDs for RetroCanvas Pro

enum class RetroCanvasCommandID
{
    // Paint commands
    BeginPaintStroke = 400,
    UpdatePaintStroke,
    EndPaintStroke,
    ClearCanvas,
    SetCanvasRegion,
    SetPlayheadPosition,
    
    // Engine commands
    SetProcessingMode,
    SetEMUSample,
    SetTubeGlow,
    SetPhaserRate,
    
    // Macro commands
    SetCharacterMacro,
    SetMotionMacro,
    SetWidthMacro,
    SetVibeMacro,
    
    // A/B commands
    SaveToSlotA,
    SaveToSlotB,
    LoadFromSlotA,
    LoadFromSlotB,
    MorphSlots,
    
    // Utility commands
    RandomizeParameters,
    SavePreset,
    LoadPreset
};