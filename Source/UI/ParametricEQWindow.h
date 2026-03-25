#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginColors.h"
#include <vector>
#include <cmath>

/**
 * Pro-Q-3 inspired parametric EQ editor.
 * 20Hz - 20kHz log scale. -24dB to +24dB gain range.
 * Draggable band nodes. Biquad-computed frequency response.
 * Mouse wheel changes Q on selected band.
 */
class ParametricEQEditor : public juce::Component
{
public:
    struct Band
    {
        float freq    = 1000.0f;
        float gain    = 0.0f;
        float q       = 0.707f;
        bool  enabled = true;
        juce::Colour color;
        enum Type { Peak, LowShelf, HighShelf, LowPass, HighPass } type = Peak;

        juce::String typeName() const
        {
            const char* names[] = {"Peak","Low Shelf","High Shelf","Low Pass","High Pass"};
            return names[static_cast<int>(type)];
        }
    };

    ParametricEQEditor();

    void paint         (juce::Graphics& g) override;
    void resized       () override;
    void mouseDown     (const juce::MouseEvent& e) override;
    void mouseDrag     (const juce::MouseEvent& e) override;
    void mouseUp       (const juce::MouseEvent& e) override;
    void mouseMove     (const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e,
                        const juce::MouseWheelDetails& w) override;

    const std::vector<Band>& getBands() const { return bands; }

private:
    juce::Rectangle<float> displayArea() const;

    static float freqToX(float freq, float w);
    static float xToFreq(float x, float w);
    static float gainToY(float gain, float h);
    static float yToGain(float y, float h);
    static float computeMagnitudeDB(float freq, const Band& b);

    void drawGrid         (juce::Graphics& g, juce::Rectangle<float> area);
    void drawSpectrumFill (juce::Graphics& g, juce::Rectangle<float> area);
    void drawResponseCurve(juce::Graphics& g, juce::Rectangle<float> area);
    void drawNodes        (juce::Graphics& g, juce::Rectangle<float> area);
    void drawBandInfoBar  (juce::Graphics& g, juce::Rectangle<float> area);
    void drawTypeButtons  (juce::Graphics& g, juce::Rectangle<float> area);

    int  hitTestBand(juce::Point<float> pt) const;
    void addBand(float freq, float gain);

    std::vector<Band> bands;
    int  dragBand     = -1;
    int  hovBand      = -1;
    int  selectedBand = -1;
    juce::Point<float> dragStart;
    float dragStartFreq = 0.0f;
    float dragStartGain = 0.0f;

    juce::Rectangle<int> typeBtnRect[5]; // type selection buttons

    static const juce::Colour palette[];
    static const int paletteSize;

    static constexpr float minFreq = 20.0f;
    static constexpr float maxFreq = 20000.0f;
    static constexpr float maxGain = 24.0f;
    static constexpr float nodeRadius = 8.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEQEditor)
};

/**
 * Floating DocumentWindow that hosts ParametricEQEditor.
 * Opened by double-clicking the EQ insert slot in ChannelStrip.
 */
class ParametricEQWindow : public juce::DocumentWindow
{
public:
    explicit ParametricEQWindow(const juce::String& channelName);
    ~ParametricEQWindow() override = default;

    void closeButtonPressed() override { setVisible(false); }
    ParametricEQEditor& getEditor() { return eqEditor; }

private:
    ParametricEQEditor eqEditor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEQWindow)
};
