#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class InteractiveDrumPad : public juce::Component
{
public:
    InteractiveDrumPad();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InteractiveDrumPad)
};
