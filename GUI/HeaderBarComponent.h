#pragma once
#include <JuceHeader.h>
#include "Core/AudioRecorder.h"

// Forward declaration
class ARTEFACTAudioProcessor;

/**
 * HeaderBarComponent - SpectralCanvas UltraThink Command Center
 * 
 * Features:
 * - Modern SpectralCanvas branding
 * - BPM sync hub with tap tempo and beat indicator
 * - Key filter system for musical constraint
 * - Recording controls with visual feedback
 * - Project management (save/load/undo/redo)
 */
class HeaderBarComponent : public juce::Component, 
                          public juce::Button::Listener,
                          public juce::Timer
{
public:
    HeaderBarComponent(ARTEFACTAudioProcessor& processor);
    ~HeaderBarComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

private:
    //==============================================================================
    // UltraThink Components - Command Center Layout
    
    // Branding
    juce::Label titleLabel {"title", "SpectralCanvas"};
    
    // BPM Sync Hub
    juce::Label bpmLabel {"bpm", "BPM"};
    juce::Slider bpmSlider;
    juce::TextButton tapTempoButton {"TAP"};
    juce::Label beatIndicator {"beat", "●"};
    
    // Key Filter System
    juce::Label keyLabel {"key", "KEY"};
    juce::ComboBox keyComboBox;
    juce::ComboBox scaleComboBox;
    
    // Recording Controls
    juce::TextButton recordButton {"● REC"};
    juce::TextButton exportButton {"EXPORT"};
    juce::Label statusLabel {"status", "READY"};
    juce::Label timeLabel {"time", "00:00.000"};
    
    // Project Management
    juce::TextButton saveButton {"SAVE"};
    juce::TextButton loadButton {"LOAD"};
    juce::TextButton undoButton {"↶"};
    juce::TextButton redoButton {"↷"};
    
    //==============================================================================
    // State
    ARTEFACTAudioProcessor& processor;
    bool isRecordingState = false;
    bool ledState = false; // For blinking LED
    
    // Musical state
    float currentBPM = 120.0f;
    int currentKey = 0; // C = 0, C# = 1, etc.
    int currentScale = 0; // Major = 0, Minor = 1, etc.
    bool isSyncEnabled = true;
    
    // Tap tempo state
    std::vector<double> tapTimes;
    double lastTapTime = 0.0;
    
    //==============================================================================
    // Helpers
    void updateRecordingStatus();
    void setupBPMControls();
    void setupKeyFilterControls(); 
    void setupProjectControls();
    void handleTapTempo();
    void updateBeatIndicator();
    juce::String getKeyName(int keyIndex);
    juce::String getScaleName(int scaleIndex);
    void drawTerminalLED(juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour color, bool active);
    juce::Font createTerminalFont(float size) const;
    juce::String formatTime(double seconds) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBarComponent)
};
