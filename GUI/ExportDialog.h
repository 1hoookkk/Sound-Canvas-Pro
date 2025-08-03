#pragma once

#include <JuceHeader.h>
#include "Core/AudioRecorder.h"

/**
 * ExportDialog - Terminal-aesthetic file export dialog
 * 
 * Features:
 * - Retro-style file format selection
 * - Terminal green aesthetic matching ARTEFACT theme
 * - WAV/AIFF format options with quality settings
 * - File location chooser
 * - Export progress indication
 */
class ExportDialog : public juce::Component,
                    public juce::Button::Listener,
                    public juce::ComboBox::Listener
{
public:
    //==============================================================================
    struct ExportSettings
    {
        AudioRecorder::ExportFormat format = AudioRecorder::ExportFormat::WAV_24bit;
        juce::File outputDirectory;
        juce::String filename;
        bool overwriteExisting = false;
        
        ExportSettings() = default;
    };
    
    //==============================================================================
    ExportDialog();
    ~ExportDialog() override;
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Control callbacks
    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    
    // Export interface
    ExportSettings getExportSettings() const { return currentSettings; }
    void setDefaultDirectory(const juce::File& directory);
    void setDefaultFilename(const juce::String& filename);
    
    // Modal dialog helpers
    static ExportSettings showExportDialog(juce::Component* parent, 
                                         const juce::String& defaultFilename = "ARTEFACT_Recording");
    
private:
    //==============================================================================
    // Terminal Colors (matching ARTEFACT theme)
    // Using ArtefactLookAndFeel color scheme instead of terminal colors
    
    //==============================================================================
    // Components
    
    // Format Selection
    juce::Label formatLabel {"formatLabel", "EXPORT FORMAT"};
    juce::ComboBox formatComboBox;
    
    // Quality Settings
    juce::Label qualityLabel {"qualityLabel", "QUALITY"};
    juce::Label qualityInfoLabel {"qualityInfo", "24-bit / 44.1kHz"};
    
    // File Settings
    juce::Label fileLabel {"fileLabel", "OUTPUT FILE"};
    juce::TextEditor filenameEditor;
    juce::TextButton browseButton {"BROWSE..."};
    juce::Label directoryLabel {"directoryLabel", ""};
    
    // Export Controls
    juce::TextButton exportButton {"EXPORT"};
    juce::TextButton cancelButton {"CANCEL"};
    
    // Options
    juce::ToggleButton overwriteToggle {"Overwrite existing files"};
    
    //==============================================================================
    // State
    ExportSettings currentSettings;
    std::function<void(bool, const ExportSettings&)> completionCallback;
    
    //==============================================================================
    // Helpers
    void setupComponents();
    void updateQualityInfo();
    void browseForDirectory();
    void validateSettings();
    juce::String getFormatExtension(AudioRecorder::ExportFormat format) const;
    void drawTerminalSection(juce::Graphics& g, juce::Rectangle<int> area, 
                           const juce::String& title, juce::Colour borderColor);
    juce::Font createTerminalFont(float size) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExportDialog)
};