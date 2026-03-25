#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Punk rock styled analog knob component
 * Features distressed metal look with bold industrial markings
 */
class AnalogKnob : public juce::Slider
{
public:
    enum class Style
    {
        Standard,    // Regular parameter knob
        Small,       // Compact knob for dense layouts
        Large        // Big knob for important parameters
    };

    explicit AnalogKnob(Style knobStyle = Style::Standard);
    
    void paint(juce::Graphics& g) override;
    
    void setKnobColor(juce::Colour color) { accentColor = color; repaint(); }
    void setLabel(const juce::String& text) { labelText = text; repaint(); }
    
private:
    Style style;
    juce::Colour accentColor{0xFFE8A020};  // Punk orange accent
    juce::Colour metalColor{0xFF4A4A4A};   // Distressed metal
    juce::Colour darkMetal{0xFF2A2A2A};    // Dark metal shadow
    juce::String labelText;
    
    void drawKnobFace(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawKnobIndicator(juce::Graphics& g, juce::Rectangle<float> bounds, float angle);
    void drawTickMarks(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawLabel(juce::Graphics& g, juce::Rectangle<float> bounds);
    
    float getKnobRadius() const;
    float getValueAngle() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogKnob)
};
