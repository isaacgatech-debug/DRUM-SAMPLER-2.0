#include "VUMeter.h"

VUMeter::VUMeter(Orientation orient)
    : orientation(orient)
{
    startTimerHz(30); // 30 FPS update rate
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
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    
    drawMetalFrame(g, bounds);
    
    auto meterBounds = bounds.reduced(4.0f);
    
    if (orientation == Orientation::Vertical)
        drawVerticalMeter(g, meterBounds);
    else
        drawHorizontalMeter(g, meterBounds);
}

void VUMeter::timerCallback()
{
    // Smooth level decay
    const float decayFactor = 0.9f;
    displayedLevel = displayedLevel * decayFactor + level * (1.0f - decayFactor);
    
    // Peak hold with slower decay
    if (peak > displayedPeak)
        displayedPeak = peak;
    else
        displayedPeak *= 0.995f;
    
    repaint();
}

void VUMeter::drawMetalFrame(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Dark metal background
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.fillRect(bounds);
    
    // Beveled edge effect
    g.setColour(juce::Colour(0xFF4A4A4A));
    g.drawRect(bounds, 1.0f);
    
    // Inner shadow
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.drawRect(bounds.reduced(1.0f), 1.0f);
    
    // Punk accent corners
    float cornerSize = 4.0f;
    g.setColour(juce::Colour(0xFFE8A020).withAlpha(0.5f));
    
    // Top-left corner
    g.drawLine(bounds.getX(), bounds.getY() + cornerSize, bounds.getX(), bounds.getY());
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getX() + cornerSize, bounds.getY());
    
    // Top-right corner
    g.drawLine(bounds.getRight(), bounds.getY() + cornerSize, bounds.getRight(), bounds.getY());
    g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight() - cornerSize, bounds.getY());
    
    // Bottom-left corner
    g.drawLine(bounds.getX(), bounds.getBottom() - cornerSize, bounds.getX(), bounds.getBottom());
    g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getX() + cornerSize, bounds.getBottom());
    
    // Bottom-right corner
    g.drawLine(bounds.getRight(), bounds.getBottom() - cornerSize, bounds.getRight(), bounds.getBottom());
    g.drawLine(bounds.getRight(), bounds.getBottom(), bounds.getRight() - cornerSize, bounds.getBottom());
}

void VUMeter::drawVerticalMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float ledHeight = (bounds.getHeight() - (numLeds - 1) * 1.0f) / numLeds;
    float ledWidth = bounds.getWidth() - 4.0f;
    
    float threshold = displayedLevel * numLeds;
    float peakThreshold = displayedPeak * numLeds;
    
    for (int i = 0; i < numLeds; ++i)
    {
        // LEDs go from bottom to top
        int ledIndex = numLeds - 1 - i;
        float y = bounds.getY() + i * (ledHeight + 1.0f) + 2.0f;
        
        juce::Rectangle<float> ledRect(bounds.getX() + 2.0f, y, ledWidth, ledHeight);
        
        // Determine LED state
        bool isActive = ledIndex < threshold;
        bool isPeak = ledIndex < peakThreshold && ledIndex >= threshold - 1;
        
        juce::Colour ledColor = getLedColour(ledIndex, numLeds);
        
        if (isActive)
        {
            // Active LED - bright with glow
            g.setColour(ledColor);
            g.fillRect(ledRect);
            
            // Highlight
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.fillRect(ledRect.removeFromTop(ledHeight * 0.3f));
        }
        else if (isPeak && (juce::Time::getMillisecondCounter() / 100) % 2 == 0)
        {
            // Blinking peak indicator
            g.setColour(ledColor.brighter(0.5f));
            g.fillRect(ledRect);
        }
        else
        {
            // Inactive LED - dark
            g.setColour(ledColor.darker(0.7f).withAlpha(0.3f));
            g.fillRect(ledRect);
        }
        
        // LED border
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRect(ledRect, 0.5f);
    }
}

void VUMeter::drawHorizontalMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float ledWidth = (bounds.getWidth() - (numLeds - 1) * 1.0f) / numLeds;
    float ledHeight = bounds.getHeight() - 4.0f;
    
    float threshold = displayedLevel * numLeds;
    float peakThreshold = displayedPeak * numLeds;
    
    for (int i = 0; i < numLeds; ++i)
    {
        float x = bounds.getX() + i * (ledWidth + 1.0f) + 2.0f;
        
        juce::Rectangle<float> ledRect(x, bounds.getY() + 2.0f, ledWidth, ledHeight);
        
        bool isActive = i < threshold;
        bool isPeak = i < peakThreshold && i >= threshold - 1;
        
        juce::Colour ledColor = getLedColour(i, numLeds);
        
        if (isActive)
        {
            g.setColour(ledColor);
            g.fillRect(ledRect);
            
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.fillRect(ledRect.removeFromLeft(ledWidth * 0.3f));
        }
        else if (isPeak && (juce::Time::getMillisecondCounter() / 100) % 2 == 0)
        {
            g.setColour(ledColor.brighter(0.5f));
            g.fillRect(ledRect);
        }
        else
        {
            g.setColour(ledColor.darker(0.7f).withAlpha(0.3f));
            g.fillRect(ledRect);
        }
        
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRect(ledRect, 0.5f);
    }
}

juce::Colour VUMeter::getLedColour(int ledIndex, int totalLeds) const
{
    // Color gradient from green -> yellow -> orange -> red
    float ratio = static_cast<float>(ledIndex) / totalLeds;
    
    if (ratio < 0.6f)
    {
        // Green zone
        return juce::Colour(0xFF00FF44);
    }
    else if (ratio < 0.8f)
    {
        // Yellow zone
        return juce::Colour(0xFFFFDD00);
    }
    else if (ratio < 0.95f)
    {
        // Orange zone
        return juce::Colour(0xFFFF8800);
    }
    else
    {
        // Red zone - clip warning
        return juce::Colour(0xFFFF2222);
    }
}
