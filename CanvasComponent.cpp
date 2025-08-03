/******************************************************************************
 * File: CanvasComponent_Modern.cpp
 * Description: Implementation of modern immersive canvas
 * 
 * Copyright (c) 2025 Spectral Audio Systems
 ******************************************************************************/

#include "CanvasComponent.h"
#include "Core/SampleMaskingEngine.h"
#include "Core/SpectralSynthEngine.h"
#include "Core/Commands.h"

//==============================================================================
CanvasComponent::CanvasComponent()
{
    setInterceptsMouseClicks(true, false);
    setOpaque(true);
}

//==============================================================================
// Visual Rendering
//==============================================================================

void CanvasComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Modern dark background
    g.fillAll(juce::Colour(ModernCanvasColors::backgroundDark));
    
    // Minimal grid
    drawMinimalGrid(g);
    
    // Paint strokes with glow
    drawPaintStrokes(g);
    
    // Crosshair cursor
    if (isMouseOverOrDragging())
    {
        drawCrosshair(g, lastMousePos);
    }
}

void CanvasComponent::resized()
{
    // Canvas fills entire bounds - no chrome
}

//==============================================================================
// Mouse Interaction
//==============================================================================

void CanvasComponent::mouseDown(const juce::MouseEvent& event)
{
    lastMousePos = event.position;
    isDragging = true;
    
    // Start new paint stroke
    auto spectralPos = screenToSpectral(event.position);
    processPaintStroke(spectralPos, 1.0f);
    
    // Create visual paint stroke
    paintStrokes.emplace_back(juce::Colours::cyan, 1.0f);
    paintStrokes.back().path.startNewSubPath(event.position);
    
    repaint();
}

void CanvasComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (!isDragging) return;
    
    lastMousePos = event.position;
    
    // Continue paint stroke
    auto spectralPos = screenToSpectral(event.position);
    processPaintStroke(spectralPos, 1.0f);
    
    // Add to visual path
    if (!paintStrokes.empty())
    {
        paintStrokes.back().path.lineTo(event.position);
    }
    
    repaint();
}

void CanvasComponent::mouseUp(const juce::MouseEvent& event)
{
    isDragging = false;
    repaint();
}

void CanvasComponent::mouseMove(const juce::MouseEvent& event)
{
    lastMousePos = event.position;
    repaint();
}

//==============================================================================
// Engine Connections
//==============================================================================

void CanvasComponent::setSampleMaskingEngine(SampleMaskingEngine* engine)
{
    sampleMaskingEngine = engine;
}

void CanvasComponent::setSpectralSynthEngine(SpectralSynthEngine* engine)
{
    spectralSynthEngine = engine;
}

void CanvasComponent::setCommandProcessor(std::function<bool(const Command&)> processor)
{
    commandProcessor = processor;
}

//==============================================================================
// Canvas Operations
//==============================================================================

void CanvasComponent::clearCanvas()
{
    paintStrokes.clear();
    repaint();
}

void CanvasComponent::startSpectralUpdates()
{
    // Start real-time updates
}

void CanvasComponent::stopSpectralUpdates()
{
    // Stop real-time updates
}

//==============================================================================
// Coordinate Mapping
//==============================================================================

juce::Point<float> CanvasComponent::screenToSpectral(juce::Point<float> screenPos) const
{
    auto bounds = getLocalBounds().toFloat();
    
    // X = time (0-1)
    float time = screenPos.x / bounds.getWidth();
    
    // Y = frequency (logarithmic, 20Hz - 20kHz)
    float normalizedY = 1.0f - (screenPos.y / bounds.getHeight());
    float logMin = std::log10(20.0f);
    float logMax = std::log10(20000.0f);
    float frequency = std::pow(10.0f, logMin + normalizedY * (logMax - logMin));
    
    return juce::Point<float>(time, frequency);
}

juce::Point<float> CanvasComponent::spectralToScreen(juce::Point<float> spectralPos) const
{
    auto bounds = getLocalBounds().toFloat();
    
    // X = time
    float screenX = spectralPos.x * bounds.getWidth();
    
    // Y = frequency (logarithmic)
    float logMin = std::log10(20.0f);
    float logMax = std::log10(20000.0f);
    float normalizedY = (std::log10(spectralPos.y) - logMin) / (logMax - logMin);
    float screenY = (1.0f - normalizedY) * bounds.getHeight();
    
    return juce::Point<float>(screenX, screenY);
}

//==============================================================================
// Canvas Rendering
//==============================================================================

void CanvasComponent::drawMinimalGrid(juce::Graphics& g) const
{
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(juce::Colour(ModernCanvasColors::gridSubtle));
    
    // Vertical time grid (every second for 5 seconds)
    for (int i = 1; i < 5; ++i)
    {
        float x = (i / 5.0f) * bounds.getWidth();
        g.drawVerticalLine(static_cast<int>(x), 0, bounds.getHeight());
    }
    
    // Horizontal frequency grid (major frequencies)
    std::vector<float> majorFreqs = {100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f};
    
    for (float freq : majorFreqs)
    {
        auto screenPos = spectralToScreen(juce::Point<float>(0.5f, freq));
        g.drawHorizontalLine(static_cast<int>(screenPos.y), 0, bounds.getWidth());
    }
}

void CanvasComponent::drawPaintStrokes(juce::Graphics& g) const
{
    for (const auto& stroke : paintStrokes)
    {
        // Glow effect
        g.setColour(stroke.color.withAlpha(0.3f));
        juce::PathStrokeType glowStroke(18.0f);
        g.strokePath(stroke.path, glowStroke);
        
        // Main stroke
        g.setColour(stroke.color.withAlpha(0.8f));
        juce::PathStrokeType mainStroke(12.0f);
        g.strokePath(stroke.path, mainStroke);
    }
}

void CanvasComponent::drawCrosshair(juce::Graphics& g, juce::Point<float> pos) const
{
    g.setColour(juce::Colour(ModernCanvasColors::paintGlow).withAlpha(0.6f));
    
    auto bounds = getLocalBounds().toFloat();
    
    // Crosshair lines
    g.drawVerticalLine(static_cast<int>(pos.x), 0, bounds.getHeight());
    g.drawHorizontalLine(static_cast<int>(pos.y), 0, bounds.getWidth());
    
    // Center dot
    g.fillEllipse(pos.x - 3, pos.y - 3, 6, 6);
}

//==============================================================================
// Audio Interaction
//==============================================================================

void CanvasComponent::processPaintStroke(juce::Point<float> spectralPos, float pressure)
{
    if (spectralSynthEngine)
    {
        // DEBUG: Log paint stroke details
        DBG("*** PAINT STROKE *** Freq: " << spectralPos.y << "Hz, Time: " << spectralPos.x << ", Pressure: " << pressure);
        
        // Create spectral oscillator at painted frequency
        spectralSynthEngine->addSpectralOscillator(spectralPos.y, pressure * 0.8f, juce::Colours::cyan);
    }
    
    if (sampleMaskingEngine)
    {
        // Process paint stroke with sample masking
        sampleMaskingEngine->addPointToMask(1, spectralPos.x, spectralPos.y / 20000.0f, pressure);
    }
}