#pragma once

#include <JuceHeader.h>
#include "ArtefactLookAndFeel.h"

/**
 * MusicalFiltersRoom - MetaSynth-style musical filters and spectral effects
 * 
 * Features:
 * - Scale-based filters (filters follow musical scales)
 * - Harmonic filters (filters based on harmonic series)
 * - Spectral morphing between audio sources
 * - Resonant filters for self-oscillation
 * - Real-time spectral freezing
 */
class MusicalFiltersRoom : public juce::Component
{
public:
    MusicalFiltersRoom();
    ~MusicalFiltersRoom() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Audio processing interface
    void processBlock(juce::AudioBuffer<float>& buffer);
    void prepareToPlay(double sampleRate, int samplesPerBlock);

private:
    //==============================================================================
    // Musical Filter Types
    enum FilterType
    {
        ScaleBased = 0,     // Filters follow musical scales
        HarmonicSeries,     // Filters based on harmonic overtones
        ResonantSweep,      // Sweeping resonant filters
        SpectralMorph       // Spectral domain morphing
    };

    //==============================================================================
    // UI Controls - MetaSynth style layout
    
    // Filter Selection
    juce::ComboBox filterTypeSelector;
    juce::Label filterTypeLabel;
    
    // Musical Scale Controls
    juce::ComboBox rootNoteSelector;     // C, C#, D, etc.
    juce::ComboBox scaleTypeSelector;    // Major, Minor, Pentatonic, etc.
    juce::Label rootNoteLabel;
    juce::Label scaleTypeLabel;
    
    // Filter Parameters
    juce::Slider resonanceSlider;        // Filter resonance/Q
    juce::Slider cutoffSlider;           // Base cutoff frequency
    juce::Slider spreadSlider;           // Harmonic spread for series filters
    juce::Slider morphSlider;            // Spectral morph amount
    
    // Filter Bank Controls
    juce::Slider numFiltersSlider;       // Number of filters in bank
    juce::Slider detuneSlider;           // Slight detuning for organic feel
    
    // Visual Feedback
    juce::Component spectrumDisplay;     // Real-time spectrum analyzer
    juce::Component filterResponseDisplay; // Filter frequency response
    
    // Action Buttons
    juce::TextButton freezeButton;       // Freeze current spectrum
    juce::TextButton clearButton;        // Clear all processing
    juce::TextButton randomizeButton;    // Randomize filter settings
    
    juce::Label roomTitle;

    //==============================================================================
    // DSP Components
    
    // Filter bank for musical filters
    std::vector<std::unique_ptr<juce::dsp::StateVariableTPTFilter<float>>> filterBank;
    
    // FFT for spectral processing
    std::unique_ptr<juce::dsp::FFT> fft;
    std::vector<std::complex<float>> fftBuffer;
    std::vector<float> window;           // Hann window for FFT
    std::vector<std::complex<float>> frozenSpectrum; // For spectral freezing
    
    // Audio buffers for spectral morphing
    juce::AudioBuffer<float> morphSourceA;
    juce::AudioBuffer<float> morphSourceB;
    
    // Parameters
    float currentSampleRate = 44100.0f;
    int currentBlockSize = 512;
    int fftSize = 2048;
    int fftOrder = 11; // 2^11 = 2048
    
    FilterType currentFilterType = ScaleBased;
    int rootNote = 60;               // Middle C
    int scaleType = 0;               // Major scale
    float resonance = 0.707f;
    float baseFrequency = 440.0f;
    int numFilters = 8;
    
    //==============================================================================
    // Scale Definitions
    struct MusicalScales
    {
        static const std::vector<std::vector<int>> scales;
        static const std::vector<juce::String> scaleNames;
        static const std::vector<juce::String> noteNames;
    };
    
    //==============================================================================
    // Helper Functions
    void initializeFilters();
    void updateFilterFrequencies();
    void updateFilterType();
    void processScaleBasedFilters(juce::AudioBuffer<float>& buffer);
    void processHarmonicFilters(juce::AudioBuffer<float>& buffer);
    void processSpectralMorph(juce::AudioBuffer<float>& buffer);
    void performSpectralFreeze();
    void randomizeParameters();
    
    // Utility functions
    float noteToFrequency(int midiNote);
    void applyWindow(juce::AudioBuffer<float>& buffer);
    
    std::unique_ptr<ArtefactLookAndFeel> lookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicalFiltersRoom)
};