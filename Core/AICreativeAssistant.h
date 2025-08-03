#pragma once
#include <JuceHeader.h>
#include <memory>
#include <atomic>
#include <vector>
#include <unordered_map>

/**
 * AI Creative Assistant - The Revolutionary Intelligence Layer
 * 
 * This is the brain that makes RetroCanvas Pro truly revolutionary.
 * It learns from user behavior, analyzes audio content, and provides
 * intelligent creative suggestions that spark new ideas.
 * 
 * Core Innovation:
 * - Learns personal production style and preferences
 * - Analyzes audio content for intelligent suggestions
 * - Provides contextual creative assistance
 * - Enables AI-human collaborative music creation
 * - Connects to community knowledge for inspiration
 */
class AICreativeAssistant
{
public:
    AICreativeAssistant();
    ~AICreativeAssistant();
    
    //==============================================================================
    // AI Lifecycle & Learning
    
    void initialize();
    void trainFromUserSession();
    void updateGlobalKnowledge();
    void shutdown();
    
    //==============================================================================
    // Smart Analysis Engine
    
    struct AudioAnalysis
    {
        // Basic audio characteristics
        float tempo = 120.0f;
        juce::String key = "C";
        juce::String scale = "Major";
        float energy = 0.5f;           // 0.0-1.0
        float rhythmComplexity = 0.5f; // 0.0-1.0
        float harmonicContent = 0.5f;  // 0.0-1.0
        
        // Genre detection
        juce::String primaryGenre = "Unknown";
        std::vector<std::pair<juce::String, float>> genreConfidence; // Genre + confidence
        
        // Frequency analysis
        std::vector<float> frequencySpectrum;
        std::vector<float> dominantFrequencies;
        bool hasFrequencyConflicts = false;
        
        // Rhythmic analysis
        std::vector<float> onsetStrengths;
        float syncopation = 0.0f;
        float groove = 0.5f;
        
        // Emotional characteristics
        float valence = 0.5f;  // Happy/sad (0.0-1.0)
        float arousal = 0.5f;  // Calm/energetic (0.0-1.0)
        float danceability = 0.5f;
    };
    
    // Real-time audio analysis
    AudioAnalysis analyzeAudioBuffer(const juce::AudioBuffer<float>& buffer);
    AudioAnalysis analyzeProject();
    void updateContinuousAnalysis(const juce::AudioBuffer<float>& buffer);
    
    //==============================================================================
    // Smart Masking Suggestions
    
    struct MaskingSuggestion
    {
        enum class Type
        {
            VolumePattern,      // Rhythmic volume patterns
            FilterSweep,        // Musical filter movements
            GranularTexture,    // Textural granular effects
            RhythmicChop,       // Beat-aligned chopping
            HarmonicReverse,    // Harmonically-aware reverse zones
            CreativeStutter,    // Musically-timed stutters
            SpectralMask,       // Frequency-specific masking
            DynamicDelay        // Tempo-synced delay patterns
        };
        
        Type type;
        juce::String description;
        juce::Path suggestedPath;        // Paint path to achieve effect
        float confidence = 0.0f;        // How confident AI is (0.0-1.0)
        juce::String reasoning;          // Why this suggestion makes sense
        
        // Parameters for the suggested effect
        std::unordered_map<juce::String, float> parameters;
        
        // Musical context
        bool isTempoSynced = false;
        float beatAlignment = 1.0f;     // Quarter note = 1.0, eighth = 0.5, etc.
        bool respectsKey = true;
        bool enhancesGroove = true;
    };
    
    // Generate intelligent masking suggestions
    std::vector<MaskingSuggestion> suggestMaskingForSample(
        const juce::AudioBuffer<float>& sample,
        const AudioAnalysis& context
    );
    
    void applyMaskingSuggestion(const MaskingSuggestion& suggestion);
    void learnFromAppliedSuggestion(const MaskingSuggestion& suggestion, bool wasAccepted);
    
    //==============================================================================
    // Smart Tracker Intelligence
    
    struct TrackerSuggestion
    {
        enum class Type
        {
            FrequencyOptimization,  // Suggest better frequency separation
            RhythmCompletion,       // Complete rhythm patterns
            PolyrhythmicLayer,      // Add complementary polyrhythm
            HarmonicProgression,    // Suggest harmonic movement
            GenreTemplate,          // Apply genre-appropriate patterns
            GrooveEnhancement,      // Improve groove and feel
            ConflictResolution,     // Fix frequency/rhythm conflicts
            CreativeVariation       // Inspiring variations on current pattern
        };
        
        Type type;
        juce::String description;
        int targetTrack = -1;           // Which track to modify (-1 = new track)
        std::vector<int> affectedRows;  // Which pattern rows to modify
        float confidence = 0.0f;
        juce::String reasoning;
        
        // Pattern data for the suggestion
        struct PatternCell
        {
            int note = -1;
            int velocity = 64;
            int instrument = -1;
            bool isAccent = false;
        };
        
        std::vector<PatternCell> suggestedPattern;
        
        // Musical intelligence
        bool respectsKey = true;
        bool enhancesGroove = true;
        bool avoidsConflicts = true;
        float musicalTension = 0.5f;    // How much tension/release this adds
    };
    
    // Generate intelligent tracker suggestions
    std::vector<TrackerSuggestion> suggestTrackerPatterns(
        const std::vector<std::vector<int>>& currentPattern,
        const AudioAnalysis& context
    );
    
    void applyTrackerSuggestion(const TrackerSuggestion& suggestion);
    void learnFromTrackerFeedback(const TrackerSuggestion& suggestion, bool wasAccepted);
    
    //==============================================================================
    // Personal Style Learning
    
    struct UserProfile
    {
        // Musical preferences
        std::unordered_map<juce::String, float> genrePreferences;   // Genre -> preference strength
        std::unordered_map<juce::String, float> effectPreferences; // Effect -> usage frequency
        std::unordered_map<juce::String, float> keyPreferences;    // Key -> preference
        
        // Workflow patterns
        float averageSessionLength = 60.0f;    // Minutes
        float maskingToTrackerRatio = 0.5f;    // How much masking vs tracking
        float complexityPreference = 0.5f;     // Simple vs complex patterns
        float experimentalness = 0.5f;         // Conservative vs experimental
        
        // Technical preferences
        float preferredTempo = 120.0f;
        int preferredPatternLength = 16;
        bool prefersQuantization = true;
        float swingPreference = 0.0f;
        
        // Collaboration style
        bool sharesCreations = false;
        bool acceptsSuggestions = true;
        float feedbackFrequency = 0.5f;
    };
    
    UserProfile getUserProfile() const { return currentUserProfile; }
    void updateUserProfile(const juce::String& action, float value);
    void analyzeUserBehavior(const juce::String& sessionData);
    
    //==============================================================================
    // Collaborative Intelligence
    
    struct CommunityInsight
    {
        juce::String insight;
        float relevance = 0.0f;         // How relevant to current project
        int communityRating = 0;        // Community votes (can be negative)
        juce::String sourceUser;
        juce::uint32 timestamp;
        
        // Associated data
        juce::String audioFingerprint;  // To match similar audio content
        std::vector<juce::String> tags;
        juce::String difficulty;        // "Beginner", "Intermediate", "Advanced"
    };
    
    // Connect to community knowledge
    std::vector<CommunityInsight> getCommunityInsights(const AudioAnalysis& context);
    void shareMaskingPattern(const juce::Path& pattern, const juce::String& description);
    void shareTrackerPattern(const std::vector<int>& pattern, const juce::String& description);
    void rateCommunityInsight(const juce::String& insightId, int rating);
    
    //==============================================================================
    // Real-Time Creative Assistance
    
    enum class AssistanceMode
    {
        Passive,        // Only suggest when asked
        Gentle,         // Occasional helpful suggestions
        Active,         // Regular creative suggestions
        Collaborative,  // AI as co-creator
        Educational     // Teaching-focused suggestions
    };
    
    void setAssistanceMode(AssistanceMode mode) { currentAssistanceMode.store(static_cast<int>(mode)); }
    AssistanceMode getAssistanceMode() const { return static_cast<AssistanceMode>(currentAssistanceMode.load()); }
    
    // Real-time suggestion engine
    void processPaintStroke(const juce::Path& stroke, float pressure);
    void processTrackerInput(int track, int row, int note);
    void processAudioChange(const juce::AudioBuffer<float>& newAudio);
    
    // Get current suggestions
    std::vector<MaskingSuggestion> getCurrentMaskingSuggestions() const;
    std::vector<TrackerSuggestion> getCurrentTrackerSuggestions() const;
    void clearSuggestions();
    
    //==============================================================================
    // Educational Features
    
    struct Tutorial
    {
        juce::String title;
        juce::String description;
        std::vector<juce::String> steps;
        juce::String difficulty;
        float estimatedTime = 10.0f;   // Minutes
        std::vector<juce::String> tags;
        
        // Interactive elements
        bool hasInteractiveDemo = false;
        juce::String demoProjectPath;
        std::vector<juce::String> requiredSamples;
    };
    
    std::vector<Tutorial> getRecommendedTutorials(const UserProfile& profile);
    void startTutorial(const juce::String& tutorialId);
    void progressTutorial(const juce::String& stepId);
    void completeTutorial(const juce::String& tutorialId);
    
    //==============================================================================
    // Performance & Privacy
    
    // Local processing (no data leaves user's machine by default)
    void enableCloudFeatures(bool enable) { cloudFeaturesEnabled.store(enable); }
    bool areCloudFeaturesEnabled() const { return cloudFeaturesEnabled.load(); }
    
    // Performance monitoring
    float getAIProcessingLoad() const { return aiProcessingLoad.load(); }
    void setMaxAIProcessingTime(float milliseconds) { maxProcessingTime.store(milliseconds); }
    
private:
    //==============================================================================
    // AI Processing Pipeline
    
    // Audio analysis engine
    class AudioAnalyzer
    {
    public:
        AudioAnalysis analyzeBuffer(const juce::AudioBuffer<float>& buffer);
        void updateSpectralAnalysis(const juce::AudioBuffer<float>& buffer);
        void detectRhythmicPatterns(const juce::AudioBuffer<float>& buffer);
        void analyzeHarmonicContent(const juce::AudioBuffer<float>& buffer);
        
    private:
        juce::dsp::FFT fft{11}; // 2048 point FFT
        std::vector<float> fftBuffer;
        std::vector<float> magnitudeBuffer;
        
        // Onset detection
        std::vector<float> onsetStrengthBuffer;
        float previousSpectralEnergy = 0.0f;
        
        // Rhythm analysis
        std::vector<float> tempoCorrelation;
        float confidenceThreshold = 0.7f;
    } audioAnalyzer;
    
    // Machine learning models (simplified for now)
    class MLModels
    {
    public:
        float classifyGenre(const AudioAnalysis& analysis);
        std::vector<float> predictMaskingSuccess(const juce::Path& mask, const AudioAnalysis& audio);
        std::vector<int> generateTrackerPattern(const AudioAnalysis& context, int trackIndex);
        float evaluateMusicalFit(const AudioAnalysis& context, const juce::String& suggestion);
        
    private:
        // Simplified models - in real implementation would use TensorFlow Lite or similar
        std::unordered_map<juce::String, std::vector<float>> genreFeatureWeights;
        std::unordered_map<juce::String, float> effectSuccessRates;
    } mlModels;
    
    //==============================================================================
    // State Management
    
    UserProfile currentUserProfile;
    std::atomic<int> currentAssistanceMode{static_cast<int>(AssistanceMode::Gentle)};
    std::atomic<bool> cloudFeaturesEnabled{false};
    
    // Current suggestions
    std::vector<MaskingSuggestion> activeMaskingSuggestions;
    std::vector<TrackerSuggestion> activeTrackerSuggestions;
    
    // Learning data
    std::vector<std::pair<juce::String, bool>> feedbackHistory;  // Suggestion ID + accepted
    juce::Time lastLearningUpdate;
    
    //==============================================================================
    // Performance & Threading
    
    std::atomic<float> aiProcessingLoad{0.0f};
    std::atomic<float> maxProcessingTime{10.0f}; // Max 10ms for AI processing
    
    juce::CriticalSection suggestionLock;
    juce::CriticalSection profileLock;
    
    // Background processing
    class BackgroundProcessor : public juce::Thread
    {
    public:
        BackgroundProcessor(AICreativeAssistant& owner) : Thread("AI Background"), assistant(owner) {}
        void run() override;
        
    private:
        AICreativeAssistant& assistant;
    } backgroundProcessor{*this};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AICreativeAssistant)
};