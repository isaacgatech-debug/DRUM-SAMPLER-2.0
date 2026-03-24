#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AnalogKnob.h"
#include "VUMeter.h"

class DrumSampler2Processor;
class MixerChannel;

/**
 * Single channel strip for the mixer
 * Features punk rock analog console styling with EQ, compressor, and routing
 */
class ChannelStrip : public juce::Component, private juce::Button::Listener
{
public:
    ChannelStrip(int channelIndex, const juce::String& channelName);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setProcessor(DrumSampler2Processor* proc);
    void updateMeter(float level, float peak);
    
    int getChannelIndex() const { return index; }
    juce::String getChannelName() const { return name; }
    
    // Parameter callbacks
    std::function<void(int, const juce::String&, float)> onParameterChanged;
    std::function<void(int, const juce::String&, bool)> onButtonChanged;
    std::function<void(int, int)> onOutputBusChanged;
    std::function<void(int, int)> onInputSourceChanged;
    
private:
    void buttonClicked(juce::Button* button) override;
    void setupKnobs();
    void setupRouting();
    void drawChannelHeader(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawSectionDivider(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& label);
    
    int index;
    juce::String name;
    DrumSampler2Processor* processor = nullptr;
    
    // EQ Section
    AnalogKnob eqLowFreq{AnalogKnob::Style::Small};
    AnalogKnob eqLowGain{AnalogKnob::Style::Small};
    AnalogKnob eqMidFreq{AnalogKnob::Style::Small};
    AnalogKnob eqMidGain{AnalogKnob::Style::Small};
    AnalogKnob eqHighFreq{AnalogKnob::Style::Small};
    AnalogKnob eqHighGain{AnalogKnob::Style::Small};
    
    // Compressor Section
    AnalogKnob compThreshold{AnalogKnob::Style::Small};
    AnalogKnob compRatio{AnalogKnob::Style::Small};
    AnalogKnob compAttack{AnalogKnob::Style::Small};
    AnalogKnob compRelease{AnalogKnob::Style::Small};
    AnalogKnob compMakeup{AnalogKnob::Style::Small};
    
    // Channel Controls
    AnalogKnob gainKnob{AnalogKnob::Style::Standard};
    AnalogKnob panKnob{AnalogKnob::Style::Small};
    
    // Buttons
    juce::TextButton muteButton{"MUTE"};
    juce::TextButton soloButton{"SOLO"};
    juce::TextButton phaseButton{"PHASE"};
    
    // Input source selection
    juce::ComboBox inputSelector;
    
    // DAW input selection (physical audio inputs)
    juce::ComboBox dawInputSelector;
    
    // Output routing
    juce::ComboBox outputSelector;
    
    // Meters
    VUMeter vuMeter{VUMeter::Orientation::Vertical};
    
    // Styling
    juce::Colour channelColor;
    juce::Colour metalColor{0xFF3A3A3A};
    juce::Colour darkMetal{0xFF222222};
    juce::Colour accentColor{0xFFE8A020};
    juce::Colour punkRed{0xFFFF3333};
    juce::Colour punkGreen{0xFF33FF33};
    
    static constexpr int stripWidth = 85;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStrip)
};
