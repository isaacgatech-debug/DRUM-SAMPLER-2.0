#include "PluginBrowser.h"

PluginBrowser::PluginBrowser()
{
}

void PluginBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.drawText("Plugin Browser", getLocalBounds(), juce::Justification::centred);
}

void PluginBrowser::resized()
{
}
