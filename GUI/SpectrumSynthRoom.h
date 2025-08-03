#pragma once

#include <JuceHeader.h>
#include "ArtefactLookAndFeel.h"

/**
 * SpectrumSynthRoom - MetaSynth-style visual spectrogram painting interface
 * 
 * Allows users to paint directly on a frequency/time canvas to create sound
 * Core MetaSynth functionality with real-time visual feedback
 */
class SpectrumSynthRoom : public juce::Component
{
public:
    SpectrumSynthRoom();
    ~SpectrumSynthRoom() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    // Spectrogram canvas area
    juce::Rectangle<int> spectrogramArea;
    
    // Painting data - stores the painted spectrogram
    juce::Image spectrogramImage;
    
    // Current brush settings
    float brushSize = 10.0f;
    juce::Colour brushColor = juce::Colour(0xffffff00); // Yellow by default
    
    // Frequency and time ranges
    float minFreq = 80.0f;
    float maxFreq = 8000.0f;
    float timeRange = 8.0f; // 8 seconds
    
    // UI Controls
    juce::Slider brushSizeSlider;
    juce::Slider frequencyRangeSlider;
    juce::Slider timeRangeSlider;
    juce::TextButton clearButton;
    juce::TextButton playButton;
    juce::Label roomTitle;
    
    // Helper functions
    void initializeSpectrogram();
    void paintOnSpectrogram(juce::Point<int> position);
    juce::Point<float> screenToSpectrogram(juce::Point<int> screenPos);
    
    std::unique_ptr<ArtefactLookAndFeel> lookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumSynthRoom)
};