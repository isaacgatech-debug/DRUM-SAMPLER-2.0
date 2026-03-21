#include "MixerView.h"

MixerView::MixerView()
{
}

void MixerView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.drawText("Mixer View", getLocalBounds(), juce::Justification::centred);
}

void MixerView::resized()
{
}
