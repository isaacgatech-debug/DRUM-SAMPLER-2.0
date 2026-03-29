#include "FrequencyBandEditor.h"
#include "PluginColors.h"
#include <cmath>

const juce::Colour FrequencyBandEditor::bandPalette[] = {
    juce::Colour(PluginColors::accent),
    juce::Colour(0xFFC49A6A),
    juce::Colour(0xFF9A8AAC),
    juce::Colour(0xFF7FA88A),
    juce::Colour(0xFFB88A9A),
    juce::Colour(0xFFC9B86A),
    juce::Colour(0xFF7A90B0),
};
const int FrequencyBandEditor::paletteSize = 7;

static const float gridFreqs[] = { 20.f, 50.f, 100.f, 200.f, 500.f,
                                    1000.f, 2000.f, 5000.f, 10000.f, 20000.f };
static const char* gridLabels[] = { "20", "50", "100", "200", "500",
                                     "1k", "2k", "5k", "10k", "20k" };
static const int numGridFreqs = 10;

// log10(20000/20) = log10(1000) = 3
float FrequencyBandEditor::freqToPercent(float freq)
{
    return std::log10(freq / 20.0f) / 3.0f * 100.0f;
}

float FrequencyBandEditor::percentToFreq(float percent)
{
    return 20.0f * std::pow(1000.0f, percent / 100.0f);
}

float FrequencyBandEditor::freqToX(float freq, float areaX, float areaW) const
{
    return areaX + freqToPercent(freq) / 100.0f * areaW;
}

FrequencyBandEditor::FrequencyBandEditor()
{
    // Sliders for selected band editing
    auto setupSlider = [this](juce::Slider& s, float min, float max, float val)
    {
        s.setRange(min, max, 0.0f);
        s.setValue(val, juce::dontSendNotification);
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 16);
        s.setColour(juce::Slider::thumbColourId, juce::Colour(PluginColors::accent));
        s.setColour(juce::Slider::trackColourId, juce::Colour(PluginColors::accentDim));
        s.setColour(juce::Slider::backgroundColourId, juce::Colour(PluginColors::pluginBg));
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colour(PluginColors::textPrimary));
        s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(PluginColors::pluginSurface));
        s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(PluginColors::pluginBorder));
        addAndMakeVisible(s);
        s.setVisible(false);
    };

    setupSlider(freqLowSlider,   20.0f, 20000.0f, 20.0f);
    setupSlider(freqHighSlider,  20.0f, 20000.0f, 200.0f);
    setupSlider(thresholdSlider,  0.0f,    1.0f,   0.5f);

    freqLowSlider.onValueChange = [this]
    {
        if (selectedBand >= 0 && selectedBand < static_cast<int>(bands.size()))
        {
            bands[selectedBand].freqLow = static_cast<float>(freqLowSlider.getValue());
            if (onBandsChanged) onBandsChanged();
            repaint();
        }
    };
    freqHighSlider.onValueChange = [this]
    {
        if (selectedBand >= 0 && selectedBand < static_cast<int>(bands.size()))
        {
            bands[selectedBand].freqHigh = static_cast<float>(freqHighSlider.getValue());
            if (onBandsChanged) onBandsChanged();
            repaint();
        }
    };
    thresholdSlider.onValueChange = [this]
    {
        if (selectedBand >= 0 && selectedBand < static_cast<int>(bands.size()))
        {
            bands[selectedBand].threshold = static_cast<float>(thresholdSlider.getValue());
            if (onBandsChanged) onBandsChanged();
            repaint();
        }
    };

    addBandButton.setColour(juce::TextButton::buttonColourId, juce::Colour(PluginColors::pluginSurface));
    addBandButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(PluginColors::accentDim));
    addBandButton.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::accent));
    addBandButton.onClick = [this] { addBand(); };
    addAndMakeVisible(addBandButton);

    addDefaultBands();
}

void FrequencyBandEditor::addDefaultBands()
{
    bands.clear();
    // Sub / low
    bands.push_back({ 20.f, 100.f, 0.5f, bandPalette[0], true });
    // Low-mid
    bands.push_back({ 100.f, 800.f, 0.5f, bandPalette[1], true });
    // High
    bands.push_back({ 2000.f, 20000.f, 0.5f, bandPalette[2], true });
    repaint();
}

void FrequencyBandEditor::addBand()
{
    int colorIdx = static_cast<int>(bands.size()) % paletteSize;
    bands.push_back({ 200.f, 2000.f, 0.5f, bandPalette[colorIdx], true });
    selectBand(static_cast<int>(bands.size()) - 1);
    if (onBandsChanged) onBandsChanged();
    repaint();
}

void FrequencyBandEditor::selectBand(int index)
{
    selectedBand = index;

    bool hasSelection = (index >= 0 && index < static_cast<int>(bands.size()));
    freqLowSlider.setVisible(hasSelection);
    freqHighSlider.setVisible(hasSelection);
    thresholdSlider.setVisible(hasSelection);

    if (hasSelection)
    {
        freqLowSlider.setValue(bands[index].freqLow, juce::dontSendNotification);
        freqHighSlider.setValue(bands[index].freqHigh, juce::dontSendNotification);
        thresholdSlider.setValue(bands[index].threshold, juce::dontSendNotification);
    }

    resized();
    repaint();
}

void FrequencyBandEditor::deleteBand(int index)
{
    if (index >= 0 && index < static_cast<int>(bands.size()))
    {
        bands.erase(bands.begin() + index);
        selectedBand = -1;
        freqLowSlider.setVisible(false);
        freqHighSlider.setVisible(false);
        thresholdSlider.setVisible(false);
        if (onBandsChanged) onBandsChanged();
        resized();
        repaint();
    }
}

void FrequencyBandEditor::resized()
{
    auto area = getLocalBounds();

    // Display: 72px tall
    displayRect = area.removeFromTop(72);
    // Ruler: 16px
    rulerRect = area.removeFromTop(16);

    // Edit controls for selected band
    bool hasSelection = (selectedBand >= 0 && selectedBand < static_cast<int>(bands.size()));
    if (hasSelection)
    {
        auto editArea = area.removeFromTop(60);
        auto row1 = editArea.removeFromTop(20);
        freqLowSlider.setBounds(row1);
        auto row2 = editArea.removeFromTop(20);
        freqHighSlider.setBounds(row2);
        auto row3 = editArea.removeFromTop(20);
        thresholdSlider.setBounds(row3);
    }

    // Band list rows
    bandListRect = area;

    // Add Band button at bottom
    addButtonRect = area.removeFromBottom(22);
    addBandButton.setBounds(addButtonRect.reduced(2, 2));
}

void FrequencyBandEditor::paint(juce::Graphics& g)
{
    auto displayF = displayRect.toFloat();
    auto rulerF   = rulerRect.toFloat();

    drawDisplay(g, displayF);
    drawRuler(g, rulerF);
    drawBandList(g);
}

void FrequencyBandEditor::drawDisplay(juce::Graphics& g, juce::Rectangle<float> area)
{
    // Background
    g.setColour(PluginColors::bg());
    g.fillRect(area);
    g.setColour(PluginColors::border());
    g.drawRect(area, 1.0f);

    auto inner = area.reduced(1.0f);
    float aX = inner.getX(), aW = inner.getWidth(), aY = inner.getY(), aH = inner.getHeight();

    // Vertical gridlines at standard freq points
    g.setColour(juce::Colour(PluginColors::pluginBorder).withAlpha(0.6f));
    for (int i = 0; i < numGridFreqs; ++i)
    {
        float x = freqToX(gridFreqs[i], aX, aW);
        g.drawVerticalLine(static_cast<int>(x), aY, aY + aH);
    }

    // Band rectangles
    for (int i = 0; i < static_cast<int>(bands.size()); ++i)
    {
        const auto& band = bands[i];
        float x1 = freqToX(band.freqLow,  aX, aW);
        float x2 = freqToX(band.freqHigh, aX, aW);
        juce::Rectangle<float> bandRect(x1, aY, x2 - x1, aH);

        // Band fill
        g.setColour(band.color.withAlpha(band.enabled ? 0.18f : 0.06f));
        g.fillRect(bandRect);

        // Selected band: bright border
        if (i == selectedBand)
        {
            g.setColour(band.color.withAlpha(0.9f));
            g.drawRect(bandRect, 1.5f);
        }
        else
        {
            g.setColour(band.color.withAlpha(0.35f));
            g.drawRect(bandRect, 0.75f);
        }

        // Horizontal threshold line
        float threshY = aY + aH - (band.threshold * aH);
        g.setColour(band.color.withAlpha(band.enabled ? 0.8f : 0.3f));
        g.drawLine(x1, threshY, x2, threshY, 1.5f);
    }
}

void FrequencyBandEditor::drawRuler(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(PluginColors::surface());
    g.fillRect(area);

    float aX = area.getX(), aW = area.getWidth();

    g.setFont(PluginFonts::mono(9.0f));
    g.setColour(PluginColors::muted());

    for (int i = 0; i < numGridFreqs; ++i)
    {
        float x = freqToX(gridFreqs[i], aX, aW);
        g.drawText(gridLabels[i],
                   static_cast<int>(x) - 14, static_cast<int>(area.getY()),
                   28, static_cast<int>(area.getHeight()),
                   juce::Justification::centred, false);
    }
}

void FrequencyBandEditor::drawBandList(juce::Graphics& g)
{
    auto area = bandListRect.toFloat();
    // Reserve bottom for add button
    area.removeFromBottom(26.0f);

    float rowH = 20.0f;
    float y = area.getY();

    g.setFont(PluginFonts::mono(10.0f));

    for (int i = 0; i < static_cast<int>(bands.size()); ++i)
    {
        const auto& band = bands[i];
        juce::Rectangle<float> rowRect(area.getX(), y, area.getWidth(), rowH);

        // Row background
        if (i == selectedBand)
            g.setColour(PluginColors::surfaceHi());
        else
            g.setColour(i % 2 == 0 ? PluginColors::surface() : PluginColors::bg());
        g.fillRect(rowRect);

        // Enable toggle indicator
        juce::Rectangle<float> toggleRect(rowRect.getX() + 4, rowRect.getCentreY() - 5, 10, 10);
        g.setColour(band.enabled ? band.color : PluginColors::muted());
        g.fillRoundedRectangle(toggleRect, 2.0f);

        // Color swatch
        juce::Rectangle<float> swatchRect(rowRect.getX() + 18, rowRect.getCentreY() - 5, 10, 10);
        g.setColour(band.color);
        g.fillRect(swatchRect);

        // Freq label
        auto labelStr = juce::String(static_cast<int>(band.freqLow)) + " – "
                      + juce::String(static_cast<int>(band.freqHigh)) + " Hz";
        g.setColour(PluginColors::text());
        g.drawText(labelStr, static_cast<int>(rowRect.getX() + 32), static_cast<int>(y),
                   static_cast<int>(rowRect.getWidth() - 80), static_cast<int>(rowH),
                   juce::Justification::centredLeft, false);

        // Threshold %
        g.setFont(PluginFonts::mono(9.0f));
        g.setColour(PluginColors::muted());
        g.drawText(juce::String(static_cast<int>(band.threshold * 100)) + "%",
                   static_cast<int>(rowRect.getRight() - 50), static_cast<int>(y),
                   40, static_cast<int>(rowH),
                   juce::Justification::centredRight, false);

        // Delete X
        g.setColour(juce::Colour(PluginColors::meterRed).withAlpha(0.7f));
        g.drawText("x",
                   static_cast<int>(rowRect.getRight() - 14), static_cast<int>(y),
                   12, static_cast<int>(rowH),
                   juce::Justification::centred, false);

        y += rowH;
    }
}

void FrequencyBandEditor::mouseDown(const juce::MouseEvent& e)
{
    auto pos = e.getPosition();

    // Check display area for band selection
    if (displayRect.contains(pos))
    {
        float aX = displayRect.toFloat().getX();
        float aW = displayRect.toFloat().getWidth();

        int clicked = -1;
        for (int i = 0; i < static_cast<int>(bands.size()); ++i)
        {
            float x1 = freqToX(bands[i].freqLow,  aX, aW);
            float x2 = freqToX(bands[i].freqHigh, aX, aW);
            if (pos.x >= x1 && pos.x <= x2)
            {
                clicked = i;
                break;
            }
        }
        selectBand(clicked);
        return;
    }

    // Check band list for selection / delete
    if (bandListRect.contains(pos))
    {
        float y = bandListRect.toFloat().getY();
        float rowH = 20.0f;
        for (int i = 0; i < static_cast<int>(bands.size()); ++i)
        {
            juce::Rectangle<float> rowRect(bandListRect.toFloat().getX(), y,
                                           bandListRect.toFloat().getWidth(), rowH);
            if (rowRect.contains(pos.toFloat()))
            {
                // Delete X: last 14px
                if (pos.x >= rowRect.getRight() - 14)
                    deleteBand(i);
                else
                    selectBand(i);
                return;
            }
            y += rowH;
        }
    }
}
