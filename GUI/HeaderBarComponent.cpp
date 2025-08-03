#include "HeaderBarComponent.h"
#include "Core/PluginProcessor.h"
#include "ExportDialog.h"
#include "ArtefactLookAndFeel.h"

//==============================================================================

//==============================================================================
HeaderBarComponent::HeaderBarComponent(ARTEFACTAudioProcessor& processorToUse)
    : processor(processorToUse)
{
    // Set up record button
    addAndMakeVisible(recordButton);
    recordButton.addListener(this);
    recordButton.setToggleable(false);
    recordButton.setColour(juce::TextButton::buttonColourId, ArtefactLookAndFeel::kPanelBackground);
    recordButton.setColour(juce::TextButton::textColourOffId, ArtefactLookAndFeel::kPrimaryGreen);
    recordButton.setColour(juce::TextButton::textColourOnId, ArtefactLookAndFeel::kWarningRed);
    
    // Set up export button
    addAndMakeVisible(exportButton);
    exportButton.addListener(this);
    exportButton.setColour(juce::TextButton::buttonColourId, ArtefactLookAndFeel::kPanelBackground);
    exportButton.setColour(juce::TextButton::textColourOffId, ArtefactLookAndFeel::kAccentColour);
    
    // Set up status labels
    addAndMakeVisible(statusLabel);
    addAndMakeVisible(timeLabel);
    
    statusLabel.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kPrimaryGreen);
    timeLabel.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kAccentColour);
    
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    timeLabel.setJustificationType(juce::Justification::centredRight);
    
    // Set terminal fonts
    auto terminalFont = createTerminalFont(11.0f);
    statusLabel.setFont(terminalFont);
    timeLabel.setFont(terminalFont);
    
    // Setup BPM sync controls
    setupBPMControls();
    setupKeyFilterControls();
    setupProjectControls();
    
    // Start timer for updates
    startTimerHz(10); // 10Hz for LED blinking and beat indicator
}

HeaderBarComponent::~HeaderBarComponent()
{
    stopTimer();
    recordButton.removeListener(this);
    exportButton.removeListener(this);
}

//==============================================================================
void HeaderBarComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Fill background with SpectralCanvas theme
    g.fillAll(ArtefactLookAndFeel::kBackground);
    
    // Draw top and bottom border lines
    g.setColour(ArtefactLookAndFeel::kAccentColour);
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));
    g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));
    
    // Draw SpectralCanvas title
    auto titleArea = bounds.removeFromLeft(200);
    g.setFont(createTerminalFont(16.0f));
    g.setColour(ArtefactLookAndFeel::kTextColour);
    g.drawText("◊ SpectralCanvas ◊", titleArea, juce::Justification::centredLeft);
    
    // Draw recording LED if recording
    if (isRecordingState)
    {
        auto ledArea = juce::Rectangle<int>(180, 8, 12, 12);
        drawTerminalLED(g, ledArea, ArtefactLookAndFeel::kWarningRed, ledState);
    }
    
    // Draw system status on the right
    auto rightArea = bounds.removeFromRight(300);
    g.setFont(createTerminalFont(9.0f));
    g.setColour(ArtefactLookAndFeel::kTextColour.withAlpha(0.7f));
    
    // Show CPU load and buffer status
    auto& paintEngine = processor.getPaintEngine();
    juce::String sysInfo = juce::String::formatted("CPU: %.1f%% | OSC: %d/1024", 
                                                   paintEngine.getCurrentCPULoad() * 100.0f,
                                                   paintEngine.getActiveOscillatorCount());
    g.drawText(sysInfo, rightArea.removeFromTop(15), juce::Justification::centredRight);
}

void HeaderBarComponent::resized()
{
    auto bounds = getLocalBounds().reduced(5, 5);
    
    // Left section: Record/Export controls
    recordButton.setBounds(bounds.removeFromLeft(80));
    bounds.removeFromLeft(5);
    exportButton.setBounds(bounds.removeFromLeft(80));
    bounds.removeFromLeft(10);
    
    // Project controls (compact)
    saveButton.setBounds(bounds.removeFromLeft(50));
    bounds.removeFromLeft(2);
    loadButton.setBounds(bounds.removeFromLeft(50));
    bounds.removeFromLeft(2);
    undoButton.setBounds(bounds.removeFromLeft(25));
    bounds.removeFromLeft(2);
    redoButton.setBounds(bounds.removeFromLeft(25));
    bounds.removeFromLeft(15);
    
    // BPM Sync section in center
    auto bpmArea = bounds.removeFromLeft(200);
    auto topRow = bpmArea.removeFromTop(bpmArea.getHeight() / 2);
    auto bottomRow = bpmArea;
    
    // Top row: BPM controls
    bpmLabel.setBounds(topRow.removeFromLeft(35));
    bpmSlider.setBounds(topRow.removeFromLeft(100));
    topRow.removeFromLeft(5);
    tapTempoButton.setBounds(topRow.removeFromLeft(40));
    topRow.removeFromLeft(5);
    beatIndicator.setBounds(topRow.removeFromLeft(15));
    
    // Bottom row: Key filter
    keyLabel.setBounds(bottomRow.removeFromLeft(35));
    keyComboBox.setBounds(bottomRow.removeFromLeft(60));
    bottomRow.removeFromLeft(5);
    scaleComboBox.setBounds(bottomRow.removeFromLeft(80));
    
    bounds.removeFromLeft(20); // gap before status
    
    // Right section: Status and time
    statusLabel.setBounds(bounds.removeFromLeft(120));
    timeLabel.setBounds(bounds.removeFromRight(100));
}

//==============================================================================
void HeaderBarComponent::buttonClicked(juce::Button* button)
{
    if (button == &recordButton)
    {
        // Get the AudioRecorder through the processor
        // We'll need to add this accessor to the processor
        if (!isRecordingState)
        {
            // Start recording via command system
            processor.pushCommandToQueue(Command(RecordingCommandID::StartRecording));
            isRecordingState = true;
            recordButton.setButtonText("■ STOP");
            statusLabel.setText("RECORDING", juce::dontSendNotification);
            DBG("Recording started");
        }
        else
        {
            // Stop recording via command system
            processor.pushCommandToQueue(Command(RecordingCommandID::StopRecording));
            isRecordingState = false;
            recordButton.setButtonText("● REC");
            statusLabel.setText("READY", juce::dontSendNotification);
            DBG("Recording stopped");
        }
    }
    else if (button == &exportButton)
    {
        // Show export dialog
        DBG("Export button clicked");
        statusLabel.setText("EXPORT DIALOG", juce::dontSendNotification);
        
        // Generate default filename with current time
        auto now = juce::Time::getCurrentTime();
        auto defaultName = "ARTEFACT_" + now.formatted("%Y%m%d_%H%M%S");
        
        // Show modal export dialog
        auto exportSettings = ExportDialog::showExportDialog(this, defaultName);
        
        if (!exportSettings.filename.isEmpty())
        {
            // User confirmed export - send command to processor
            statusLabel.setText("EXPORTING...", juce::dontSendNotification);
            
            // Send export command with settings
            auto outputFile = exportSettings.outputDirectory.getChildFile(exportSettings.filename);
            Command exportCmd(RecordingCommandID::ExportToFile, outputFile.getFullPathName());
            exportCmd.intParam = static_cast<int>(exportSettings.format);
            processor.pushCommandToQueue(exportCmd);
            
            DBG("Export command sent: " << outputFile.getFullPathName());
            DBG("Format: " << static_cast<int>(exportSettings.format));
            
            // Simulate export completion (in real implementation, this would be triggered by the AudioRecorder)
            juce::Timer::callAfterDelay(1500, [this]() {
                statusLabel.setText("EXPORT COMPLETE", juce::dontSendNotification);
                juce::Timer::callAfterDelay(2000, [this]() {
                    statusLabel.setText("READY", juce::dontSendNotification);
                });
            });
        }
        else
        {
            // User cancelled
            statusLabel.setText("READY", juce::dontSendNotification);
        }
    }
    else if (button == &tapTempoButton)
    {
        handleTapTempo();
    }
    else if (button == &saveButton)
    {
        // TODO: Implement project save
        statusLabel.setText("SAVE PROJECT", juce::dontSendNotification);
        DBG("SpectralCanvas: Save project requested");
    }
    else if (button == &loadButton)
    {
        // TODO: Implement project load
        statusLabel.setText("LOAD PROJECT", juce::dontSendNotification);
        DBG("SpectralCanvas: Load project requested");
    }
    else if (button == &undoButton)
    {
        // TODO: Implement undo
        DBG("SpectralCanvas: Undo requested");
    }
    else if (button == &redoButton)
    {
        // TODO: Implement redo
        DBG("SpectralCanvas: Redo requested");
    }
}

void HeaderBarComponent::timerCallback()
{
    updateRecordingStatus();
    updateBeatIndicator();
    
    // Blink LED when recording
    if (isRecordingState)
    {
        ledState = !ledState;
        repaint();
    }
}

//==============================================================================
void HeaderBarComponent::updateRecordingStatus()
{
    if (isRecordingState)
    {
        // Get actual recording time from AudioRecorder
        auto& recorder = processor.getAudioRecorder();
        double recordingTime = recorder.getRecordedSeconds();
        
        timeLabel.setText(formatTime(recordingTime), juce::dontSendNotification);
        
        // Update status with buffer usage
        float bufferUsage = recorder.getBufferUsagePercent();
        if (bufferUsage > 90.0f)
        {
            statusLabel.setText("BUFFER FULL!", juce::dontSendNotification);
        }
        else if (bufferUsage > 75.0f)
        {
            statusLabel.setText("BUFFER HIGH", juce::dontSendNotification);
        }
        else
        {
            statusLabel.setText("RECORDING", juce::dontSendNotification);
        }
    }
    else
    {
        timeLabel.setText("00:00.000", juce::dontSendNotification);
        
        // Show recording info when not recording
        auto& recorder = processor.getAudioRecorder();
        if (recorder.getRecordedSeconds() > 0.0)
        {
            statusLabel.setText("READY TO EXPORT", juce::dontSendNotification);
        }
        else
        {
            statusLabel.setText("READY", juce::dontSendNotification);
        }
    }
}

void HeaderBarComponent::drawTerminalLED(juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour color, bool active)
{
    if (active)
    {
        // Draw filled circle
        g.setColour(color);
        g.fillEllipse(bounds.toFloat());
        
        // Draw glow effect
        g.setColour(color.withAlpha(0.3f));
        auto glowBounds = bounds.expanded(2);
        g.fillEllipse(glowBounds.toFloat());
    }
    else
    {
        // Draw outline only
        g.setColour(color.withAlpha(0.3f));
        g.drawEllipse(bounds.toFloat(), 1.0f);
    }
}

juce::Font HeaderBarComponent::createTerminalFont(float size) const
{
    return juce::Font(juce::FontOptions("Courier New", size, juce::Font::FontStyleFlags::bold))
           .withExtraKerningFactor(0.0f);
}

juce::String HeaderBarComponent::formatTime(double seconds) const
{
    int totalMs = static_cast<int>(seconds * 1000);
    int minutes = totalMs / 60000;
    int secs = (totalMs % 60000) / 1000;
    int ms = totalMs % 1000;
    
    return juce::String::formatted("%02d:%02d.%03d", minutes, secs, ms);
}

//==============================================================================
// BPM Sync Implementation

void HeaderBarComponent::setupBPMControls()
{
    // BPM Label
    addAndMakeVisible(bpmLabel);
    bpmLabel.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kTextColour);
    bpmLabel.setFont(createTerminalFont(10.0f));
    
    // BPM Slider
    addAndMakeVisible(bpmSlider);
    bpmSlider.setRange(60.0, 200.0, 0.1);
    bpmSlider.setValue(currentBPM);
    bpmSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    bpmSlider.onValueChange = [this]() {
        currentBPM = static_cast<float>(bpmSlider.getValue());
        // Sync BPM to processor
        processor.setTempo(currentBPM);
        DBG("SpectralCanvas BPM: " << currentBPM);
    };
    
    // Tap Tempo Button
    addAndMakeVisible(tapTempoButton);
    tapTempoButton.addListener(this);
    tapTempoButton.setColour(juce::TextButton::buttonColourId, ArtefactLookAndFeel::kAccentColour.withAlpha(0.3f));
    tapTempoButton.setColour(juce::TextButton::textColourOffId, ArtefactLookAndFeel::kTextColour);
    
    // Beat Indicator
    addAndMakeVisible(beatIndicator);
    beatIndicator.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kPrimaryGreen);
    beatIndicator.setFont(createTerminalFont(16.0f));
    beatIndicator.setJustificationType(juce::Justification::centred);
}

void HeaderBarComponent::setupKeyFilterControls()
{
    // Key Label
    addAndMakeVisible(keyLabel);
    keyLabel.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kTextColour);
    keyLabel.setFont(createTerminalFont(10.0f));
    
    // Key ComboBox
    addAndMakeVisible(keyComboBox);
    for (int i = 0; i < 12; ++i)
        keyComboBox.addItem(getKeyName(i), i + 1);
    keyComboBox.setSelectedId(1); // C
    keyComboBox.setColour(juce::ComboBox::backgroundColourId, ArtefactLookAndFeel::kPanelBackground);
    keyComboBox.setColour(juce::ComboBox::textColourId, ArtefactLookAndFeel::kTextColour);
    
    // Scale ComboBox
    addAndMakeVisible(scaleComboBox);
    scaleComboBox.addItem("Major", 1);
    scaleComboBox.addItem("Minor", 2);
    scaleComboBox.addItem("Pentatonic", 3);
    scaleComboBox.addItem("Chromatic", 4);
    scaleComboBox.setSelectedId(1); // Major
    scaleComboBox.setColour(juce::ComboBox::backgroundColourId, ArtefactLookAndFeel::kPanelBackground);
    scaleComboBox.setColour(juce::ComboBox::textColourId, ArtefactLookAndFeel::kTextColour);
}

void HeaderBarComponent::setupProjectControls()
{
    // Save Button
    addAndMakeVisible(saveButton);
    saveButton.addListener(this);
    saveButton.setColour(juce::TextButton::buttonColourId, ArtefactLookAndFeel::kPrimaryBlue.withAlpha(0.3f));
    saveButton.setColour(juce::TextButton::textColourOffId, ArtefactLookAndFeel::kTextColour);
    
    // Load Button
    addAndMakeVisible(loadButton);
    loadButton.addListener(this);
    loadButton.setColour(juce::TextButton::buttonColourId, ArtefactLookAndFeel::kPrimaryGreen.withAlpha(0.3f));
    loadButton.setColour(juce::TextButton::textColourOffId, ArtefactLookAndFeel::kTextColour);
    
    // Undo/Redo Buttons
    addAndMakeVisible(undoButton);
    addAndMakeVisible(redoButton);
    undoButton.addListener(this);
    redoButton.addListener(this);
    undoButton.setColour(juce::TextButton::buttonColourId, ArtefactLookAndFeel::kPanelBackground);
    redoButton.setColour(juce::TextButton::buttonColourId, ArtefactLookAndFeel::kPanelBackground);
}

void HeaderBarComponent::handleTapTempo()
{
    double currentTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    
    if (lastTapTime > 0.0)
    {
        double interval = currentTime - lastTapTime;
        
        // Valid tap tempo range (60-200 BPM)
        if (interval > 0.3 && interval < 1.0) // 60-200 BPM
        {
            tapTimes.push_back(currentTime);
            
            // Keep only last 4 taps for averaging
            if (tapTimes.size() > 4)
                tapTimes.erase(tapTimes.begin());
            
            // Calculate average BPM from tap intervals
            if (tapTimes.size() >= 2)
            {
                double totalInterval = tapTimes.back() - tapTimes.front();
                double avgInterval = totalInterval / (tapTimes.size() - 1);
                double newBPM = 60.0 / avgInterval;
                
                // Update BPM
                currentBPM = static_cast<float>(juce::jlimit(60.0, 200.0, newBPM));
                bpmSlider.setValue(currentBPM, juce::dontSendNotification);
                processor.setTempo(currentBPM);
                
                DBG("SpectralCanvas Tap Tempo: " << currentBPM << " BPM");
            }
        }
        else
        {
            // Reset if interval too long/short
            tapTimes.clear();
        }
    }
    
    lastTapTime = currentTime;
}

void HeaderBarComponent::updateBeatIndicator()
{
    // Get beat position from processor
    auto playHead = processor.getPlayHead();
    if (playHead != nullptr)
    {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        if (playHead->getCurrentPosition(posInfo))
        {
            if (posInfo.isPlaying)
            {
                // Flash beat indicator on quarter notes
                double beatPosition = posInfo.ppqPosition;
                int currentBeat = static_cast<int>(beatPosition) % 4;
                bool onBeat = (beatPosition - std::floor(beatPosition)) < 0.1;
                
                if (onBeat)
                {
                    beatIndicator.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kPrimaryGreen);
                    beatIndicator.setText("●", juce::dontSendNotification);
                }
                else
                {
                    beatIndicator.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kPrimaryGreen.withAlpha(0.3f));
                    beatIndicator.setText("○", juce::dontSendNotification);
                }
                
                // Sync our BPM with host
                if (posInfo.bpm > 0)
                {
                    currentBPM = static_cast<float>(posInfo.bpm);
                    bpmSlider.setValue(currentBPM, juce::dontSendNotification);
                }
            }
            else
            {
                beatIndicator.setText("○", juce::dontSendNotification);
                beatIndicator.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kPrimaryGreen.withAlpha(0.2f));
            }
        }
    }
}

juce::String HeaderBarComponent::getKeyName(int keyIndex)
{
    const char* keyNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return juce::String(keyNames[keyIndex % 12]);
}

juce::String HeaderBarComponent::getScaleName(int scaleIndex)
{
    const char* scaleNames[] = {"Major", "Minor", "Pentatonic", "Chromatic"};
    return juce::String(scaleNames[scaleIndex % 4]);
}
