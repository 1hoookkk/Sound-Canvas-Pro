#pragma once
#include <JuceHeader.h>
#include <memory>
#include <atomic>
#include <unordered_map>

/**
 * EMU Rompler Engine - "Vintage Vault"
 * 
 * Authentic recreation of classic EMU rompler sounds with modern efficiency.
 * Features curated samples from E-mu Proteus, Emulator II, and Planet Phatt.
 * 
 * Key Features:
 * - High-quality sample streaming with velocity layers
 * - Authentic EMU filter modeling with resonance
 * - Vintage converter emulation (bit reduction, aliasing)
 * - Efficient polyphonic voice management
 * - Paint interface control integration
 */
class EMURomplerEngine
{
public:
    EMURomplerEngine();
    ~EMURomplerEngine();
    
    //==============================================================================
    // Audio Processing Lifecycle
    
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void releaseResources();
    
    //==============================================================================
    // Sample Management
    
    enum class SampleCategory
    {
        Bass = 0,
        Leads,
        Pads,
        Strings,
        Brass,
        Drums,
        Textures,
        Effects
    };
    
    struct SampleInfo
    {
        juce::String name;
        SampleCategory category;
        int rootNote = 60;  // Middle C
        float baseTuning = 440.0f;
        bool hasVelocityLayers = false;
        int numVelocityLayers = 1;
        juce::File sampleFile;
        
        // EMU-specific properties
        bool useEMUFilter = true;
        float filterCutoff = 1.0f;  // 0.0-1.0
        float filterResonance = 0.0f;  // 0.0-1.0
        bool useVintageCharacter = true;
    };
    
    // Sample library management
    void loadSampleLibrary(const juce::File& libraryDirectory);
    void addSample(const SampleInfo& sampleInfo);
    void setCurrentSample(int sampleIndex);
    void setCurrentSample(const juce::String& sampleName);
    
    juce::StringArray getSampleNames(SampleCategory category = SampleCategory::Bass) const;
    SampleInfo getCurrentSampleInfo() const;
    int getNumSamples() const { return static_cast<int>(sampleLibrary.size()); }
    
    //==============================================================================
    // Voice Management & Synthesis
    
    void noteOn(int midiNote, float velocity, int voiceId = -1);
    void noteOff(int midiNote, int voiceId = -1);
    void allNotesOff();
    void sustainPedal(bool isDown);
    
    // Pitch control
    void setPitchBend(float semitones);  // -12.0 to +12.0
    void setFineTune(float cents);       // -100.0 to +100.0
    void setCoarseTune(int semitones);   // -24 to +24
    
    //==============================================================================
    // EMU-Style Parameters
    
    // Filter parameters (classic EMU resonant filter)
    void setFilterCutoff(float cutoff);      // 0.0-1.0 (maps to 100Hz-20kHz)
    void setFilterResonance(float resonance); // 0.0-1.0
    void setFilterTracking(float amount);     // 0.0-1.0 (keyboard tracking)
    void setFilterType(int type);            // 0=LPF, 1=HPF, 2=BPF, 3=Notch
    
    // Amplitude envelope (classic ADSR)
    void setAttackTime(float timeSeconds);    // 0.001-10.0
    void setDecayTime(float timeSeconds);     // 0.001-10.0
    void setSustainLevel(float level);        // 0.0-1.0
    void setReleaseTime(float timeSeconds);   // 0.001-10.0
    
    // LFO modulation
    void setLFORate(float hz);               // 0.1-20.0
    void setLFODepth(float depth);           // 0.0-1.0
    void setLFODestination(int dest);        // 0=Pitch, 1=Filter, 2=Amp
    void setLFOWaveform(int waveform);       // 0=Sine, 1=Triangle, 2=Square, 3=Saw
    
    // Vintage character controls
    void setVintageAmount(float amount);      // 0.0-1.0 (bit reduction + aliasing)
    void setConverterType(int type);         // 0=Clean, 1=EMU12bit, 2=EMU16bit
    void setAnalogNoise(float amount);       // 0.0-1.0
    
    // Master controls
    void setMasterVolume(float volume);      // 0.0-1.0
    void setPan(float pan);                  // -1.0 to +1.0
    void setPolyphony(int maxVoices);        // 1-64
    
    //==============================================================================
    // Paint Interface Integration
    
    // Map paint coordinates to EMU parameters
    void paintControl(float x, float y, float pressure, juce::Colour color);
    
    // Configure paint mappings
    void setXAxisMapping(int parameter);     // What X controls
    void setYAxisMapping(int parameter);     // What Y controls
    void setPressureMapping(int parameter);  // What pressure controls
    void setColorMapping(int parameter);     // What color controls
    
    //==============================================================================
    // Performance & Monitoring
    
    struct PerformanceInfo
    {
        int activeVoices = 0;
        float cpuUsagePercent = 0.0f;
        float memoryUsageMB = 0.0f;
        int samplesCacheHits = 0;
        int samplesCacheMisses = 0;
    };
    
    PerformanceInfo getPerformanceInfo() const;
    void resetPerformanceCounters();
    
private:
    //==============================================================================
    // Voice Management
    
    class EMUVoice
    {
    public:
        EMUVoice();
        ~EMUVoice();
        
        void prepare(double sampleRate, int samplesPerBlock);
        bool renderNextBlock(juce::AudioBuffer<float>& output, int startSample, int numSamples);
        
        void startNote(int midiNote, float velocity, const SampleInfo& sample);
        void stopNote(float allowTailOff);
        void pitchWheelMoved(float newPitchWheelValue);
        
        bool isActive() const { return isPlaying || isReleasing; }
        bool isPlayingNote(int midiNote) const { return currentMidiNote == midiNote && isPlaying; }
        
        // Parameter control
        void setFilterParams(float cutoff, float resonance, int type);
        void setEnvelopeParams(float attack, float decay, float sustain, float release);
        void setLFOParams(float rate, float depth, int destination, int waveform);
        void setVintageParams(float amount, int converterType, float noiseAmount);
        
    private:
        // Voice state
        std::atomic<bool> isPlaying{false};
        std::atomic<bool> isReleasing{false};
        int currentMidiNote = -1;
        float currentVelocity = 0.0f;
        
        // Sample playback
        std::unique_ptr<juce::AudioFormatReader> sampleReader;
        double currentSamplePosition = 0.0;
        double sampleRate = 44100.0;
        float pitchRatio = 1.0f;
        
        // Synthesis components
        struct AMPLIFIERenvelope
        {
            float attackRate = 0.01f;
            float decayRate = 0.1f; 
            float sustainLevel = 0.7f;
            float releaseRate = 0.3f;
            
            enum class Stage { Attack, Decay, Sustain, Release, Idle };
            Stage currentStage = Stage::Idle;
            float currentLevel = 0.0f;
            
            float getNextValue();
            void noteOn();
            void noteOff();
            void setSampleRate(double sr) { sampleRate = sr; }
            
        private:
            double sampleRate = 44100.0;
        } amplifierEnvelope;
        
        // Authentic CEM3389 4-pole resonant filter emulation
        struct CEM3389Filter
        {
            float cutoff = 1.0f;           // 0.0-1.0 normalized cutoff
            float resonance = 0.0f;        // 0.0-1.0 resonance amount
            
            // 4-pole (24dB/octave) implementation using cascaded SVF stages
            struct SVFStage
            {
                float low = 0.0f;
                float band = 0.0f;
                float high = 0.0f;
                float notch = 0.0f;
                float f = 0.0f;    // frequency coefficient
                float fb = 0.0f;   // feedback coefficient
            };
            
            SVFStage stage1, stage2;  // Two 2-pole stages = 4-pole total
            
            // CEM3389-specific characteristics
            float selfOscLevel = 0.0f;           // Self-oscillation amplitude
            float analogDrift = 0.0f;            // Frequency drift simulation
            float temperatureDrift = 0.0f;       // Temperature simulation
            float frequencyHz = 1000.0f;         // Current frequency in Hz
            float resonanceCompensation = 1.0f;  // Frequency-dependent Q
            
            // Non-linear behavior modeling
            float saturationAmount = 0.0f;      // Non-linear saturation
            float harmonicDistortion = 0.0f;    // Harmonic content at high Q
            
            // Analog drift simulation
            juce::Random driftGenerator;
            float driftPhase = 0.0f;
            
            void setSampleRate(double sr);
            void setParams(float newCutoff, float newResonance);
            float process(float input);
            
            // CEM3389-specific methods
            void updateAnalogDrift();            // Simulate analog component drift
            float applyNonLinearities(float input, float resonanceLevel);
            float calculateFrequencyDependentQ(float frequency, float baseQ);
            
        private:
            double sampleRate = 44100.0;
            float lastCutoff = -1.0f;           // For change detection
            float lastResonance = -1.0f;
        } filter;
        
        // LFO for modulation
        struct LFO
        {
            float rate = 2.0f;  // Hz
            float depth = 0.0f;
            int destination = 0; // 0=Pitch, 1=Filter, 2=Amp
            int waveform = 0;   // 0=Sine
            
            float phase = 0.0f;
            float phaseIncrement = 0.0f;
            
            void setSampleRate(double sr);
            void setRate(float hz);
            float getNextValue();
            
        private:
            double sampleRate = 44100.0;
        } lfo;
        
        // EMU Audity vintage character processing (39kHz + converter emulation)
        struct AudityVintageProcessor
        {
            float amount = 0.0f;               // Overall vintage character amount
            int converterType = 0;             // 0=Clean, 1=Audity39k, 2=AudityFull
            float noiseAmount = 0.0f;          // Analog noise level
            
            // 39kHz sample rate simulation
            float targetSampleRate = 39000.0f;    // Authentic Audity sample rate
            float currentSampleRate = 44100.0f;   // Host sample rate
            float resampleRatio = 1.0f;           // Resampling ratio
            
            // Anti-aliasing filter emulation (imperfect filter characteristic)
            struct AntiAliasingFilter
            {
                float cutoff = 19500.0f;      // Nyquist frequency for 39kHz
                float low1 = 0.0f, low2 = 0.0f;
                float f = 0.0f, fb = 0.0f;
                
                void setSampleRate(double sr);
                float process(float input);  // Gentle roll-off with phase distortion
            } antiAliasFilter;
            
            // 16-bit converter character
            float converterNonlinearity = 0.0f;   // Converter distortion
            float quantizationNoise = 0.0f;       // Quantization artifacts
            
            // Analog signal path warmth
            float analogSaturation = 0.0f;        // Subtle tube-like saturation
            float powerSupplyNoise = 0.0f;        // Power supply artifacts
            
            // Component drift simulation
            juce::Random noiseGenerator;
            juce::Random driftGenerator;
            
            void setSampleRate(double sr);
            float process(float input);
            void setParams(float vintageAmount, int converter, float noise);
            
            // Audity-specific processing
            float apply39kHzCharacter(float input);     // 39kHz resampling + aliasing
            float applyConverterCharacter(float input); // 16-bit converter emulation
            float applyAnalogWarmth(float input);       // Analog signal path
            
        } vintageProcessor;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EMUVoice)
    };
    
    //==============================================================================
    // Engine Implementation
    
    // Voice management
    static constexpr int MAX_VOICES = 64;
    std::array<std::unique_ptr<EMUVoice>, MAX_VOICES> voices;
    std::atomic<int> maxPolyphony{32};
    
    EMUVoice* findFreeVoice();
    EMUVoice* findVoicePlayingNote(int midiNote);
    void killQuietestVoice();  // Voice stealing
    
    // Sample library
    std::vector<SampleInfo> sampleLibrary;
    std::atomic<int> currentSampleIndex{0};
    juce::AudioFormatManager formatManager;
    
    // Sample caching system
    struct CacheEntry
    {
        std::unique_ptr<juce::AudioBuffer<float>> buffer;
        juce::uint32 lastAccessTime;
        bool isLoaded = false;
    };
    
    static constexpr int MAX_CACHE_SIZE = 50; // Number of samples to keep in memory
    std::unordered_map<int, CacheEntry> sampleCache;
    void manageSampleCache();
    
    // Audio processing state
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    int numChannels = 2;
    
    // Global parameters
    std::atomic<float> masterVolume{0.8f};
    std::atomic<float> masterPan{0.0f};
    std::atomic<float> pitchBend{0.0f};
    std::atomic<float> fineTune{0.0f};
    std::atomic<int> coarseTune{0};
    
    // Filter parameters (applied to all voices)
    std::atomic<float> globalFilterCutoff{1.0f};
    std::atomic<float> globalFilterResonance{0.0f};
    std::atomic<float> globalFilterTracking{1.0f};
    std::atomic<int> globalFilterType{0};
    
    // Envelope parameters
    std::atomic<float> globalAttack{0.01f};
    std::atomic<float> globalDecay{0.1f};
    std::atomic<float> globalSustain{0.7f};
    std::atomic<float> globalRelease{0.3f};
    
    // LFO parameters
    std::atomic<float> globalLFORate{2.0f};
    std::atomic<float> globalLFODepth{0.0f};
    std::atomic<int> globalLFODestination{0};
    std::atomic<int> globalLFOWaveform{0};
    
    // Vintage character
    std::atomic<float> globalVintageAmount{0.0f};
    std::atomic<int> globalConverterType{0};
    std::atomic<float> globalAnalogNoise{0.0f};
    
    // Performance monitoring
    std::atomic<float> cpuUsage{0.0f};
    std::atomic<int> cacheHits{0};
    std::atomic<int> cacheMisses{0};
    juce::Time lastProcessTime;
    
    // Thread safety
    juce::CriticalSection voiceLock;
    juce::CriticalSection cacheLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EMURomplerEngine)
};