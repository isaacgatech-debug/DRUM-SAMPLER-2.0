#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <memory>
#include "ChannelStrip.h"
#include "PluginColors.h"
#include "../Core/MicBusLayout.h"

class DrumTechProcessor;

/**
 * Spacious analog console mixer.
 * 96px left labels sidebar (INPUT/EQ/SENDS/S·M/VU/FADER/PAN),
 * Viewport with horizontal scroll for mic stem channel strips,
 * 128px master strip pinned right, 2px separator.
 */
class MixerView : public juce::Component, private juce::Timer
{
public:
    static constexpr int numChannels  = MicBus::count;
    static constexpr int sidebarW    = 96;
    static constexpr int masterW     = 128;

    MixerView();
    ~MixerView() override;

    void paint  (juce::Graphics& g) override;
    void resized() override;

    void setProcessor(DrumTechProcessor* proc);

private:
    void timerCallback() override;
    void createChannelStrips();
    void drawSidebar (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawMasterSection(juce::Graphics& g, juce::Rectangle<float> bounds);

    DrumTechProcessor* processor = nullptr;

    std::array<std::unique_ptr<ChannelStrip>, numChannels> channelStrips;

    // Container component that holds all strips (lives inside viewport)
    struct StripsContainer : public juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(PluginColors::pluginBg).withAlpha(0.78f));
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
