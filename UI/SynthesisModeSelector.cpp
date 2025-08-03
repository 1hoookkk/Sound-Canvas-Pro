#include "SynthesisModeSelector.h"

//==============================================================================
SynthesisModeSelector::SynthesisModeSelector()
{
    setupModeTabs();
    setCurrentMode(SynthMode::HybridSynthesis); // Default to most capable mode
}

SynthesisModeSelector::~SynthesisModeSelector()
{
    // Components cleaned up automatically
}

//==============================================================================
void SynthesisModeSelector::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Professional background
    g.fillAll(juce::Colour(ModeSelectorColors::backgroundDark));
    
    // Draw tab backgrounds and visual feedback
    for (size_t i = 0; i < modeTabs.size(); ++i)
    {
        const auto& tab = *modeTabs[i];
        auto tabBounds = getTabBounds(static_cast<int>(i));
        
        // Tab background
        juce::Colour tabColor = tab.isActive ? 
            juce::Colour(ModeSelectorColors::tabActive) : 
            juce::Colour(ModeSelectorColors::tabInactive);
            
        g.setColour(tabColor);
        g.fillRoundedRectangle(tabBounds.toFloat(), 4.0f);
        
        // Active tab glow effect
        if (tab.isActive)
        {
            g.setColour(tab.color.withAlpha(0.3f));
            g.drawRoundedRectangle(tabBounds.expanded(2).toFloat(), 6.0f, 2.0f);
            
            // Inner glow
            g.setColour(tab.color.withAlpha(0.6f));
            g.drawRoundedRectangle(tabBounds.toFloat(), 4.0f, 1.0f);
        }
        else
        {
            // Subtle border for inactive tabs
            g.setColour(juce::Colour(ModeSelectorColors::tabActive));
            g.drawRoundedRectangle(tabBounds.toFloat(), 4.0f, 1.0f);
        }
        
        // Mode indicator dot
        auto dotBounds = tabBounds.removeFromLeft(20).reduced(6);
        g.setColour(tab.color.withAlpha(tab.isActive ? 1.0f : 0.5f));
        g.fillEllipse(dotBounds.toFloat());
        
        // Mode description (when active)
        if (tab.isActive && !tab.description.isEmpty())
        {
            auto descBounds = bounds.removeFromBottom(20);
            g.setColour(juce::Colour(ModeSelectorColors::textNormal));
            g.setFont(juce::FontOptions(10.0f));
            g.drawText(tab.description, descBounds, juce::Justification::centred);
        }
    }
}

void SynthesisModeSelector::resized()
{
    auto bounds = getLocalBounds();
    
    // Layout tabs vertically
    for (size_t i = 0; i < modeTabs.size(); ++i)
    {
        auto& tab = *modeTabs[i];
        auto tabBounds = getTabBounds(static_cast<int>(i));
        
        // Position button within tab bounds (leaving space for indicator dot)
        auto buttonBounds = tabBounds.withTrimmedLeft(24).reduced(4);
        tab.button->setBounds(buttonBounds);
    }
}

//==============================================================================
void SynthesisModeSelector::setCurrentMode(SynthMode mode)
{
    if (currentMode == mode) return;
    
    currentMode = mode;
    updateTabStates();
    
    if (onModeChanged)
        onModeChanged(mode);
        
    repaint();
}

//==============================================================================
void SynthesisModeSelector::buttonClicked(juce::Button* button)
{
    // Find which tab was clicked
    for (const auto& tab : modeTabs)
    {
        if (tab->button.get() == button)
        {
            setCurrentMode(tab->mode);
            break;
        }
    }
}

//==============================================================================
void SynthesisModeSelector::setupModeTabs()
{
    modeTabs.clear();
    
    // Create mode tabs with professional color coding
    modeTabs.push_back(std::make_unique<ModeTab>(
        SynthMode::PaintSynthesis,
        "PAINT",
        "Direct paint-to-audio synthesis with real-time brush control",
        juce::Colour(ModeSelectorColors::paintRed)
    ));
    
    modeTabs.push_back(std::make_unique<ModeTab>(
        SynthMode::SpectralOscillators,
        "SPECTRAL", 
        "Frequency-based oscillator bank for instant synthesis",
        juce::Colour(ModeSelectorColors::spectralCyan)
    ));
    
    modeTabs.push_back(std::make_unique<ModeTab>(
        SynthMode::TrackerSequencing,
        "TRACKER",
        "Linear tracker-style sequencing from paint strokes", 
        juce::Colour(ModeSelectorColors::trackerGreen)
    ));
    
    modeTabs.push_back(std::make_unique<ModeTab>(
        SynthMode::SampleSynthesis,
        "SAMPLE",
        "Sample-based synthesis with spectral paint modulation",
        juce::Colour(ModeSelectorColors::sampleOrange)
    ));
    
    modeTabs.push_back(std::make_unique<ModeTab>(
        SynthMode::HybridSynthesis,
        "HYBRID",
        "Combines all synthesis methods for maximum flexibility",
        juce::Colour(ModeSelectorColors::hybridPurple)
    ));
    
    // Setup all tab buttons
    for (auto& tab : modeTabs)
    {
        setupTabButton(*tab);
        addAndMakeVisible(tab->button.get());
        tab->button->addListener(this);
    }
    
    updateTabStates();
}

void SynthesisModeSelector::setupTabButton(ModeTab& tab)
{
    auto& button = *tab.button;
    
    // Professional button styling
    button.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    button.setColour(juce::TextButton::textColourOffId, juce::Colour(ModeSelectorColors::textNormal));
    button.setColour(juce::TextButton::textColourOnId, juce::Colour(ModeSelectorColors::textActive));
    
    // Font styling handled by LookAndFeel system
}

void SynthesisModeSelector::updateTabStates()
{
    for (auto& tab : modeTabs)
    {
        tab->isActive = (tab->mode == currentMode);
        tab->button->setToggleState(tab->isActive, juce::dontSendNotification);
    }
}

juce::Rectangle<int> SynthesisModeSelector::getTabBounds(int tabIndex) const
{
    auto bounds = getLocalBounds();
    const int tabHeight = 45;
    const int tabSpacing = 2;
    
    int y = tabIndex * (tabHeight + tabSpacing);
    return juce::Rectangle<int>(0, y, bounds.getWidth(), tabHeight);
}