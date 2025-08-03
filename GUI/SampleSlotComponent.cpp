// Source/GUI/SampleSlotComponent.cpp
#include "SampleSlotComponent.h"
#include "ArtefactLookAndFeel.h"
#include "Core/PluginProcessor.h"
#include <cmath>

//==============================================================================
SampleSlotComponent::SampleSlotComponent(ARTEFACTAudioProcessor& processorToUse, int slotIdx)
    : processor(processorToUse), slotIndex(slotIdx)
{
    // Set up sliders
    addAndMakeVisible(pitchSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(driveSlider);
    addAndMakeVisible(crushSlider);
    addAndMakeVisible(syncButton);
    
    // Configure sliders
    pitchSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    speedSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    volumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    crushSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    
    pitchSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    speedSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    driveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    crushSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    
    // Set initial size
    setSize(300, 40);
    
    // Start timer for updates
    startTimerHz(30);
}

SampleSlotComponent::~SampleSlotComponent()
{
    stopTimer();
}

//==============================================================================
void SampleSlotComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Fill background - SpectralCanvas modern style
    g.fillAll(ArtefactLookAndFeel::kPanelBackground);
    
    // Draw modern border
    g.setColour(ArtefactLookAndFeel::kAccentColour);
    g.drawRoundedRectangle(bounds.toFloat().reduced(1), 3.0f, 2.0f);
    
    // Draw slot number with modern font
    g.setFont(juce::Font("Tahoma", 11.0f, juce::Font::bold));
    g.setColour(ArtefactLookAndFeel::kAccentColour);
    g.drawText("SLOT " + juce::String(slotIndex + 1), bounds.removeFromTop(20), juce::Justification::centred);
    
    // Draw waveform if available - modern vibrant style
    if (!waveformPath.isEmpty())
    {
        g.setColour(ArtefactLookAndFeel::kReadoutGreen);
        g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
    }
    
    // Draw playhead - vibrant purple
    if (playheadPosition > 0.0f)
    {
        g.setColour(juce::Colour(0xff9B59B6)); // Purple playhead
        int playheadX = static_cast<int>(playheadPosition * getWidth());
        g.drawVerticalLine(playheadX, 20.0f, static_cast<float>(getHeight()));
    }
}

bool SampleSlotComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    DBG("SpectralCanvas: Drag detected over slot " << slotIndex + 1 << " with " << files.size() << " files");
    
    for (const auto& file : files)
    {
        DBG("SpectralCanvas: File: " << file);
        if (file.endsWithIgnoreCase(".wav") || 
            file.endsWithIgnoreCase(".aiff") || 
            file.endsWithIgnoreCase(".mp3") ||
            file.endsWithIgnoreCase(".flac"))
        {
            DBG("SpectralCanvas: File type accepted");
            return true;
        }
    }
    DBG("SpectralCanvas: No supported audio files found");
    return false;
}

void SampleSlotComponent::filesDropped(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(x, y);
    
    DBG("SpectralCanvas: Files dropped on slot " << slotIndex + 1 << " - " << files.size() << " files");
    
    if (files.size() > 0)
    {
        juce::File audioFile(files[0]);
        DBG("SpectralCanvas: First file: " << audioFile.getFullPathName());
        
        if (audioFile.existsAsFile())
        {
            DBG("SpectralCanvas: File exists, sending LoadSample command");
            // Send command to load sample
            processor.pushCommandToQueue(
                Command(ForgeCommandID::LoadSample, slotIndex, audioFile.getFullPathName())
            );
            DBG("SpectralCanvas: LoadSample command sent successfully");
        }
        else
        {
            DBG("SpectralCanvas: ERROR - File does not exist!");
        }
    }
    else
    {
        DBG("SpectralCanvas: ERROR - No files in drop event!");
    }
}

void SampleSlotComponent::timerCallback()
{
    updateFromProcessor();
}

void SampleSlotComponent::updateWaveformPath()
{
    waveformPath.clear();
    
    auto& voice = processor.getForgeProcessor().getVoice(slotIndex);
    if (voice.hasSample())
    {
        // Simple waveform representation
        auto bounds = getLocalBounds().removeFromBottom(getHeight() - 20);
        waveformPath.startNewSubPath(bounds.getX(), bounds.getCentreY());
        
        // Draw a simple sine wave as placeholder
        for (int x = 0; x < bounds.getWidth(); x += 4)
        {
            float y = bounds.getCentreY() + 10.0f * std::sin(x * 0.1f);
            waveformPath.lineTo(bounds.getX() + x, y);
        }
    }
}

void SampleSlotComponent::updateFromProcessor()
{
    auto& voice = processor.getForgeProcessor().getVoice(slotIndex);
    
    if (voice.hasSample() && voice.isActive())
    {
        // Update playhead position (normalized 0.0 - 1.0)
        playheadPosition = voice.getProgress();
        repaint();
    }
    else
    {
        playheadPosition = 0.0f;
    }
}

void SampleSlotComponent::resized()
{
    if (isExpanded)
    {
        const int knob = 40, gap = 60;
        pitchSlider.setBounds(20, 50, knob, knob);
        speedSlider.setBounds(20 + gap, 50, knob, knob);
        volumeSlider.setBounds(20 + gap * 2, 50, knob, knob);
        driveSlider.setBounds(20 + gap * 3, 50, knob, knob);
        crushSlider.setBounds(20 + gap * 4, 50, knob, knob);
        syncButton.setBounds(getWidth() - 60, 50, 50, 25);

        for (juce::Component* c : {
            static_cast<juce::Component*>(&pitchSlider),
            static_cast<juce::Component*>(&speedSlider),
            static_cast<juce::Component*>(&volumeSlider),
            static_cast<juce::Component*>(&driveSlider),
            static_cast<juce::Component*>(&crushSlider),
            static_cast<juce::Component*>(&syncButton)
            })
            c->setVisible(true);
    }
    else
    {
        for (juce::Component* c : {
            static_cast<juce::Component*>(&pitchSlider),
            static_cast<juce::Component*>(&speedSlider),
            static_cast<juce::Component*>(&volumeSlider),
            static_cast<juce::Component*>(&driveSlider),
            static_cast<juce::Component*>(&crushSlider),
            static_cast<juce::Component*>(&syncButton)
            })
            c->setVisible(false);
    }
}

void SampleSlotComponent::mouseDown(const juce::MouseEvent& e)
{
    auto& v = processor.getForgeProcessor().getVoice(slotIndex);

    if (e.mods.isRightButtonDown())
    {
        isExpanded = !isExpanded;
        setSize(getWidth(), isExpanded ? 100 : 40);

        for (juce::Component* c : {
            static_cast<juce::Component*>(&pitchSlider),
            static_cast<juce::Component*>(&speedSlider),
            static_cast<juce::Component*>(&volumeSlider),
            static_cast<juce::Component*>(&driveSlider),
            static_cast<juce::Component*>(&crushSlider),
            static_cast<juce::Component*>(&syncButton)
            })
            c->setVisible(isExpanded);

        resized();
        if (auto* p = getParentComponent()) p->resized();
    }
    else if (v.hasSample())
    {
        processor.pushCommandToQueue(
            Command(v.isActive() ? ForgeCommandID::StopPlayback
                : ForgeCommandID::StartPlayback,
                slotIndex));
    }
    repaint();
}
