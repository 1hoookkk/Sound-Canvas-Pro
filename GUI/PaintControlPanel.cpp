#include "PaintControlPanel.h"
#include "Core/PluginProcessor.h"
#include "Core/SpectralMask.h"
#include <cmath>

//==============================================================================
// SpectralCanvas Modern Color Definitions (Early 2000s Pro Audio Inspired)

const juce::Colour PaintControlPanel::PANEL_BACKGROUND = juce::Colour(0xffFAFAFA);  // Near white
const juce::Colour PaintControlPanel::PRIMARY_BLUE     = juce::Colour(0xff4169E1);  // Royal blue
const juce::Colour PaintControlPanel::SECONDARY_BLUE   = juce::Colour(0xff87CEEB);  // Sky blue  
const juce::Colour PaintControlPanel::TEXT_DARK        = juce::Colour(0xff2C3E50);  // Dark blue-gray
const juce::Colour PaintControlPanel::ACCENT_PURPLE    = juce::Colour(0xff9B59B6);  // Purple

//==============================================================================
// Constructor and Destructor

PaintControlPanel::PaintControlPanel(ARTEFACTAudioProcessor& processorToUse)
    : processor(processorToUse)
{
    setupControls();
    
    DBG("SpectralCanvas PaintControlPanel initialized with modern vibrant aesthetic");
}

PaintControlPanel::~PaintControlPanel() = default;

//==============================================================================
// Setup Methods

void PaintControlPanel::setupControls()
{
    // Set up brush type buttons
    addAndMakeVisible(brushLabel);
    addAndMakeVisible(sineBrushButton);
    addAndMakeVisible(harmonicBrushButton);
    addAndMakeVisible(noiseBrushButton);
    addAndMakeVisible(sampleBrushButton);
    addAndMakeVisible(granularBrushButton);
    addAndMakeVisible(cdpMorphButton);
    
    sineBrushButton.addListener(this);
    harmonicBrushButton.addListener(this);
    noiseBrushButton.addListener(this);
    sampleBrushButton.addListener(this);
    granularBrushButton.addListener(this);
    cdpMorphButton.addListener(this);
    
    sineBrushButton.setToggleable(true);
    harmonicBrushButton.setToggleable(true);
    noiseBrushButton.setToggleable(true);
    sampleBrushButton.setToggleable(true);
    granularBrushButton.setToggleable(true);
    cdpMorphButton.setToggleable(true);
    
    // Set up brush parameter sliders
    addAndMakeVisible(brushSizeLabel);
    addAndMakeVisible(brushSizeSlider);
    addAndMakeVisible(brushPressureLabel);
    addAndMakeVisible(brushPressureSlider);
    
    brushSizeSlider.setRange(0.5, 10.0, 0.1);
    brushSizeSlider.setValue(2.0);
    brushSizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    brushSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    brushSizeSlider.addListener(this);
    
    brushPressureSlider.setRange(0.0, 1.0, 0.01);
    brushPressureSlider.setValue(1.0);
    brushPressureSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    brushPressureSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    brushPressureSlider.addListener(this);
    
    // Set up sample brush controls
    addAndMakeVisible(sampleSlotLabel);
    addAndMakeVisible(sampleSlotSlider);
    
    sampleSlotSlider.setRange(1, 8, 1);  // ForgeVoice slots 1-8 (0-7 internally)
    sampleSlotSlider.setValue(1);
    sampleSlotSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    sampleSlotSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    sampleSlotSlider.addListener(this);
    
    // Set up canvas controls
    addAndMakeVisible(canvasLabel);
    addAndMakeVisible(clearCanvasButton);
    addAndMakeVisible(resetViewButton);
    
    clearCanvasButton.addListener(this);
    resetViewButton.addListener(this);
    
    // Set up frequency range controls
    addAndMakeVisible(freqRangeLabel);
    addAndMakeVisible(minFreqSlider);
    addAndMakeVisible(maxFreqSlider);
    addAndMakeVisible(minFreqValueLabel);
    addAndMakeVisible(maxFreqValueLabel);
    
    minFreqSlider.setRange(20.0, 1000.0, 1.0);
    minFreqSlider.setValue(80.0);
    minFreqSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    minFreqSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    minFreqSlider.addListener(this);
    
    maxFreqSlider.setRange(1000.0, 20000.0, 10.0);
    maxFreqSlider.setValue(8000.0);
    maxFreqSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    maxFreqSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    maxFreqSlider.addListener(this);
    
    // Set up master controls
    addAndMakeVisible(masterLabel);
    addAndMakeVisible(masterGainSlider);
    addAndMakeVisible(masterGainValueLabel);
    addAndMakeVisible(paintActiveButton);
    
    masterGainSlider.setRange(0.0, 2.0, 0.01);
    masterGainSlider.setValue(0.7);
    masterGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    masterGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterGainSlider.addListener(this);
    
    paintActiveButton.setToggleable(true);
    paintActiveButton.setToggleState(false, juce::dontSendNotification);
    paintActiveButton.addListener(this);
    
    // Set up mode controls
    addAndMakeVisible(modeLabel);
    addAndMakeVisible(canvasModeButton);
    addAndMakeVisible(forgeModeButton);
    addAndMakeVisible(hybridModeButton);
    
    canvasModeButton.addListener(this);
    forgeModeButton.addListener(this);
    hybridModeButton.addListener(this);
    
    canvasModeButton.setToggleable(true);
    forgeModeButton.setToggleable(true);
    hybridModeButton.setToggleable(true);
    
    // Set up spectral masking controls (MetaSynth-style)
    addAndMakeVisible(spectralMaskLabel);
    addAndMakeVisible(spectralMaskEnableButton);
    addAndMakeVisible(maskSourceLabel);
    addAndMakeVisible(maskSourceSlider);
    addAndMakeVisible(maskTypeLabel);
    addAndMakeVisible(maskTypeComboBox);
    addAndMakeVisible(maskStrengthLabel);
    addAndMakeVisible(maskStrengthSlider);
    addAndMakeVisible(timeStretchLabel);
    addAndMakeVisible(timeStretchSlider);
    
    spectralMaskEnableButton.addListener(this);
    spectralMaskEnableButton.setToggleable(true);
    
    maskSourceSlider.setRange(1, 8, 1);  // ForgeVoice slots 1-8
    maskSourceSlider.setValue(1);
    maskSourceSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    maskSourceSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 30, 20);
    maskSourceSlider.addListener(this);
    
    maskTypeComboBox.addItem("Off", 1);
    maskTypeComboBox.addItem("Spectral Gate", 2);
    maskTypeComboBox.addItem("Spectral Filter", 3);
    maskTypeComboBox.addItem("Rhythmic Gate", 4);
    maskTypeComboBox.addItem("Spectral Morph", 5);
    maskTypeComboBox.setSelectedId(1);
    maskTypeComboBox.addListener(this);
    
    maskStrengthSlider.setRange(0.0, 1.0, 0.01);
    maskStrengthSlider.setValue(0.7);
    maskStrengthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    maskStrengthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    maskStrengthSlider.addListener(this);
    
    timeStretchSlider.setRange(0.1, 4.0, 0.1);
    timeStretchSlider.setValue(1.0);
    timeStretchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    timeStretchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    timeStretchSlider.addListener(this);
    
    // Set initial brush selection
    updateBrushButtons();
    updateModeButtons();
    updateSpectralMaskControls();
    
    // Style all labels with clean modern font
    auto modernFont = createModernFont(11.0f);
    brushLabel.setFont(modernFont.withStyle(juce::Font::bold));
    brushSizeLabel.setFont(modernFont);
    brushPressureLabel.setFont(modernFont);
    canvasLabel.setFont(modernFont.withStyle(juce::Font::bold));
    freqRangeLabel.setFont(modernFont.withStyle(juce::Font::bold));
    masterLabel.setFont(modernFont.withStyle(juce::Font::bold));
    modeLabel.setFont(modernFont.withStyle(juce::Font::bold));
    minFreqValueLabel.setFont(modernFont);
    maxFreqValueLabel.setFont(modernFont);
    spectralMaskLabel.setFont(modernFont.withStyle(juce::Font::bold));
    maskSourceLabel.setFont(modernFont);
    maskTypeLabel.setFont(modernFont);
    maskStrengthLabel.setFont(modernFont);
    timeStretchLabel.setFont(modernFont);
    masterGainValueLabel.setFont(modernFont);
    
    // Set vibrant modern label colors
    brushLabel.setColour(juce::Label::textColourId, PRIMARY_BLUE);
    brushSizeLabel.setColour(juce::Label::textColourId, TEXT_DARK);
    brushPressureLabel.setColour(juce::Label::textColourId, TEXT_DARK);
    canvasLabel.setColour(juce::Label::textColourId, PRIMARY_BLUE);
    freqRangeLabel.setColour(juce::Label::textColourId, PRIMARY_BLUE);
    masterLabel.setColour(juce::Label::textColourId, PRIMARY_BLUE);
    modeLabel.setColour(juce::Label::textColourId, PRIMARY_BLUE);
    minFreqValueLabel.setColour(juce::Label::textColourId, ACCENT_PURPLE);
    maxFreqValueLabel.setColour(juce::Label::textColourId, ACCENT_PURPLE);
    masterGainValueLabel.setColour(juce::Label::textColourId, ACCENT_PURPLE);
    spectralMaskLabel.setColour(juce::Label::textColourId, ACCENT_PURPLE);
    maskSourceLabel.setColour(juce::Label::textColourId, TEXT_DARK);
    maskTypeLabel.setColour(juce::Label::textColourId, TEXT_DARK);
    maskStrengthLabel.setColour(juce::Label::textColourId, TEXT_DARK);
    timeStretchLabel.setColour(juce::Label::textColourId, TEXT_DARK);
}

//==============================================================================
// Component Overrides

void PaintControlPanel::paint(juce::Graphics& g)
{
    // Modern clean background
    g.fillAll(PANEL_BACKGROUND);
    
    auto bounds = getLocalBounds().reduced(8);
    
    // Draw modern panel sections with subtle borders
    int yPos = 15;
    
    // Brush section
    auto brushSection = juce::Rectangle<int>(bounds.getX(), yPos, bounds.getWidth(), 180);
    drawModernSection(g, brushSection, "BRUSH", PRIMARY_BLUE);
    yPos += 190;
    
    // Canvas section
    auto canvasSection = juce::Rectangle<int>(bounds.getX(), yPos, bounds.getWidth(), 120);
    drawModernSection(g, canvasSection, "CANVAS", PRIMARY_BLUE);
    yPos += 130;
    
    // Frequency section
    auto freqSection = juce::Rectangle<int>(bounds.getX(), yPos, bounds.getWidth(), 100);
    drawModernSection(g, freqSection, "FREQUENCY", PRIMARY_BLUE);
    yPos += 110;
    
    // Master section
    auto masterSection = juce::Rectangle<int>(bounds.getX(), yPos, bounds.getWidth(), 80);
    drawModernSection(g, masterSection, "MASTER", PRIMARY_BLUE);
    yPos += 90;
    
    // Mode section
    auto modeSection = juce::Rectangle<int>(bounds.getX(), yPos, bounds.getWidth(), 80);
    drawModernSection(g, modeSection, "MODE", PRIMARY_BLUE);
    yPos += 90;
    
    // Spectral masking section
    auto spectralSection = juce::Rectangle<int>(bounds.getX(), yPos, bounds.getWidth(), 140);
    drawModernSection(g, spectralSection, "SPECTRAL MASK", ACCENT_PURPLE);
}

void PaintControlPanel::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    int yPos = 25;
    
    // Brush Controls Section
    brushLabel.setBounds(bounds.getX() + 5, yPos, 100, 20);
    yPos += 25;
    
    sineBrushButton.setBounds(bounds.getX() + 5, yPos, 100, 25);
    harmonicBrushButton.setBounds(bounds.getX() + 110, yPos, 100, 25);
    yPos += 30;
    
    noiseBrushButton.setBounds(bounds.getX() + 5, yPos, 100, 25);
    sampleBrushButton.setBounds(bounds.getX() + 110, yPos, 100, 25);
    yPos += 30;
    
    granularBrushButton.setBounds(bounds.getX() + 5, yPos, 100, 25);
    cdpMorphButton.setBounds(bounds.getX() + 110, yPos, 100, 25);
    yPos += 35;
    
    // Brush Parameters
    brushSizeLabel.setBounds(bounds.getX() + 5, yPos, 50, 20);
    brushSizeSlider.setBounds(bounds.getX() + 60, yPos, 150, 20);
    yPos += 25;
    
    brushPressureLabel.setBounds(bounds.getX() + 5, yPos, 50, 20);
    brushPressureSlider.setBounds(bounds.getX() + 60, yPos, 150, 20);
    yPos += 25;
    
    sampleSlotLabel.setBounds(bounds.getX() + 5, yPos, 50, 20);
    sampleSlotSlider.setBounds(bounds.getX() + 60, yPos, 150, 20);
    yPos += 45;
    
    // Canvas Controls Section
    canvasLabel.setBounds(bounds.getX() + 5, yPos, 100, 20);
    yPos += 25;
    
    clearCanvasButton.setBounds(bounds.getX() + 5, yPos, 100, 25);
    resetViewButton.setBounds(bounds.getX() + 110, yPos, 100, 25);
    yPos += 35;
    
    // Frequency Range Section
    freqRangeLabel.setBounds(bounds.getX() + 5, yPos, 100, 20);
    yPos += 25;
    
    minFreqSlider.setBounds(bounds.getX() + 5, yPos, 100, 20);
    minFreqValueLabel.setBounds(bounds.getX() + 110, yPos, 50, 20);
    yPos += 25;
    
    maxFreqSlider.setBounds(bounds.getX() + 5, yPos, 100, 20);
    maxFreqValueLabel.setBounds(bounds.getX() + 110, yPos, 50, 20);
    yPos += 35;
    
    // Master Controls Section
    masterLabel.setBounds(bounds.getX() + 5, yPos, 100, 20);
    yPos += 25;
    
    masterGainSlider.setBounds(bounds.getX() + 5, yPos, 120, 20);
    masterGainValueLabel.setBounds(bounds.getX() + 130, yPos, 50, 20);
    paintActiveButton.setBounds(bounds.getX() + 180, yPos, 80, 25);
    yPos += 35;
    
    // Mode Controls Section
    modeLabel.setBounds(bounds.getX() + 5, yPos, 100, 20);
    yPos += 25;
    
    canvasModeButton.setBounds(bounds.getX() + 5, yPos, 65, 25);
    forgeModeButton.setBounds(bounds.getX() + 75, yPos, 65, 25);
    hybridModeButton.setBounds(bounds.getX() + 145, yPos, 65, 25);
    yPos += 35;
    
    // Spectral Masking Controls Section
    spectralMaskLabel.setBounds(bounds.getX() + 5, yPos, 100, 20);
    yPos += 25;
    
    spectralMaskEnableButton.setBounds(bounds.getX() + 5, yPos, 80, 25);
    yPos += 30;
    
    maskSourceLabel.setBounds(bounds.getX() + 5, yPos, 50, 20);
    maskSourceSlider.setBounds(bounds.getX() + 60, yPos, 100, 20);
    yPos += 25;
    
    maskTypeLabel.setBounds(bounds.getX() + 5, yPos, 40, 20);
    maskTypeComboBox.setBounds(bounds.getX() + 50, yPos, 120, 25);
    yPos += 30;
    
    maskStrengthLabel.setBounds(bounds.getX() + 5, yPos, 60, 20);
    maskStrengthSlider.setBounds(bounds.getX() + 70, yPos, 100, 20);
    yPos += 25;
    
    timeStretchLabel.setBounds(bounds.getX() + 5, yPos, 40, 20);
    timeStretchSlider.setBounds(bounds.getX() + 50, yPos, 120, 20);
}

//==============================================================================
// Control Callbacks

void PaintControlPanel::buttonClicked(juce::Button* button)
{
    // Brush selection
    if (button == &sineBrushButton)
    {
        currentBrushType = RetroCanvasComponent::BrushType::SineBrush;
        updateBrushButtons();
        if (canvasComponent) canvasComponent->setBrushType(currentBrushType);
    }
    else if (button == &harmonicBrushButton)
    {
        currentBrushType = RetroCanvasComponent::BrushType::HarmonicBrush;
        updateBrushButtons();
        if (canvasComponent) canvasComponent->setBrushType(currentBrushType);
    }
    else if (button == &noiseBrushButton)
    {
        currentBrushType = RetroCanvasComponent::BrushType::NoiseBrush;
        updateBrushButtons();
        if (canvasComponent) canvasComponent->setBrushType(currentBrushType);
    }
    else if (button == &sampleBrushButton)
    {
        currentBrushType = RetroCanvasComponent::BrushType::SampleBrush;
        updateBrushButtons();
        if (canvasComponent) canvasComponent->setBrushType(currentBrushType);
    }
    else if (button == &granularBrushButton)
    {
        currentBrushType = RetroCanvasComponent::BrushType::GranularPen;
        updateBrushButtons();
        if (canvasComponent) canvasComponent->setBrushType(currentBrushType);
    }
    else if (button == &cdpMorphButton)
    {
        currentBrushType = RetroCanvasComponent::BrushType::CDPMorph;
        updateBrushButtons();
        if (canvasComponent) canvasComponent->setBrushType(currentBrushType);
    }
    
    // Canvas controls
    else if (button == &clearCanvasButton)
    {
        sendPaintCommand(PaintCommandID::ClearCanvas);
        if (canvasComponent) canvasComponent->clearCanvas();
        DBG("Canvas cleared");
    }
    else if (button == &resetViewButton)
    {
        if (canvasComponent) canvasComponent->resetView();
        DBG("Canvas view reset");
    }
    
    // Master controls
    else if (button == &paintActiveButton)
    {
        isPaintActive = button->getToggleState();
        button->setButtonText(isPaintActive ? "PAINT: ON" : "PAINT: OFF");
        sendPaintCommand(PaintCommandID::SetPaintActive, isPaintActive ? 1.0f : 0.0f);
        DBG("Paint active: " << (isPaintActive ? "ON" : "OFF"));
    }
    
    // Mode controls
    else if (button == &canvasModeButton)
    {
        // TODO: Send mode change command to processor
        updateModeButtons();
        DBG("Canvas mode selected");
    }
    else if (button == &forgeModeButton)
    {
        // TODO: Send mode change command to processor
        updateModeButtons();
        DBG("Forge mode selected");
    }
    else if (button == &hybridModeButton)
    {
        // TODO: Send mode change command to processor
        updateModeButtons();
        DBG("Hybrid mode selected");
    }
    
    // Spectral masking controls
    else if (button == &spectralMaskEnableButton)
    {
        isSpectralMaskEnabled = button->getToggleState();
        updateSpectralMaskControls();
        DBG("Spectral mask " << (isSpectralMaskEnabled ? "enabled" : "disabled"));
    }
}

void PaintControlPanel::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &brushSizeSlider)
    {
        float size = (float)slider->getValue();
        if (canvasComponent) canvasComponent->setBrushSize(size);
        DBG("Brush size: " << size);
    }
    else if (slider == &brushPressureSlider)
    {
        float pressure = (float)slider->getValue();
        if (canvasComponent) canvasComponent->setBrushPressure(pressure);
        DBG("Brush pressure: " << pressure);
    }
    else if (slider == &sampleSlotSlider)
    {
        int slotIndex = (int)slider->getValue() - 1;  // Convert 1-8 to 0-7
        processor.setActivePaintBrush(slotIndex);
        DBG("Active paint brush slot: " << slotIndex + 1);
    }
    else if (slider == &minFreqSlider)
    {
        float freq = (float)slider->getValue();
        float maxFreq = (float)maxFreqSlider.getValue();
        sendPaintCommand(PaintCommandID::SetFrequencyRange, freq, maxFreq);
        
        juce::String freqText = freq >= 1000.0f ? 
            juce::String(freq / 1000.0f, 1) + "kHz" : 
            juce::String((int)freq) + "Hz";
        minFreqValueLabel.setText(freqText, juce::dontSendNotification);
    }
    else if (slider == &maxFreqSlider)
    {
        float freq = (float)slider->getValue();
        float minFreq = (float)minFreqSlider.getValue();
        sendPaintCommand(PaintCommandID::SetFrequencyRange, minFreq, freq);
        
        juce::String freqText = freq >= 1000.0f ? 
            juce::String(freq / 1000.0f, 1) + "kHz" : 
            juce::String((int)freq) + "Hz";
        maxFreqValueLabel.setText(freqText, juce::dontSendNotification);
    }
    else if (slider == &masterGainSlider)
    {
        float gain = (float)slider->getValue();
        sendPaintCommand(PaintCommandID::SetMasterGain, gain);
        
        int percentage = (int)(gain * 100.0f);
        masterGainValueLabel.setText(juce::String(percentage) + "%", juce::dontSendNotification);
    }
    else if (slider == &maskSourceSlider)
    {
        currentMaskSource = (int)slider->getValue() - 1;  // Convert 1-8 to 0-7
        updateSpectralMaskControls();
        DBG("Spectral mask source: slot " << currentMaskSource + 1);
    }
    else if (slider == &maskStrengthSlider)
    {
        float strength = (float)slider->getValue();
        // Apply to active mask source
        if (isSpectralMaskEnabled)
        {
            auto& voice = processor.getForgeProcessor().getVoice(currentMaskSource);
            if (voice.getSpectralMask())
            {
                voice.getSpectralMask()->setMaskStrength(strength);
                DBG("Mask strength: " << strength);
            }
        }
    }
    else if (slider == &timeStretchSlider)
    {
        float stretch = (float)slider->getValue();
        // Apply to active mask source
        if (isSpectralMaskEnabled)
        {
            auto& voice = processor.getForgeProcessor().getVoice(currentMaskSource);
            if (voice.getSpectralMask())
            {
                voice.getSpectralMask()->setTimeStretch(stretch);
                DBG("Time stretch: " << stretch);
            }
        }
    }
}

//==============================================================================
// Helper Methods

void PaintControlPanel::updateBrushButtons()
{
    // Update brush button states
    sineBrushButton.setToggleState(currentBrushType == RetroCanvasComponent::BrushType::SineBrush, juce::dontSendNotification);
    harmonicBrushButton.setToggleState(currentBrushType == RetroCanvasComponent::BrushType::HarmonicBrush, juce::dontSendNotification);
    noiseBrushButton.setToggleState(currentBrushType == RetroCanvasComponent::BrushType::NoiseBrush, juce::dontSendNotification);
    sampleBrushButton.setToggleState(currentBrushType == RetroCanvasComponent::BrushType::SampleBrush, juce::dontSendNotification);
    granularBrushButton.setToggleState(currentBrushType == RetroCanvasComponent::BrushType::GranularPen, juce::dontSendNotification);
    cdpMorphButton.setToggleState(currentBrushType == RetroCanvasComponent::BrushType::CDPMorph, juce::dontSendNotification);
}

void PaintControlPanel::updateModeButtons()
{
    // TODO: Get actual mode from processor
    // For now, default to canvas mode
    canvasModeButton.setToggleState(true, juce::dontSendNotification);
    forgeModeButton.setToggleState(false, juce::dontSendNotification);
    hybridModeButton.setToggleState(false, juce::dontSendNotification);
}

void PaintControlPanel::sendPaintCommand(PaintCommandID commandID, float value1, float value2)
{
    // Use explicit constructor to avoid ambiguity between position and range constructors
    Command cmd(commandID, value1, value2, 1.0f); // Add pressure parameter to use position constructor
    processor.pushCommandToQueue(cmd);
}

void PaintControlPanel::sendForgeCommand(ForgeCommandID commandID, float value)
{
    Command cmd(commandID, value);
    processor.pushCommandToQueue(cmd);
}

void PaintControlPanel::drawModernSection(juce::Graphics& g, juce::Rectangle<int> area, 
                                          const juce::String& title, juce::Colour borderColor)
{
    // Draw subtle modern border
    g.setColour(borderColor.withAlpha(0.3f));
    g.drawRoundedRectangle(area.toFloat(), 4.0f, 1.0f);
    
    // Draw clean title bar
    if (title.isNotEmpty())
    {
        auto titleArea = area.removeFromTop(22);
        
        // Gradient title background
        juce::ColourGradient gradient(borderColor.withAlpha(0.1f), 
                                     titleArea.getX(), titleArea.getY(),
                                     borderColor.withAlpha(0.05f), 
                                     titleArea.getX(), titleArea.getBottom(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(titleArea.toFloat(), 4.0f);
        
        // Title text
        g.setColour(borderColor);
        g.setFont(createModernFont(11.0f).withStyle(juce::Font::bold));
        g.drawText(title, titleArea, juce::Justification::centred);
    }
}

void PaintControlPanel::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &maskTypeComboBox)
    {
        int selectedId = maskTypeComboBox.getSelectedId();
        
        if (isSpectralMaskEnabled && selectedId > 1)
        {
            auto& voice = processor.getForgeProcessor().getVoice(currentMaskSource);
            if (voice.getSpectralMask())
            {
                // Map ComboBox IDs to SpectralMask::MaskType enum
                SpectralMask::MaskType maskType = static_cast<SpectralMask::MaskType>(selectedId - 2);
                voice.getSpectralMask()->setMaskType(maskType);
                DBG("Mask type changed to: " << selectedId - 2);
            }
        }
    }
}

void PaintControlPanel::updateSpectralMaskControls()
{
    // Update button text based on enable state
    spectralMaskEnableButton.setButtonText(isSpectralMaskEnabled ? "MASK: ON" : "MASK: OFF");
    
    // Enable/disable dependent controls
    maskSourceSlider.setEnabled(isSpectralMaskEnabled);
    maskTypeComboBox.setEnabled(isSpectralMaskEnabled);
    maskStrengthSlider.setEnabled(isSpectralMaskEnabled);
    timeStretchSlider.setEnabled(isSpectralMaskEnabled);
    
    // If enabling spectral mask, set up the selected voice
    if (isSpectralMaskEnabled)
    {
        auto& voice = processor.getForgeProcessor().getVoice(currentMaskSource);
        voice.enableSpectralMask(true);
        
        if (voice.getSpectralMask())
        {
            // Sync UI values with mask settings
            voice.getSpectralMask()->setMaskStrength((float)maskStrengthSlider.getValue());
            voice.getSpectralMask()->setTimeStretch((float)timeStretchSlider.getValue());
            
            int maskTypeId = maskTypeComboBox.getSelectedId();
            if (maskTypeId > 1)
            {
                SpectralMask::MaskType maskType = static_cast<SpectralMask::MaskType>(maskTypeId - 2);
                voice.getSpectralMask()->setMaskType(maskType);
            }
        }
    }
    else
    {
        // Disable spectral mask on all voices
        for (int i = 0; i < 8; ++i)
        {
            processor.getForgeProcessor().getVoice(i).enableSpectralMask(false);
        }
    }
}

juce::Font PaintControlPanel::createModernFont(float size) const
{
    // Clean, readable font inspired by early 2000s pro audio software
    return juce::Font(juce::FontOptions("Tahoma", size, juce::Font::FontStyleFlags::plain))
           .withExtraKerningFactor(0.0f);
}