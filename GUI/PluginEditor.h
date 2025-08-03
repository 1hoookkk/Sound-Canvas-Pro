// Source/GUI/PluginEditor.h
#pragma once

#include <JuceHeader.h>
#include "Core/Commands.h"
#include "Core/ForgeProcessor.h"
#include "Core/PaintEngine.h"
#include "Core/ParameterBridge.h"
#include "GUI/RetroCanvasComponent.h"
#include "GUI/PaintControlPanel.h"

// Forward declarations
class ARTEFACTAudioProcessor;
class ArtefactLookAndFeel;
class HeaderBarComponent;
class ForgePanel;

class ARTEFACTAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Button::Listener,
    public juce::Timer,
    public juce::ComponentListener
{
public:
    explicit ARTEFACTAudioProcessorEditor(ARTEFACTAudioProcessor&);
    ~ARTEFACTAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button*) override;
    void timerCallback() override;
    
    // ComponentListener overrides for window focus detection
    void componentVisibilityChanged(juce::Component& component) override;
    void componentParentHierarchyChanged(juce::Component& component) override;

private:
    ARTEFACTAudioProcessor& audioProcessor;
    
    // UI Components
    std::unique_ptr<ArtefactLookAndFeel> artefactLookAndFeel;
    std::unique_ptr<HeaderBarComponent> headerBar;
    std::unique_ptr<ForgePanel> forgePanel;
    std::unique_ptr<RetroCanvasComponent> retroCanvasComponent;
    std::unique_ptr<PaintControlPanel> paintControlPanel;
    juce::TextButton testButton {"Test"};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ARTEFACTAudioProcessorEditor)
};
