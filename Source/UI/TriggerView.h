#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class TriggerView : public juce::Component
{
public:
    TriggerView() = default;
    ~TriggerView() override = default;

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(24.0f, juce::Font::bold));
        g.drawText("Trigger View - Coming Soon", getLocalBounds(), juce::Justification::centred);
    }

    void resized() override
    {
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerView)
};
