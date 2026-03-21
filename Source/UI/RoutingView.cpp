#include "RoutingView.h"

RoutingView::RoutingView()
{
}

void RoutingView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.drawText("Routing", getLocalBounds(), juce::Justification::centred);
}

void RoutingView::resized()
{
}
