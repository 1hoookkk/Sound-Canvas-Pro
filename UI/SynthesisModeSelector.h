#pragma once
#include <JuceHeader.h>

/**
 * Modern Synthesis Mode Selector Component
 * 
 * Professional tabbed interface for selecting between SpectralCanvas synthesis modes.
 * Replaces the confusing harmonic exciter slider hack with clear visual feedback.
 * 
 * Features:
 * - 5 distinct synthesis mode tabs with icons and color coding
 * - Visual feedback for active mode
 * - Mode-specific parameter displays
 * - Early 2000s DAW aesthetic with modern polish
 */
class SynthesisModeSelector : public juce::Component,
                              public juce::Button::Listener
{
public:
    //==============================================================================
    // Synthesis Modes (matching SpectralSynthEngine)
    
    enum class SynthMode
    {
        PaintSynthesis = 0,      // Paint strokes directly create audio
        SpectralOscillators,     // Frequency-based oscillator bank  
        TrackerSequencing,       // Linear tracker-style sequencing
        SampleSynthesis,         // Sample-based synthesis with paint modulation
        HybridSynthesis          // Combines all synthesis methods
    };
    
    //==============================================================================
    // Component Interface
    
    SynthesisModeSelector();
    ~SynthesisModeSelector() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Mode selection
    void setCurrentMode(SynthMode mode);
    SynthMode getCurrentMode() const { return currentMode; }
    
    // Callbacks
    std::function<void(SynthMode)> onModeChanged;
    
    //==============================================================================
    // Button Listener
    
    void buttonClicked(juce::Button* button) override;
    
private:
    //==============================================================================
    // Mode Tab Structure
    
    struct ModeTab
    {
        SynthMode mode;
        juce::String name;
        juce::String description;
        juce::Colour color;
        std::unique_ptr<juce::TextButton> button;
        bool isActive = false;
        
        ModeTab(SynthMode m, const juce::String& n, const juce::String& desc, juce::Colour c)
            : mode(m), name(n), description(desc), color(c)
        {
            button = std::make_unique<juce::TextButton>(name);
        }
    };
    
    //==============================================================================
    // Mode Configuration
    
    std::vector<std::unique_ptr<ModeTab>> modeTabs;
    SynthMode currentMode = SynthMode::HybridSynthesis;
    
    //==============================================================================
    // Visual Styling
    
    struct ModeSelectorColors
    {
        static constexpr juce::uint32 backgroundDark = 0xFF1A1A1A;
        static constexpr juce::uint32 tabInactive = 0xFF2A2A2A;
        static constexpr juce::uint32 tabActive = 0xFF404040;
        static constexpr juce::uint32 textNormal = 0xFFCCCCCC;
        static constexpr juce::uint32 textActive = 0xFFFFFFFF;
        static constexpr juce::uint32 glowCyan = 0xFF00AAFF;
        static constexpr juce::uint32 paintRed = 0xFFFF4444;
        static constexpr juce::uint32 spectralCyan = 0xFF44AAFF;
        static constexpr juce::uint32 trackerGreen = 0xFF44FF88;
        static constexpr juce::uint32 sampleOrange = 0xFFFF8844;
        static constexpr juce::uint32 hybridPurple = 0xFFAA44FF;
    };
    
    //==============================================================================
    // Helper Methods
    
    void setupModeTabs();
    void updateTabStates();
    void setupTabButton(ModeTab& tab);
    juce::Rectangle<int> getTabBounds(int tabIndex) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthesisModeSelector)
};