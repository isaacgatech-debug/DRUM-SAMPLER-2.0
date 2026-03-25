#include "AnalogKnob.h"

AnalogKnob::AnalogKnob(Style knobStyle)
    : style(knobStyle)
{
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    
    // Set rotary parameters based on style
    float startAngle = juce::MathConstants<float>::pi * 1.25f;
    float endAngle = juce::MathConstants<float>::pi * 2.75f;
    setRotaryParameters(startAngle, endAngle, true);
}

void AnalogKnob::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Determine size based on style
    float size = getKnobRadius() * 2.0f;
    auto knobBounds = bounds.withSizeKeepingCentre(size, size);
    
    drawKnobFace(g, knobBounds);
    drawTickMarks(g, knobBounds);
    drawKnobIndicator(g, knobBounds, getValueAngle());
    drawLabel(g, bounds);
}

void AnalogKnob::drawKnobFace(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float radius = bounds.getWidth() / 2.0f;
    juce::Point<float> center = bounds.getCentre();
    
    // Outer metal ring with distress effect
    juce::Path outerRing;
    outerRing.addEllipse(bounds.reduced(2.0f));
    
    g.setGradientFill(juce::ColourGradient(
        metalColor.brighter(0.2f), center.x, center.y - radius,
        darkMetal, center.x, center.y + radius,
        false));
    g.fillPath(outerRing);
    
    // Inner shadow ring for depth
    g.setColour(darkMetal);
    g.drawEllipse(bounds.reduced(2.0f), 2.0f);
    
    // Main knob face - distressed dark metal
    auto faceBounds = bounds.reduced(6.0f);
    juce::Path face;
    face.addEllipse(faceBounds);
    
    // Create radial gradient for 3D metal effect
    g.setGradientFill(juce::ColourGradient(
        darkMetal.brighter(0.1f), center.x - radius * 0.3f, center.y - radius * 0.3f,
        darkMetal.darker(0.2f), center.x + radius * 0.3f, center.y + radius * 0.3f,
        false));
    g.fillPath(face);
    
    // Punk accent ring near edge
    auto accentBounds = bounds.reduced(8.0f);
    g.setColour(accentColor.withAlpha(0.6f));
    g.drawEllipse(accentBounds, 1.5f);
    
    // Screw marks for analog feel
    float screwRadius = radius * 0.7f;
    for (int i = 0; i < 4; ++i)
    {
        float angle = juce::MathConstants<float>::halfPi * i + juce::MathConstants<float>::pi / 4;
        float sx = center.x + std::cos(angle) * screwRadius;
        float sy = center.y + std::sin(angle) * screwRadius;
        
        g.setColour(darkMetal.darker(0.3f));
        g.fillEllipse(sx - 2.0f, sy - 2.0f, 4.0f, 4.0f);
        g.setColour(metalColor.brighter(0.1f));
        g.fillEllipse(sx - 1.0f, sy - 1.0f, 2.0f, 2.0f);
    }
}

void AnalogKnob::drawKnobIndicator(juce::Graphics& g, juce::Rectangle<float> bounds, float angle)
{
    float radius = bounds.getWidth() / 2.0f;
    juce::Point<float> center = bounds.getCentre();
    
    // Calculate indicator position
    float indicatorRadius = radius * 0.55f;
    float ix = center.x + std::cos(angle) * indicatorRadius;
    float iy = center.y + std::sin(angle) * indicatorRadius;
    
    // Draw indicator dot
    float dotSize = style == Style::Large ? 8.0f : (style == Style::Small ? 4.0f : 6.0f);
    
    // Glow effect
    g.setColour(accentColor.withAlpha(0.4f));
    g.fillEllipse(ix - dotSize/2 - 2, iy - dotSize/2 - 2, dotSize + 4, dotSize + 4);
    
    // Main dot
    g.setColour(accentColor);
    g.fillEllipse(ix - dotSize/2, iy - dotSize/2, dotSize, dotSize);
    
    // Highlight
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.fillEllipse(ix - dotSize/4, iy - dotSize/4, dotSize/2, dotSize/2);
}

void AnalogKnob::drawTickMarks(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float radius = bounds.getWidth() / 2.0f;
    juce::Point<float> center = bounds.getCentre();
    
    // Tick mark parameters
    float startAngle = juce::MathConstants<float>::pi * 1.25f;
    float endAngle = juce::MathConstants<float>::pi * 2.75f;
    int numTicks = style == Style::Large ? 11 : 7;
    
    float innerRadius = radius * 0.75f;
    float outerRadius = radius * 0.85f;
    
    g.setColour(metalColor.brighter(0.3f));
    
    for (int i = 0; i < numTicks; ++i)
    {
        float t = static_cast<float>(i) / (numTicks - 1);
        float angle = startAngle + t * (endAngle - startAngle);
        
        float x1 = center.x + std::cos(angle) * innerRadius;
        float y1 = center.y + std::sin(angle) * innerRadius;
        float x2 = center.x + std::cos(angle) * outerRadius;
        float y2 = center.y + std::sin(angle) * outerRadius;
        
        float thickness = (i == 0 || i == numTicks - 1 || i == (numTicks - 1) / 2) ? 2.0f : 1.0f;
        g.drawLine(x1, y1, x2, y2, thickness);
    }
}

void AnalogKnob::drawLabel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (labelText.isEmpty())
        return;
    
    float fontSize = style == Style::Large ? 12.0f : (style == Style::Small ? 9.0f : 10.0f);
    g.setFont(juce::FontOptions(fontSize, juce::Font::bold));
    g.setColour(juce::Colours::lightgrey);
    
    // Draw label at bottom
    auto labelBounds = bounds.removeFromBottom(16.0f);
    g.drawText(labelText, labelBounds, juce::Justification::centred, false);
}

float AnalogKnob::getKnobRadius() const
{
    float baseSize = static_cast<float>(juce::jmin(getWidth(), getHeight()));
    
    switch (style)
    {
        case Style::Small:  return baseSize * 0.35f;
        case Style::Large:  return baseSize * 0.45f;
        default:            return baseSize * 0.40f;
    }
}

float AnalogKnob::getValueAngle() const
{
    float proportion = (getValue() - getMinimum()) / (getMaximum() - getMinimum());
    float startAngle = juce::MathConstants<float>::pi * 1.25f;
    float endAngle = juce::MathConstants<float>::pi * 2.75f;
    return startAngle + proportion * (endAngle - startAngle);
}
