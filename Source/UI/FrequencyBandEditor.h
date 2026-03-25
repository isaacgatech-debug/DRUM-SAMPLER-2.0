#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include "PluginColors.h"

/**
 * Log-scale multiband frequency threshold editor.
 * Purpose: like a multiband compressor — each band monitors its freq range independently.
 * Enables ghost-note detection (e.g., 200 Hz hit won't be masked by 8 kHz cymbal bleed).
 *
 * freqToPercent(f) = log10(f / 20) / log10(1000) * 100
 * percentToFreq(p) = 20 * pow(1000, p / 100)
 */
class FrequencyBandEditor : public juce::Component
{
public:
    struct FrequencyBand
    {
        float freqLow    = 20.0f;
        float freqHigh   = 200.0f;
        float threshold  = 0.5f;
        juce::Colour color;
        bool  enabled    = true;
    };

    FrequencyBandEditor();

    void paint    (juce::Graphics& g) override;
    void resized  () override;
    void mouseDown(const juce::MouseEvent& e) override;

    void addDefaultBands();
    const std::vector<FrequencyBand>& getBands() const { return bands; }

    // Called when bands change
    std::function<void()> onBandsChanged;

private:
    // Log-scale helpers
    static float freqToPercent(float freq);
    static float percentToFreq(float percent);

    float freqToX(float freq, float areaX, float areaW) const;

    void drawDisplay    (juce::Graphics& g, juce::Rectangle<float> displayArea);
    void drawRuler      (juce::Graphics& g, juce::Rectangle<float> rulerArea);
    void drawBandList   (juce::Graphics& g);
    void drawEditControls(juce::Graphics& g);

    void rebuildLayout();
    void selectBand(int index);
    void deleteBand(int index);
    void addBand();

    std::vector<FrequencyBand> bands;
    int selectedBand = -1;

    // Layout rects (computed in resized)
    juce::Rectangle<int> displayRect;
    juce::Rectangle<int> rulerRect;
    juce::Rectangle<int> bandListRect;
    juce::Rectangle<int> editRect;
    juce::Rectangle<int> addButtonRect;

    // Controls for selected band
    juce::Slider freqLowSlider, freqHighSlider, thresholdSlider;
    juce::TextButton addBandButton{"+ Add Band"};

    // Band-specific palette
    static const juce::Colour bandPalette[];
    static const int paletteSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyBandEditor)
};
