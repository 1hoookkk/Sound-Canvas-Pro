#include "PluginEditor.h"
#include "Core/PluginProcessor.h"       // NOT "../PluginProcessor.h"
#include "ArtefactLookAndFeel.h"
#include "HeaderBarComponent.h"
#include "ForgePanel.h"
#include "RetroCanvasComponent.h"
#include "../Core/Commands.h"          

//==============================================================================
ARTEFACTAudioProcessorEditor::ARTEFACTAudioProcessorEditor(ARTEFACTAudioProcessor& p)
    : juce::AudioProcessorEditor(p),
    audioProcessor(p)
{
    // Create look and feel first
    artefactLookAndFeel = std::make_unique<ArtefactLookAndFeel>();
    setLookAndFeel(artefactLookAndFeel.get());

    // Create UI components
    headerBar = std::make_unique<HeaderBarComponent>(p);
    forgePanel = std::make_unique<ForgePanel>(p);
    retroCanvasComponent = std::make_unique<RetroCanvasComponent>();
    paintControlPanel = std::make_unique<PaintControlPanel>(p);

    addAndMakeVisible(headerBar.get());
    addAndMakeVisible(forgePanel.get());
    addAndMakeVisible(retroCanvasComponent.get());
    addAndMakeVisible(paintControlPanel.get());

    // Set up canvas integration with PaintEngine
    retroCanvasComponent->setPaintEngine(&p.getPaintEngine());
    retroCanvasComponent->setCommandTarget([&p](const Command& cmd) {
        return p.pushCommandToQueue(cmd);
    });
    retroCanvasComponent->setProcessor(&p);
    
    // Set up paint control panel integration
    paintControlPanel->setCanvasComponent(retroCanvasComponent.get());

    // Add test button
    addAndMakeVisible(testButton);
    testButton.addListener(this);

    // Start timer for real-time updates (30 FPS for GUI)
    startTimerHz(30);
    
    // Add component listener for window focus detection (prevents audio feedback)
    addComponentListener(this);

    setSize(800, 600);
}

ARTEFACTAudioProcessorEditor::~ARTEFACTAudioProcessorEditor()
{
    removeComponentListener(this);
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
void ARTEFACTAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ARTEFACTAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int headerH = 35;        // Smaller header for compact layout
    const int forgePanelWidth = 200;     // Reduced width for 800px window
    const int paintControlWidth = 180;   // Reduced width for compact layout

    // Header bar across the top
    headerBar->setBounds(bounds.removeFromTop(headerH));

    // Place test button in header area
    testButton.setBounds(getWidth() - 85, 3, 80, 25);

    // Compact three-panel layout: Forge (left), Canvas (center), Paint Controls (right)
    auto forgeArea = bounds.removeFromLeft(forgePanelWidth);
    forgePanel->setBounds(forgeArea);
    
    auto paintControlArea = bounds.removeFromRight(paintControlWidth);
    paintControlPanel->setBounds(paintControlArea);

    // Canvas takes the remaining center space (approximately 420px wide)
    retroCanvasComponent->setBounds(bounds);
}

void ARTEFACTAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &testButton)
    {
        // When the test button is clicked, push our test command to the queue.
        audioProcessor.pushCommandToQueue(Command(ForgeCommandID::Test));
        DBG("Test button clicked - command sent!");
    }
}

void ARTEFACTAudioProcessorEditor::timerCallback()
{
    // Update canvas with real-time performance information
    if (retroCanvasComponent)
    {
        auto& paintEngine = audioProcessor.getPaintEngine();
        retroCanvasComponent->setPerformanceInfo(
            paintEngine.getCurrentCPULoad(),
            paintEngine.getActiveOscillatorCount(),
            0.008f  // Approximate 8ms latency - could be more accurate
        );
    }
}

//==============================================================================
// Window Focus Detection (prevents audio feedback when minimized)

void ARTEFACTAudioProcessorEditor::componentVisibilityChanged(juce::Component& component)
{
    // Check if this is the main editor component
    if (&component == this)
    {
        if (component.isVisible())
        {
            audioProcessor.resumeAudioProcessing();
            DBG("SpectralCanvas: Window visible - audio resumed");
        }
        else
        {
            audioProcessor.pauseAudioProcessing();
            DBG("SpectralCanvas: Window hidden - audio paused");
        }
    }
}

void ARTEFACTAudioProcessorEditor::componentParentHierarchyChanged(juce::Component& component)
{
    // Detect when component is added/removed from window (minimized/restored)
    if (&component == this)
    {
        if (auto* peer = component.getPeer())
        {
            if (peer->isMinimised())
            {
                audioProcessor.pauseAudioProcessing();
                DBG("SpectralCanvas: Window minimized - audio paused");
            }
            else
            {
                audioProcessor.resumeAudioProcessing();
                DBG("SpectralCanvas: Window restored - audio resumed");
            }
        }
    }
}