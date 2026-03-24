#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Punk rock styled VU meter with LED-style bars
 * Features distressed metal frame with industrial LED styling
 */
class VUMeter : public juce::Component, private juce::Timer
{
public:
    enum class Orientation
    {
        Vertical,
        Horizontal
    };

    explicit VUMeter(Orientation orient = Orientation::Vertical);
    ~VUMeter() override;
    
    void paint(juce::Graphics& g) override;
    
    void setLevel(float newLevel);
    void setPeak(float newPeak);
    
    float getLevel() const { return level; }
    float getPeak() const { return peak; }
    
private:
    void timerCallback() override;
    
    Orientation orientation;
    
    float level = 0.0f;
    float peak = 0.0f;
    float displayedLevel = 0.0f;
    float displayedPeak = 0.0f;
    
    // LED segments
    static constexpr int numLeds = 20;
    
    void drawVerticalMeter(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawHorizontalMeter(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawMetalFrame(juce::Graphics& g, juce::Rectangle<float> bounds);
    
    juce::Colour getLedColour(int ledIndex, int totalLeds) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};
