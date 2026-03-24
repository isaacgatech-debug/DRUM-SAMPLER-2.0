#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <memory>
#include "ChannelStrip.h"

class DrumSampler2Processor;

/**
 * Main mixer window with 12 channel strips
 * Punk rock analog console styling
 */
class MixerView : public juce::Component, private juce::Timer
{
public:
    static constexpr int numChannels = 12;
    
    MixerView();
    ~MixerView() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setProcessor(DrumSampler2Processor* proc);
    
private:
    void timerCallback() override;
    void createChannelStrips();
    void drawMixerFrame(juce::Graphics& g);
    void drawMasterSection(juce::Graphics& g, juce::Rectangle<float> bounds);
    
    DrumSampler2Processor* processor = nullptr;
    
    std::array<std::unique_ptr<ChannelStrip>, numChannels> channelStrips;
    
    // Default channel names for drum inputs
    const std::array<juce::String, numChannels> channelNames = {
        "Kick In", "Kick Out", "Snare Top", "Snare Bottom",
        "Tom 1", "Tom 2", "Tom 3", "OVH L", "OVH R",
        "Hat", "RM L", "RM R"
    };
    
    // Styling
    juce::Colour bgColor{0xFF1A1A1A};
    juce::Colour metalColor{0xFF3A3A3A};
    juce::Colour darkMetal{0xFF222222};
    juce::Colour accentColor{0xFFE8A020};
    
    static constexpr int stripWidth = 85;
    static constexpr int masterWidth = 100;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerView)
};
