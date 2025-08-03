#include "ExportDialog.h"
#include "ArtefactLookAndFeel.h"

//==============================================================================

//==============================================================================
ExportDialog::ExportDialog()
{
    setupComponents();
    setSize(600, 400);
    
    // Set default export directory to user's music folder
    auto musicDir = juce::File::getSpecialLocation(juce::File::userMusicDirectory);
    currentSettings.outputDirectory = musicDir.getChildFile("ARTEFACT_Recordings");
    directoryLabel.setText(currentSettings.outputDirectory.getFullPathName(), juce::dontSendNotification);
}

ExportDialog::~ExportDialog()
{
    formatComboBox.removeListener(this);
    exportButton.removeListener(this);
    cancelButton.removeListener(this);
    browseButton.removeListener(this);
}

//==============================================================================
void ExportDialog::paint(juce::Graphics& g)
{
    // Fill background with SpectralCanvas theme
    g.fillAll(ArtefactLookAndFeel::kBackground);
    
    // Draw main border
    g.setColour(ArtefactLookAndFeel::kAccentColour);
    g.drawRect(getLocalBounds(), 2);
    
    // Draw title bar
    auto titleArea = getLocalBounds().removeFromTop(40);
    g.setColour(ArtefactLookAndFeel::kAccentColour.withAlpha(0.1f));
    g.fillRect(titleArea);
    
    g.setColour(ArtefactLookAndFeel::kTextColour);
    g.setFont(createTerminalFont(14.0f));
    g.drawText("◆ SPECTRALCANVAS EXPORT ◆", titleArea, juce::Justification::centred);
    
    auto bounds = getLocalBounds().reduced(20, 50);
    int yPos = 20;
    
    // Draw sections
    auto formatSection = juce::Rectangle<int>(bounds.getX(), yPos, bounds.getWidth(), 100);
    drawTerminalSection(g, formatSection, "FORMAT SETTINGS", ArtefactLookAndFeel::kAccentColour);
    yPos += 120;
    
    auto fileSection = juce::Rectangle<int>(bounds.getX(), yPos, bounds.getWidth(), 120);
    drawTerminalSection(g, fileSection, "FILE SETTINGS", ArtefactLookAndFeel::kAccentColour);
    yPos += 140;
    
    auto optionsSection = juce::Rectangle<int>(bounds.getX(), yPos, bounds.getWidth(), 60);
    drawTerminalSection(g, optionsSection, "OPTIONS", ArtefactLookAndFeel::kAccentColour);
}

void ExportDialog::resized()
{
    auto bounds = getLocalBounds().reduced(30, 60);
    int yPos = 30;
    
    // Format Section
    formatLabel.setBounds(bounds.getX() + 10, yPos, 120, 25);
    formatComboBox.setBounds(bounds.getX() + 140, yPos, 200, 25);
    yPos += 35;
    
    qualityLabel.setBounds(bounds.getX() + 10, yPos, 80, 25);
    qualityInfoLabel.setBounds(bounds.getX() + 100, yPos, 300, 25);
    yPos += 55;
    
    // File Section
    fileLabel.setBounds(bounds.getX() + 10, yPos, 120, 25);
    yPos += 30;
    
    filenameEditor.setBounds(bounds.getX() + 10, yPos, 300, 25);
    browseButton.setBounds(bounds.getX() + 320, yPos, 100, 25);
    yPos += 35;
    
    directoryLabel.setBounds(bounds.getX() + 10, yPos, bounds.getWidth() - 20, 25);
    yPos += 65;
    
    // Options Section
    overwriteToggle.setBounds(bounds.getX() + 10, yPos, 200, 25);
    yPos += 50;
    
    // Buttons
    cancelButton.setBounds(bounds.getRight() - 180, getHeight() - 50, 80, 30);
    exportButton.setBounds(bounds.getRight() - 90, getHeight() - 50, 80, 30);
}

//==============================================================================
void ExportDialog::buttonClicked(juce::Button* button)
{
    if (button == &exportButton)
    {
        validateSettings();
        
        // Get filename from editor
        currentSettings.filename = filenameEditor.getText();
        
        // Add extension if not present
        juce::String extension = getFormatExtension(currentSettings.format);
        if (!currentSettings.filename.endsWithIgnoreCase(extension))
        {
            currentSettings.filename += extension;
        }
        
        currentSettings.overwriteExisting = overwriteToggle.getToggleState();
        
        // Check if file exists and warn user
        auto outputFile = currentSettings.outputDirectory.getChildFile(currentSettings.filename);
        if (outputFile.existsAsFile() && !currentSettings.overwriteExisting)
        {
            // For now, just skip the overwrite check - will improve with proper async dialog later
            // TODO: Implement proper async file overwrite dialog
        }
        
        // Close dialog with success
        if (auto* parent = findParentComponentOfClass<juce::DialogWindow>())
        {
            parent->setVisible(false);
        }
    }
    else if (button == &cancelButton)
    {
        // Close dialog with cancel
        if (auto* parent = findParentComponentOfClass<juce::DialogWindow>())
        {
            parent->setVisible(false);
        }
    }
    else if (button == &browseButton)
    {
        browseForDirectory();
    }
}

void ExportDialog::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &formatComboBox)
    {
        int selectedId = formatComboBox.getSelectedId();
        currentSettings.format = static_cast<AudioRecorder::ExportFormat>(selectedId - 1);
        updateQualityInfo();
    }
}

//==============================================================================
void ExportDialog::setupComponents()
{
    // Format ComboBox
    addAndMakeVisible(formatComboBox);
    formatComboBox.addItem("WAV 16-bit", static_cast<int>(AudioRecorder::ExportFormat::WAV_16bit) + 1);
    formatComboBox.addItem("WAV 24-bit", static_cast<int>(AudioRecorder::ExportFormat::WAV_24bit) + 1);
    formatComboBox.addItem("WAV 32-bit Float", static_cast<int>(AudioRecorder::ExportFormat::WAV_32bit_Float) + 1);
    formatComboBox.addItem("AIFF 16-bit", static_cast<int>(AudioRecorder::ExportFormat::AIFF_16bit) + 1);
    formatComboBox.addItem("AIFF 24-bit", static_cast<int>(AudioRecorder::ExportFormat::AIFF_24bit) + 1);
    formatComboBox.setSelectedId(static_cast<int>(AudioRecorder::ExportFormat::WAV_24bit) + 1);
    formatComboBox.addListener(this);
    
    // Labels
    addAndMakeVisible(formatLabel);
    addAndMakeVisible(qualityLabel);
    addAndMakeVisible(qualityInfoLabel);
    addAndMakeVisible(fileLabel);
    addAndMakeVisible(directoryLabel);
    
    // File controls
    addAndMakeVisible(filenameEditor);
    addAndMakeVisible(browseButton);
    addAndMakeVisible(overwriteToggle);
    
    // Buttons
    addAndMakeVisible(exportButton);
    addAndMakeVisible(cancelButton);
    
    exportButton.addListener(this);
    cancelButton.addListener(this);
    browseButton.addListener(this);
    
    // Set default filename with timestamp
    auto now = juce::Time::getCurrentTime();
    auto timeString = now.formatted("%Y%m%d_%H%M%S");
    filenameEditor.setText("ARTEFACT_Recording_" + timeString);
    
    // Style components with terminal colors
    auto terminalFont = createTerminalFont(11.0f);
    
    formatLabel.setFont(terminalFont);
    formatLabel.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kAccentColour);
    
    qualityLabel.setFont(terminalFont);
    qualityLabel.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kTextColour);
    
    qualityInfoLabel.setFont(terminalFont);
    qualityInfoLabel.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kPrimaryGreen);
    
    fileLabel.setFont(terminalFont);
    fileLabel.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kAccentColour);
    
    directoryLabel.setFont(createTerminalFont(9.0f));
    directoryLabel.setColour(juce::Label::textColourId, ArtefactLookAndFeel::kPrimaryGreen);
    
    // Terminal styling for editors and buttons
    filenameEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    filenameEditor.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    filenameEditor.setColour(juce::TextEditor::outlineColourId, ArtefactLookAndFeel::kAccentColour);
    filenameEditor.setFont(terminalFont);
    
    formatComboBox.setColour(juce::ComboBox::backgroundColourId, ArtefactLookAndFeel::kPanelBackground);
    formatComboBox.setColour(juce::ComboBox::textColourId, ArtefactLookAndFeel::kTextColour);
    formatComboBox.setColour(juce::ComboBox::outlineColourId, ArtefactLookAndFeel::kAccentColour);
    
    exportButton.setColour(juce::TextButton::buttonColourId, ArtefactLookAndFeel::kAccentColour.withAlpha(0.2f));
    exportButton.setColour(juce::TextButton::textColourOffId, ArtefactLookAndFeel::kAccentColour);
    
    cancelButton.setColour(juce::TextButton::buttonColourId, ArtefactLookAndFeel::kWarningRed.withAlpha(0.2f));
    cancelButton.setColour(juce::TextButton::textColourOffId, ArtefactLookAndFeel::kWarningRed);
    
    browseButton.setColour(juce::TextButton::buttonColourId, ArtefactLookAndFeel::kPrimaryGreen.withAlpha(0.2f));
    browseButton.setColour(juce::TextButton::textColourOffId, ArtefactLookAndFeel::kPrimaryGreen);
    
    overwriteToggle.setColour(juce::ToggleButton::textColourId, ArtefactLookAndFeel::kTextColour);
    overwriteToggle.setColour(juce::ToggleButton::tickColourId, ArtefactLookAndFeel::kAccentColour);
    
    updateQualityInfo();
}

void ExportDialog::updateQualityInfo()
{
    juce::String qualityText;
    
    switch (currentSettings.format)
    {
        case AudioRecorder::ExportFormat::WAV_16bit:
            qualityText = "16-bit / 44.1kHz WAV";
            break;
        case AudioRecorder::ExportFormat::WAV_24bit:
            qualityText = "24-bit / 44.1kHz WAV";
            break;
        case AudioRecorder::ExportFormat::WAV_32bit_Float:
            qualityText = "32-bit Float / 44.1kHz WAV";
            break;
        case AudioRecorder::ExportFormat::AIFF_16bit:
            qualityText = "16-bit / 44.1kHz AIFF";
            break;
        case AudioRecorder::ExportFormat::AIFF_24bit:
            qualityText = "24-bit / 44.1kHz AIFF";
            break;
    }
    
    qualityInfoLabel.setText(qualityText, juce::dontSendNotification);
}

void ExportDialog::browseForDirectory()
{
    auto chooser = std::make_unique<juce::FileChooser>(
        "Choose Export Directory",
        currentSettings.outputDirectory,
        "",
        true
    );
    
    auto chooserFlags = juce::FileBrowserComponent::openMode | 
                       juce::FileBrowserComponent::canSelectDirectories;
    
    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto result = fc.getResult();
        if (result.exists() && result.isDirectory())
        {
            currentSettings.outputDirectory = result;
            directoryLabel.setText(result.getFullPathName(), juce::dontSendNotification);
        }
    });
}

void ExportDialog::validateSettings()
{
    // Ensure output directory exists
    if (!currentSettings.outputDirectory.exists())
    {
        currentSettings.outputDirectory.createDirectory();
    }
    
    // Validate filename
    auto filename = filenameEditor.getText().trim();
    if (filename.isEmpty())
    {
        filenameEditor.setText("ARTEFACT_Recording");
    }
}

juce::String ExportDialog::getFormatExtension(AudioRecorder::ExportFormat format) const
{
    switch (format)
    {
        case AudioRecorder::ExportFormat::WAV_16bit:
        case AudioRecorder::ExportFormat::WAV_24bit:
        case AudioRecorder::ExportFormat::WAV_32bit_Float:
            return ".wav";
        case AudioRecorder::ExportFormat::AIFF_16bit:
        case AudioRecorder::ExportFormat::AIFF_24bit:
            return ".aiff";
    }
    return ".wav";
}

void ExportDialog::setDefaultDirectory(const juce::File& directory)
{
    currentSettings.outputDirectory = directory;
    directoryLabel.setText(directory.getFullPathName(), juce::dontSendNotification);
}

void ExportDialog::setDefaultFilename(const juce::String& filename)
{
    filenameEditor.setText(filename);
}

//==============================================================================
// Static Modal Dialog

ExportDialog::ExportSettings ExportDialog::showExportDialog(juce::Component* parent, 
                                                           const juce::String& defaultFilename)
{
    auto dialog = std::make_unique<ExportDialog>();
    dialog->setDefaultFilename(defaultFilename);
    
    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(dialog.release());
    options.dialogTitle = "Export Audio";
    options.componentToCentreAround = parent;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = false;
    options.resizable = false;
    options.useBottomRightCornerResizer = false;
    
    auto* dialogPtr = static_cast<ExportDialog*>(options.content.get());
    
    auto* dialogWindow = options.launchAsync();
    if (dialogWindow != nullptr)
    {
        // For now, return default settings - this will be improved with proper modal handling
        return dialogPtr->getExportSettings();
    }
    
    return ExportSettings{}; // Return default settings if cancelled
}

//==============================================================================
// Helper Methods

void ExportDialog::drawTerminalSection(juce::Graphics& g, juce::Rectangle<int> area, 
                                     const juce::String& title, juce::Colour borderColor)
{
    // Draw section border
    g.setColour(borderColor.withAlpha(0.5f));
    g.drawRect(area, 1);
    
    // Draw title background
    if (title.isNotEmpty())
    {
        auto titleArea = area.removeFromTop(25);
        g.setColour(borderColor.withAlpha(0.1f));
        g.fillRect(titleArea);
        
        g.setColour(borderColor);
        g.setFont(createTerminalFont(10.0f));
        g.drawText("▸ " + title, titleArea.reduced(5, 0), juce::Justification::centredLeft);
    }
}

juce::Font ExportDialog::createTerminalFont(float size) const
{
    return juce::Font(juce::FontOptions("Courier New", size, juce::Font::FontStyleFlags::bold))
           .withExtraKerningFactor(0.0f);
}