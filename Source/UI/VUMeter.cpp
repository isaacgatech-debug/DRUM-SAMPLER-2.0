#include "VUMeter.h"
#include "PluginColors.h"

VUMeter::VUMeter(Orientation orient)
    : orientation(orient)
{
    startTimerHz(30);
}

VUMeter::~VUMeter()
{
    stopTimer();
}

void VUMeter::setLevel(float newLevel)
{
    level = juce::jlimit(0.0f, 1.0f, newLevel);
}

void VUMeter::setPeak(float newPeak)
{
    peak = juce::jlimit(0.0f, 1.0f, newPeak);
}

void VUMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Clean dark background
    g.setColour(PluginColors::bg());
    g.fillRect(bounds);

    // 1px border
    g.setColour(PluginColors::border());
    g.drawRect(bounds, 1.0f);

    auto meterBounds = bounds.reduced(2.0f);

    if (orientation == Orientation::Vertical)
        drawVerticalMeter(g, meterBounds);
    else
        drawHorizontalMeter(g, meterBounds);
}

void VUMeter::timerCallback()
{
    const float decayFactor = 0.88f;
    displayedLevel = displayedLevel * decayFactor + level * (1.0f - decayFactor);

    if (peak > displayedPeak)
        displayedPeak = peak;
    else
        displayedPeak *= 0.997f;

    repaint();
}

void VUMeter::drawVerticalMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const float gap       = 1.0f;
    const float ledHeight = (bounds.getHeight() - (numLeds - 1) * gap) / numLeds;
    const float ledWidth  = bounds.getWidth();

    const float threshold     = displayedLevel * numLeds;
    const float peakThreshold = displayedPeak  * numLeds;

    for (int i = 0; i < numLeds; ++i)
    {
        // LEDs fill from bottom to top — ledIndex 0 is the bottom
        int   ledIndex = numLeds - 1 - i;
        float y        = bounds.getY() + i * (ledHeight + gap);

        juce::Rectangle<float> ledRect(bounds.getX(), y, ledWidth, ledHeight);
        juce::Colour color = getLedColour(ledIndex);

        bool isActive = ledIndex < threshold;
        bool isPeak   = (ledIndex < peakThreshold + 0.5f) && (ledIndex > threshold - 1.5f);

        if (isActive)
        {
            g.setColour(color);
            g.fillRect(ledRect);
            // Subtle highlight on top half
            g.setColour(juce::Colours::white.withAlpha(0.12f));
            g.fillRect(ledRect.removeFromTop(ledHeight * 0.35f));
        }
        else if (isPeak)
        {
            // Peak hold — brighter single segment
            g.setColour(color.brighter(0.4f));
            g.fillRect(ledRect);
        }
        else
        {
            // Inactive — very dim
            g.setColour(color.withAlpha(0.08f));
            g.fillRect(ledRect);
        }
    }
}

void VUMeter::drawHorizontalMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const float gap      = 1.0f;
    const float ledWidth = (bounds.getWidth() - (numLeds - 1) * gap) / numLeds;
    const float ledHeight= bounds.getHeight();

    const float threshold     = displayedLevel * numLeds;
    const float peakThreshold = displayedPeak  * numLeds;

    for (int i = 0; i < numLeds; ++i)
    {
        float x = bounds.getX() + i * (ledWidth + gap);
        juce::Rectangle<float> ledRect(x, bounds.getY(), ledWidth, ledHeight);
        juce::Colour color = getLedColour(i);

        bool isActive = i < threshold;
        bool isPeak   = (i < peakThreshold + 0.5f) && (i > threshold - 1.5f);

        if (isActive)
        {
            g.setColour(color);
            g.fillRect(ledRect);
        }
        else if (isPeak)
        {
            g.setColour(color.brighter(0.4f));
            g.fillRect(ledRect);
        }
        else
        {
            g.setColour(color.withAlpha(0.08f));
            g.fillRect(ledRect);
        }
    }
}

juce::Colour VUMeter::getLedColour(int ledIndex) const
{
    // 18 segments: 0-12 green (0-70%), 13-15 yellow (70-85%), 16-17 red (85-100%)
    if (ledIndex < 13)
        return juce::Colour(PluginColors::meterGreen);
    else if (ledIndex < 16)
        return juce::Colour(PluginColors::meterYellow);
    else
        return juce::Colour(PluginColors::meterRed);
}
