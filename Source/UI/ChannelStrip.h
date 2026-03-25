#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AnalogKnob.h"
#include "VUMeter.h"
#include "PluginColors.h"

class DrumSampler2Processor;
class MixerChannel;

/**
 * Analog console channel strip — DAW-style redesign.
 * 88px wide. Top color bar (3px + glow), mini EQ polyline (56x24px),
 * FX send knobs, [S]/[M] buttons, 18-seg VU meter, vertical fader,
 * dB readout (monospace), pan slider.
 */
class ChannelStrip : public juce::Component, private juce::Button::Listener
{
public:
    ChannelStrip(int channelIndex, const juce::String& channelName);

    void paint  (juce::Graphics& g) override;
    void resized() override;

    void setProcessor(DrumSampler2Processor* proc);
    void updateMeter(float level, float peak);

    int          getChannelIndex() const { return index; }
    juce::String getChannelName()  const { return name;  }

    std::function<void(int, const juce::String&, float)> onParameterChanged;
    std::function<void(int, const juce::String&, bool)>  onButtonChanged;
    std::function<void(int, int)>                        onOutputBusChanged;
    std::function<void(int, int)>                        onInputSourceChanged;

    static constexpr int stripWidth = 88;

private:
    void buttonClicked(juce::Button* button) override;

    void drawColorBar     (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawHeader       (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawMiniEQ       (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawSectionLabel (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& label);

    int          index;
    juce::String name;
    juce::String shortName;
    DrumSampler2Processor* processor = nullptr;

    juce::Colour channelColor;

    // FX Send knobs
    AnalogKnob fx1Knob{AnalogKnob::Style::Small};
    AnalogKnob fx2Knob{AnalogKnob::Style::Small};

    // Solo / Mute
    juce::TextButton soloBtn{"S"};
    juce::TextButton muteBtn{"M"};

    // VU meter
    VUMeter vuMeter{VUMeter::Orientation::Vertical};

    // Fader (vertical)
    juce::Slider fader;

    // dB readout
    juce::Label dbReadout;

    // Pan slider (horizontal)
    juce::Slider panSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStrip)
};
