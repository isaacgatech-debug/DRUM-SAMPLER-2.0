#include "MixerUI.h"

MixerUI::MixerUI()
{
}

void MixerUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.drawText("Mixer", getLocalBounds(), juce::Justification::centred);
}

void MixerUI::resized()
{
}
