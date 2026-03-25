#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * DAW-style segmented VU meter.
 * 18 segments: green (0-70%), yellow (70-85%), red (85-100%).
 * Clean dark background, no punk metal frame.
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
    float getPeak()  const { return peak; }

private:
    void timerCallback() override;

    Orientation orientation;

    float level          = 0.0f;
    float peak           = 0.0f;
    float displayedLevel = 0.0f;
    float displayedPeak  = 0.0f;

    // 18 segments: green 0-12 (0-70%), yellow 13-15 (70-85%), red 16-17 (85-100%)
    static constexpr int numLeds = 18;

    void drawVerticalMeter  (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawHorizontalMeter(juce::Graphics& g, juce::Rectangle<float> bounds);

    juce::Colour getLedColour(int ledIndex) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};
