#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class RoutingView : public juce::Component
{
public:
    RoutingView();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RoutingView)
};
