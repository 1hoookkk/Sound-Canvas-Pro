// GUI/ArtefactLookAndFeel.cpp
#include "ArtefactLookAndFeel.h"

// SpectralCanvas Amateur/Hobbyist UI - Early DIY Software Aesthetic
const juce::Colour ArtefactLookAndFeel::kBackground = juce::Colour(0xffc0c0c0);        // Windows 95 gray
const juce::Colour ArtefactLookAndFeel::kPanelBackground = juce::Colour(0xffd4d0c8);  // Classic dialog gray
const juce::Colour ArtefactLookAndFeel::kBevelLight = juce::Colour(0xffffffff);       // White highlights
const juce::Colour ArtefactLookAndFeel::kBevelDark = juce::Colour(0xff808080);        // Dark gray shadows
const juce::Colour ArtefactLookAndFeel::kTextColour = juce::Colour(0xff000000);       // Plain black text
const juce::Colour ArtefactLookAndFeel::kAccentColour = juce::Colour(0xff0000ff);     // Basic blue
const juce::Colour ArtefactLookAndFeel::kCanvasBlack = juce::Colour(0xfff0f0f0);      // Light gray canvas
const juce::Colour ArtefactLookAndFeel::kWarningRed = juce::Colour(0xffff0000);       // Pure red
const juce::Colour ArtefactLookAndFeel::kReadoutGreen = juce::Colour(0xff008000);     // Basic green

// Amateur Extended Palette - Basic Colors
const juce::Colour ArtefactLookAndFeel::kPrimaryBlue = juce::Colour(0xff0000ff);      // Pure blue
const juce::Colour ArtefactLookAndFeel::kPrimaryGreen = juce::Colour(0xff008000);     // Basic green
const juce::Colour ArtefactLookAndFeel::kPrimaryPurple = juce::Colour(0xff800080);    // Basic purple
const juce::Colour ArtefactLookAndFeel::kAccentPurple = juce::Colour(0xff800080);     // Basic purple

ArtefactLookAndFeel::ArtefactLookAndFeel()
{
    // Set up SpectralCanvas amateur/hobbyist color scheme
    setColour(juce::ResizableWindow::backgroundColourId, kBackground);
    setColour(juce::Label::textColourId, kTextColour);
    setColour(juce::Slider::textBoxTextColourId, kTextColour);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxOutlineColourId, kAccentColour);
    setColour(juce::TextButton::buttonColourId, kPanelBackground);
    setColour(juce::TextButton::buttonOnColourId, kAccentColour);
    setColour(juce::TextButton::textColourOffId, kTextColour);
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    
    // Additional colors for modern vibrant look
    setColour(juce::ComboBox::backgroundColourId, kPanelBackground);
    setColour(juce::ComboBox::textColourId, kTextColour);
    setColour(juce::ComboBox::outlineColourId, kAccentColour);
    setColour(juce::PopupMenu::backgroundColourId, kPanelBackground);
    setColour(juce::PopupMenu::textColourId, kTextColour);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, kAccentColour);
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
}

juce::Font ArtefactLookAndFeel::getSilkscreenFont(float height)
{
    if (silkscreenTypeface != nullptr)
        return juce::Font(silkscreenTypeface).withHeight(height);

    // Fallback to a monospace font
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), height, juce::Font::FontStyleFlags::bold));
}

juce::Font ArtefactLookAndFeel::getSystemFont(float height)
{
    // Amateur system font - whatever's available
    return juce::Font(juce::FontOptions(height, juce::Font::FontStyleFlags::plain));
}

juce::Font ArtefactLookAndFeel::getHeaderFont(float height)
{
    // Amateur headers - just make it bold
    return juce::Font(juce::FontOptions(height, juce::Font::FontStyleFlags::bold));
}

juce::Font ArtefactLookAndFeel::getUIFont(float height)
{
    // Amateur UI font - basic system font
    return juce::Font(juce::FontOptions(height, juce::Font::FontStyleFlags::plain));
}

juce::Font ArtefactLookAndFeel::getMonospaceFont(float height)
{
    // For numerical displays and technical readouts
    return juce::Font(juce::FontOptions("Courier New", height, juce::Font::FontStyleFlags::plain));
}

void ArtefactLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    const float radius = juce::jmin(width / 2, height / 2) - 4.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Amateur knob design - basic and functional
    
    // Simple filled circle
    g.setColour(kPanelBackground);
    g.fillEllipse(rx, ry, rw, rw);
    
    // Basic border
    g.setColour(kTextColour);
    g.drawEllipse(rx, ry, rw, rw, 2.0f);
    
    // Simple line indicator from center
    const float lineLength = radius * 0.8f;
    const float lineX = centreX + std::cos(angle - juce::MathConstants<float>::halfPi) * lineLength;
    const float lineY = centreY + std::sin(angle - juce::MathConstants<float>::halfPi) * lineLength;
    
    g.setColour(kAccentColour);
    g.drawLine(centreX, centreY, lineX, lineY, 3.0f);
    
    // Center dot
    g.setColour(kTextColour);
    g.fillEllipse(centreX - 3, centreY - 3, 6, 6);
}

void ArtefactLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour& backgroundColour,
    bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f, 1.0f);
    
    const bool toggleState = button.getToggleState();
    
    // Amateur button design - Windows 95 style
    juce::Colour buttonFill = kPanelBackground;
    
    if (toggleState)
    {
        buttonFill = kAccentColour;
    }
    else if (isMouseOverButton)
    {
        buttonFill = kPanelBackground.brighter(0.1f);
    }
    
    // Fill button
    g.setColour(buttonFill);
    g.fillRect(bounds);
    
    // 3D bevel effect (amateur Windows 95 style)
    if (isButtonDown || toggleState)
    {
        // Pressed - dark on top/left, light on bottom/right
        g.setColour(kBevelDark);
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY(), 2.0f); // top
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom(), 2.0f); // left
        
        g.setColour(kBevelLight);
        g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom(), 1.0f); // bottom
        g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom(), 1.0f); // right
    }
    else
    {
        // Normal - light on top/left, dark on bottom/right
        g.setColour(kBevelLight);
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY(), 2.0f); // top
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom(), 2.0f); // left
        
        g.setColour(kBevelDark);
        g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom(), 1.0f); // bottom
        g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom(), 1.0f); // right
    }
}

void ArtefactLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height,
    juce::TextEditor& textEditor)
{
    if (textEditor.isEnabled())
    {
        g.setColour(textEditor.hasKeyboardFocus(true) ? kAccentColour : kBevelDark);
        g.drawRect(0, 0, width, height, 2);
    }
}

void ArtefactLookAndFeel::fillTextEditorBackground(juce::Graphics& g, int width, int height,
    juce::TextEditor& textEditor)
{
    g.setColour(kBackground);
    g.fillRect(0, 0, width, height);
}

void ArtefactLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos,
    const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto trackArea = juce::Rectangle<float>(x, y, width, height);
    
    // Amateur recessed track - just a basic inset rectangle
    g.setColour(kBevelDark);
    g.fillRect(trackArea);
    
    // Basic inset border
    g.setColour(kBevelDark);
    g.drawRect(trackArea, 1.0f);
    
    // Amateur slider thumb - just a rectangle
    float thumbSize = (style == juce::Slider::LinearHorizontal) ? 20.0f : 16.0f;
    juce::Rectangle<float> thumbArea;
    
    if (style == juce::Slider::LinearHorizontal)
    {
        thumbArea = juce::Rectangle<float>(sliderPos - thumbSize/2, 
                                         y + 1, 
                                         thumbSize, height - 2);
    }
    else
    {
        thumbArea = juce::Rectangle<float>(x + 1, 
                                         sliderPos - thumbSize/2,
                                         width - 2, thumbSize);
    }
    
    // Simple thumb fill
    g.setColour(kPanelBackground);
    g.fillRect(thumbArea);
    
    // Basic 3D effect
    g.setColour(kBevelLight);
    g.drawLine(thumbArea.getX(), thumbArea.getY(), thumbArea.getRight(), thumbArea.getY(), 1.0f);  // Top
    g.drawLine(thumbArea.getX(), thumbArea.getY(), thumbArea.getX(), thumbArea.getBottom(), 1.0f); // Left
    
    g.setColour(kBevelDark);
    g.drawLine(thumbArea.getX(), thumbArea.getBottom(), thumbArea.getRight(), thumbArea.getBottom(), 1.0f); // Bottom
    g.drawLine(thumbArea.getRight(), thumbArea.getY(), thumbArea.getRight(), thumbArea.getBottom(), 1.0f);   // Right
}