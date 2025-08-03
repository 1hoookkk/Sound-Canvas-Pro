#pragma once
#include <JuceHeader.h>
#include <memory>
#include <atomic>
#include <vector>

/**
 * GPU Acceleration Engine - Real-Time Performance Revolution
 * 
 * Leverages modern GPU compute power to achieve sub-5ms paint-to-audio latency
 * for complex operations that would be impossible on CPU alone.
 * 
 * Core Innovation:
 * - GPU-accelerated spectral analysis and synthesis
 * - Parallel processing of multiple paint masks
 * - Real-time granular synthesis with hundreds of grains
 * - Massive oscillator counts (1000+) for dense textures
 * - AI model inference acceleration
 * 
 * Supported APIs: Metal (macOS/iOS), CUDA (Windows/Linux), OpenCL (fallback)
 */
class GPUAccelerationEngine
{
public:
    GPUAccelerationEngine();
    ~GPUAccelerationEngine();
    
    //==============================================================================
    // GPU Initialization & Management
    
    enum class GPUApi
    {
        None = 0,       // CPU fallback
        Metal,          // macOS/iOS optimal
        CUDA,           // NVIDIA GPUs
        OpenCL,         // Cross-platform fallback
        DirectCompute   // Windows/DirectX
    };
    
    bool initialize();
    void shutdown();
    bool isGPUAvailable() const { return gpuAvailable.load(); }
    GPUApi getCurrentAPI() const { return currentAPI; }
    
    // GPU device information
    struct GPUInfo
    {
        juce::String deviceName;
        size_t totalMemoryMB = 0;
        size_t availableMemoryMB = 0;
        int computeUnits = 0;
        float computeCapability = 0.0f;
        bool supportsDoublePrecision = false;
        bool supportsHalfPrecision = false;
    };
    
    GPUInfo getGPUInfo() const;
    
    //==============================================================================
    // Real-Time Audio Processing
    
    // GPU-accelerated sample masking
    struct MaskingGPUData
    {
        const float* sampleData;
        size_t sampleLength;
        const float* maskPath;          // Paint path as float array
        size_t maskPoints;
        int maskType;                   // Volume, filter, granular, etc.
        float intensity;
        float* outputBuffer;
        size_t outputLength;
    };
    
    bool processSampleMasking(const MaskingGPUData& data);
    void batchProcessMasks(const std::vector<MaskingGPUData>& masks);
    
    // GPU-accelerated oscillator synthesis  
    struct OscillatorBankGPU
    {
        struct OscillatorGPU
        {
            float frequency;
            float amplitude;
            float phase;
            float pan;
            int waveform;           // 0=sine, 1=saw, 2=square, etc.
            bool isActive;
        };
        
        std::vector<OscillatorGPU> oscillators;
        float* outputBuffer;
        size_t bufferSize;
        int numChannels;
        float sampleRate;
    };
    
    bool synthesizeOscillatorBank(OscillatorBankGPU& bank);
    void updateOscillatorParameters(int oscillatorIndex, float freq, float amp, float pan);
    
    //==============================================================================
    // Spectral Processing
    
    // GPU-accelerated FFT analysis
    struct SpectralAnalysisGPU
    {
        const float* inputBuffer;
        size_t inputLength;
        float* magnitudeBuffer;     // Output magnitude spectrum
        float* phaseBuffer;         // Output phase spectrum
        size_t fftSize;
        int windowType;             // 0=Hann, 1=Hamming, 2=Blackman
        bool computePhase;
    };
    
    bool performSpectralAnalysis(const SpectralAnalysisGPU& analysis);
    
    // GPU-accelerated spectral synthesis
    struct SpectralSynthesisGPU
    {
        const float* magnitudeBuffer;
        const float* phaseBuffer;
        float* outputBuffer;
        size_t fftSize;
        size_t outputLength;
        float overlapFactor;
    };
    
    bool performSpectralSynthesis(const SpectralSynthesisGPU& synthesis);
    
    // Real-time spectral masking
    struct SpectralMaskingGPU
    {
        const float* inputSpectrum;
        const float* maskData;      // 2D mask: [frequency][time]
        float* outputSpectrum;
        size_t frequencyBins;
        size_t timeFrames;
        int maskMode;               // How to apply the mask
        float intensity;
    };
    
    bool applySpectralMasking(const SpectralMaskingGPU& masking);
    
    //==============================================================================
    // Advanced Granular Synthesis
    
    struct GranularSynthesisGPU
    {
        struct GrainGPU
        {
            double sourcePosition;      // Position in source sample
            double grainPosition;       // Current position within grain
            double grainSize;           // Grain length in samples
            float amplitude;
            float pan;
            float pitch;                // Pitch multiplier
            float envelope;             // Current envelope value
            bool isActive;
        };
        
        const float* sourceBuffer;
        size_t sourceLength;
        std::vector<GrainGPU> grains;
        float* outputBuffer;
        size_t outputLength;
        int numChannels;
        
        // Granular parameters
        float grainDensity;             // Grains per second
        float grainSize;                // Average grain size
        float grainSizeVariation;       // Size randomization
        float pitchVariation;           // Pitch randomization
        float positionSpread;           // Spatial randomization
    };
    
    bool processGranularSynthesis(GranularSynthesisGPU& granular);
    void updateGranularParameters(float density, float size, float variation);
    
    //==============================================================================
    // Paint Stroke Processing
    
    // GPU-accelerated paint stroke analysis
    struct PaintStrokeGPU
    {
        struct PaintPoint
        {
            float x, y;
            float pressure;
            float velocity;
            float timestamp;
        };
        
        std::vector<PaintPoint> points;
        float strokeLength;
        float averagePressure;
        float averageVelocity;
        float curvature;                // How curved the stroke is
        float smoothness;               // How smooth the stroke is
    };
    
    bool analyzePaintStroke(PaintStrokeGPU& stroke);
    void generateAudioFromStroke(const PaintStrokeGPU& stroke, float* outputBuffer, size_t length);
    
    // Real-time paint path interpolation
    void interpolatePaintPath(const std::vector<juce::Point<float>>& inputPoints,
                            std::vector<juce::Point<float>>& outputPoints,
                            int targetPointCount);
    
    //==============================================================================
    // AI Model Acceleration
    
    // GPU-accelerated AI inference
    struct AIInferenceGPU
    {
        const float* inputData;
        size_t inputSize;
        float* outputData;
        size_t outputSize;
        const float* modelWeights;
        size_t weightsSize;
        int modelType;                  // 0=classification, 1=regression, etc.
    };
    
    bool runAIInference(const AIInferenceGPU& inference);
    void loadAIModel(const juce::String& modelPath);
    void unloadAIModel();
    
    //==============================================================================
    // Performance & Memory Management
    
    struct PerformanceMetrics
    {
        float gpuUtilization = 0.0f;        // 0.0-1.0
        size_t memoryUsedMB = 0;
        size_t memoryAvailableMB = 0;
        float averageLatencyMs = 0.0f;
        int processedFrames = 0;
        int droppedFrames = 0;
        
        // Detailed timing
        float maskingTimeMs = 0.0f;
        float spectralTimeMs = 0.0f;
        float granularTimeMs = 0.0f;
        float aiInferenceTimeMs = 0.0f;
    };
    
    PerformanceMetrics getPerformanceMetrics() const;
    void resetPerformanceCounters();
    
    // Memory management
    bool allocateGPUBuffer(size_t sizeBytes, void** buffer);
    void freeGPUBuffer(void* buffer);
    void optimizeMemoryUsage();
    
    // Adaptive quality based on performance
    void setQualityMode(int mode);      // 0=Performance, 1=Balanced, 2=Quality
    int getQualityMode() const { return currentQualityMode.load(); }
    
    //==============================================================================
    // Compute Shader Management
    
    enum class ShaderType
    {
        SampleMasking,
        OscillatorSynthesis,
        SpectralAnalysis,
        SpectralSynthesis,
        GranularSynthesis,
        PaintStrokeAnalysis,
        AIInference
    };
    
    bool loadComputeShader(ShaderType type, const juce::String& shaderSource);
    void unloadComputeShader(ShaderType type);
    bool isShaderLoaded(ShaderType type) const;
    
private:
    //==============================================================================
    // Platform-Specific Implementations
    
    #ifdef JUCE_MAC
    class MetalImplementation;
    std::unique_ptr<MetalImplementation> metalImpl;
    #endif
    
    #ifdef JUCE_WINDOWS
    class CUDAImplementation;
    class DirectComputeImplementation;
    std::unique_ptr<CUDAImplementation> cudaImpl;
    std::unique_ptr<DirectComputeImplementation> directComputeImpl;
    #endif
    
    class OpenCLImplementation;
    std::unique_ptr<OpenCLImplementation> openclImpl;
    
    //==============================================================================
    // State Management
    
    std::atomic<bool> gpuAvailable{false};
    GPUApi currentAPI = GPUApi::None;
    std::atomic<int> currentQualityMode{1}; // Balanced by default
    
    // Performance tracking
    mutable PerformanceMetrics currentMetrics;
    juce::Time lastPerformanceUpdate;
    std::atomic<float> averageFrameTime{0.0f};
    
    // Buffer management
    struct GPUBuffer
    {
        void* deviceBuffer = nullptr;
        size_t size = 0;
        bool inUse = false;
        juce::uint32 lastUsed = 0;
    };
    
    std::vector<GPUBuffer> allocatedBuffers;
    size_t totalAllocatedMemory = 0;
    size_t maxMemoryUsage = 0;
    
    //==============================================================================
    // Compute Shader Storage
    
    struct ComputeShader
    {
        void* shaderHandle = nullptr;
        juce::String source;
        bool isCompiled = false;
        juce::uint32 lastUsed = 0;
    };
    
    std::unordered_map<int, ComputeShader> loadedShaders;
    
    //==============================================================================
    // Threading & Synchronization
    
    juce::CriticalSection gpuLock;
    juce::CriticalSection bufferLock;
    juce::CriticalSection shaderLock;
    
    // Command queue for GPU operations
    struct GPUCommand
    {
        enum Type { Masking, Synthesis, Spectral, Granular, AI } type;
        void* data;
        std::function<void()> completion;
        juce::uint32 priority;
        juce::uint32 timestamp;
    };
    
    std::vector<GPUCommand> commandQueue;
    std::atomic<bool> isProcessingCommands{false};
    
    // Background processing thread
    class GPUProcessingThread : public juce::Thread
    {
    public:
        GPUProcessingThread(GPUAccelerationEngine& owner) 
            : Thread("GPU Processing"), engine(owner) {}
        void run() override;
        
    private:
        GPUAccelerationEngine& engine;
    } processingThread{*this};
    
    //==============================================================================
    // Helper Methods
    
    bool initializeMetal();
    bool initializeCUDA();
    bool initializeOpenCL();
    bool initializeDirectCompute();
    
    void detectAvailableGPUs();
    void selectBestGPU();
    
    bool compileShader(ShaderType type, const juce::String& source);
    void executeShader(ShaderType type, const void* parameters);
    
    void updatePerformanceMetrics();
    void optimizeForPerformance();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GPUAccelerationEngine)
};