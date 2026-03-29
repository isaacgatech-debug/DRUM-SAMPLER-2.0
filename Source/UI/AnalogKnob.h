#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Rotary control with a neutral “console” look (flat metal, minimal garnish).
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
    juce::Colour accentColor{0xFFD4A74A};
    juce::Colour metalColor{0xFF505050};
    juce::Colour darkMetal{0xFF353535};
    juce::String labelText;
    
    void drawKnobFace(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawKnobIndicator(juce::Graphics& g, juce::Rectangle<float> bounds, float angle);
    void drawTickMarks(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawLabel(juce::Graphics& g, juce::Rectangle<float> bounds);
    
    float getKnobRadius() const;
    float getValueAngle() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogKnob)
};
