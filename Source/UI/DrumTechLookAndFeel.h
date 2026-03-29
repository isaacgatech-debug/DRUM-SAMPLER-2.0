#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginColors.h"

/**
 * Larger fonts and thicker linear sliders than LookAndFeel_V4 defaults.
 */
class DrumTechLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return PluginFonts::label(juce::jlimit(12.5f, 16.0f, static_cast<float>(buttonHeight) * 0.42f));
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return PluginFonts::label(12.5f);
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return PluginFonts::label(12.5f);
    }

    juce::Font getPopupMenuFont() override
    {
        return PluginFonts::label(12.5f);
    }

    int getSliderThumbRadius(juce::Slider& slider) override;
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override;
};
