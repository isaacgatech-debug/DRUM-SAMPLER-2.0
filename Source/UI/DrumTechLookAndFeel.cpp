#include "DrumTechLookAndFeel.h"

int DrumTechLookAndFeel::getSliderThumbRadius(juce::Slider& slider)
{
    return juce::jmin(UIDesign::linearSliderThumbRadius,
                      slider.isHorizontal() ? static_cast<int>((float)slider.getHeight() * 0.48f)
                                            : static_cast<int>((float)slider.getWidth() * 0.48f));
}

void DrumTechLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float minSliderPos, float maxSliderPos,
                                           const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (slider.isBar())
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    // Console-style vertical fader (mixer / master)
    if (!slider.isHorizontal()
        && static_cast<bool>(slider.getProperties()["drumtechConsoleFader"]))
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(width), static_cast<float>(height));
        const float trackW = juce::jlimit(5.0f, 10.0f, bounds.getWidth() * 0.28f);
        auto track = bounds.withSizeKeepingCentre(trackW, bounds.getHeight() - 6.0f);

        g.setColour(juce::Colour(PluginColors::pluginBg).brighter(0.08f));
        g.fillRoundedRectangle(track, trackW * 0.45f);
        g.setColour(juce::Colour(PluginColors::pluginBorder));
        g.drawRoundedRectangle(track, trackW * 0.45f, 1.0f);

        // Center zero / unity line
        const float midY = track.getCentreY();
        g.setColour(juce::Colour(PluginColors::pluginBorder).withAlpha(0.55f));
        g.drawHorizontalLine(static_cast<int>(midY), track.getX() + 1.0f, track.getRight() - 1.0f);

        // Ticks (dB-ish marks)
        g.setColour(juce::Colour(PluginColors::textDim).withAlpha(0.35f));
        for (int i = 0; i <= 8; ++i)
        {
            const float t = static_cast<float>(i) / 8.0f;
            const float ty = track.getY() + t * track.getHeight();
            const float tw = (i % 2 == 0) ? 4.0f : 2.5f;
            g.drawHorizontalLine(static_cast<int>(ty), track.getX() - tw - 1.0f, track.getX() - 1.0f);
        }

        const float capW = juce::jmin(bounds.getWidth() - 2.0f, trackW + 10.0f);
        const float capH = juce::jlimit(10.0f, 16.0f, bounds.getWidth() * 0.85f);
        auto cap = juce::Rectangle<float>(capW, capH).withCentre({ track.getCentreX(), sliderPos });

        juce::ColourGradient grad(juce::Colour(PluginColors::pluginSurfaceHi), cap.getX(), cap.getY(),
                                  juce::Colour(PluginColors::pluginSurface).darker(0.15f), cap.getX(), cap.getBottom(),
                                  false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(cap, 2.0f);
        g.setColour(juce::Colour(PluginColors::pluginBorder));
        g.drawRoundedRectangle(cap, 2.0f, 1.0f);
        return;
    }

    const auto isTwoVal   = (style == juce::Slider::SliderStyle::TwoValueVertical
                             || style == juce::Slider::SliderStyle::TwoValueHorizontal);
    const auto isThreeVal = (style == juce::Slider::SliderStyle::ThreeValueVertical
                             || style == juce::Slider::SliderStyle::ThreeValueHorizontal);

    if (isTwoVal || isThreeVal)
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    // Single-thumb linear: thicker track than V4 (was capped at 6px).
    const float trackWidth = juce::jlimit(
        6.0f,
        UIDesign::linearSliderTrackMax,
        slider.isHorizontal() ? (float)height * 0.42f : (float)width * 0.42f);

    juce::Point<float> startPoint(slider.isHorizontal() ? (float)x : (float)x + (float)width * 0.5f,
                                  slider.isHorizontal() ? (float)y + (float)height * 0.5f : (float)(height + y));

    juce::Point<float> endPoint(slider.isHorizontal() ? (float)(width + x) : startPoint.x,
                                slider.isHorizontal() ? startPoint.y : (float)y);

    juce::Path backgroundTrack;
    backgroundTrack.startNewSubPath(startPoint);
    backgroundTrack.lineTo(endPoint);
    g.setColour(slider.findColour(juce::Slider::backgroundColourId));
    g.strokePath(backgroundTrack, {trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});

    const auto kx = slider.isHorizontal() ? sliderPos : ((float)x + (float)width * 0.5f);
    const auto ky = slider.isHorizontal() ? ((float)y + (float)height * 0.5f) : sliderPos;

    const juce::Point<float> minPoint = startPoint;
    const juce::Point<float> maxPoint{kx, ky};

    juce::Path valueTrack;
    valueTrack.startNewSubPath(minPoint);
    valueTrack.lineTo(maxPoint);
    g.setColour(slider.findColour(juce::Slider::trackColourId));
    g.strokePath(valueTrack, {trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});

    const int thumbW = getSliderThumbRadius(slider);
    g.setColour(slider.findColour(juce::Slider::thumbColourId));
    g.fillEllipse(
        juce::Rectangle<float>((float)thumbW, (float)thumbW).withCentre(maxPoint));
}
