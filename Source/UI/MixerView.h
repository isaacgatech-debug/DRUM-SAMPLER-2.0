#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <memory>
#include "ChannelStrip.h"
#include "PluginColors.h"

class DrumSampler2Processor;

/**
 * Spacious analog console mixer.
 * 80px left labels sidebar (INPUT/EQ/SENDS/S·M/VU/FADER/PAN),
 * Viewport with horizontal scroll for 12 channel strips,
 * 100px master strip pinned right, 2px separator.
 */
class MixerView : public juce::Component, private juce::Timer
{
public:
    static constexpr int numChannels  = 12;
    static constexpr int sidebarW    = 80;
    static constexpr int masterW     = 104;

    MixerView();
    ~MixerView() override;

    void paint  (juce::Graphics& g) override;
    void resized() override;

    void setProcessor(DrumSampler2Processor* proc);

private:
    void timerCallback() override;
    void createChannelStrips();
    void drawSidebar (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawMasterSection(juce::Graphics& g, juce::Rectangle<float> bounds);

    DrumSampler2Processor* processor = nullptr;

    std::array<std::unique_ptr<ChannelStrip>, numChannels> channelStrips;

    // Channel name mapping to match handoff
    const std::array<juce::String, numChannels> channelNames = {
        "Kick In", "Kick Out", "Snare Top", "Snare Bot",
        "Hi-Hat", "Tom 1", "Tom 2", "Tom 3",
        "OVH L", "OVH R", "Room L", "Room R"
    };

    // Container component that holds all strips (lives inside viewport)
    struct StripsContainer : public juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(PluginColors::pluginBg));
        }
    };

    StripsContainer stripsContainer;
    juce::Viewport  stripsViewport;

    // Master fader / vol for master section
    juce::Slider masterFader;
    juce::Label  masterDbLabel;
    VUMeter      masterVU{VUMeter::Orientation::Vertical};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerView)
};
