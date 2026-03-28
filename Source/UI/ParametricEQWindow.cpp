#include "ParametricEQWindow.h"
#include "ThemeManager.h"

//==============================================================================
// Band colour palette
//==============================================================================
const juce::Colour ParametricEQEditor::palette[] =
{
    juce::Colour(0xFF00C8FF),  // 0  cyan
    juce::Colour(0xFFF97316),  // 1  orange
    juce::Colour(0xFFA855F7),  // 2  purple
    juce::Colour(0xFF22C55E),  // 3  green
    juce::Colour(0xFFEC4899),  // 4  pink
    juce::Colour(0xFFEAB308),  // 5  yellow
    juce::Colour(0xFF3B82F6),  // 6  blue
    juce::Colour(0xFFEF4444),  // 7  red
};
const int ParametricEQEditor::paletteSize = 8;

//==============================================================================
// Log-scale helpers
//==============================================================================
float ParametricEQEditor::freqToX(float freq, float w)
{
    float logMin = std::log10(minFreq);
    float logMax = std::log10(maxFreq);
    return (std::log10(juce::jlimit(minFreq, maxFreq, freq)) - logMin) / (logMax - logMin) * w;
}

float ParametricEQEditor::xToFreq(float x, float w)
{
    float logMin = std::log10(minFreq);
    float logMax = std::log10(maxFreq);
    float t = juce::jlimit(0.0f, 1.0f, x / w);
    return std::pow(10.0f, logMin + t * (logMax - logMin));
}

float ParametricEQEditor::gainToY(float gain, float h)
{
    return h * 0.5f - (gain / maxGain) * (h * 0.5f - 8.0f);
}

float ParametricEQEditor::yToGain(float y, float h)
{
    return (h * 0.5f - y) / (h * 0.5f - 8.0f) * maxGain;
}

//==============================================================================
// Biquad magnitude computation
//==============================================================================
float ParametricEQEditor::computeMagnitudeDB(float freq, const Band& b)
{
    if (!b.enabled) return 0.0f;

    const float sampleRate = 44100.0f;
    const float pi = juce::MathConstants<float>::pi;
    const float omega = 2.0f * pi * freq / sampleRate;
    const float sinW  = std::sin(omega);
    const float cosW  = std::cos(omega);
    const float A     = std::pow(10.0f, b.gain / 40.0f);
    const float sqA   = std::sqrt(A);
    const float alpha_q = sinW / (2.0f * b.q);

    float b0 = 1, b1 = 0, b2 = 0, a0 = 1, a1 = 0, a2 = 0;

    switch (b.type)
    {
        case Band::Peak:
            b0 = 1.0f + alpha_q * A;
            b1 = -2.0f * cosW;
            b2 = 1.0f - alpha_q * A;
            a0 = 1.0f + alpha_q / A;
            a1 = -2.0f * cosW;
            a2 = 1.0f - alpha_q / A;
            break;

        case Band::LowShelf:
        {
            float alpha = sinW / 2.0f * std::sqrt((A + 1.0f / A) * (1.0f / 0.707f - 1.0f) + 2.0f);
            b0 = A * ((A+1) - (A-1)*cosW + 2.0f*sqA*alpha);
            b1 = 2.0f*A*((A-1) - (A+1)*cosW);
            b2 = A * ((A+1) - (A-1)*cosW - 2.0f*sqA*alpha);
            a0 = (A+1) + (A-1)*cosW + 2.0f*sqA*alpha;
            a1 = -2.0f*((A-1) + (A+1)*cosW);
            a2 = (A+1) + (A-1)*cosW - 2.0f*sqA*alpha;
            break;
        }
        case Band::HighShelf:
        {
            float alpha = sinW / 2.0f * std::sqrt((A + 1.0f / A) * (1.0f / 0.707f - 1.0f) + 2.0f);
            b0 = A * ((A+1) + (A-1)*cosW + 2.0f*sqA*alpha);
            b1 = -2.0f*A*((A-1) + (A+1)*cosW);
            b2 = A * ((A+1) + (A-1)*cosW - 2.0f*sqA*alpha);
            a0 = (A+1) - (A-1)*cosW + 2.0f*sqA*alpha;
            a1 = 2.0f*((A-1) - (A+1)*cosW);
            a2 = (A+1) - (A-1)*cosW - 2.0f*sqA*alpha;
            break;
        }
        case Band::LowPass:
            b0 = (1.0f - cosW) / 2.0f;
            b1 = 1.0f - cosW;
            b2 = (1.0f - cosW) / 2.0f;
            a0 = 1.0f + alpha_q;
            a1 = -2.0f * cosW;
            a2 = 1.0f - alpha_q;
            break;

        case Band::HighPass:
            b0 = (1.0f + cosW) / 2.0f;
            b1 = -(1.0f + cosW);
            b2 = (1.0f + cosW) / 2.0f;
            a0 = 1.0f + alpha_q;
            a1 = -2.0f * cosW;
            a2 = 1.0f - alpha_q;
            break;

        default: return 0.0f;
    }

    // Normalise by a0
    b0 /= a0; b1 /= a0; b2 /= a0;
    a1 /= a0; a2 /= a0;

    // |H(e^jω)|
    float nr = b0 + b1*cosW + b2*std::cos(2*omega);
    float ni = -b1*sinW - b2*std::sin(2*omega);
    float dr = 1.0f + a1*cosW + a2*std::cos(2*omega);
    float di = -a1*sinW - a2*std::sin(2*omega);

    float nMag2 = nr*nr + ni*ni;
    float dMag2 = dr*dr + di*di;
    float mag   = (dMag2 > 1e-10f) ? std::sqrt(nMag2 / dMag2) : 1.0f;
    return 20.0f * std::log10(juce::jlimit(1e-5f, 100.0f, mag));
}

//==============================================================================
// ParametricEQEditor
//==============================================================================
ParametricEQEditor::ParametricEQEditor()
{
    addBand(80.0f,    0.0f);   // band 0 – Low Shelf
    addBand(250.0f,   0.0f);   // band 1 – Peak
    addBand(1000.0f,  0.0f);   // band 2 – Peak (mid)
    addBand(4000.0f,  0.0f);   // band 3 – Peak (high mid)
    addBand(16000.0f, 0.0f);   // band 4 – High Shelf

    bands[0].type = Band::LowShelf;
    bands[4].type = Band::HighShelf;

    setMouseCursor(juce::MouseCursor::CrosshairCursor);
}

void ParametricEQEditor::addBand(float freq, float gain)
{
    Band b;
    b.freq   = freq;
    b.gain   = gain;
    b.color  = palette[bands.size() % paletteSize];
    b.q      = 0.707f;
    bands.push_back(b);
}

juce::Rectangle<float> ParametricEQEditor::displayArea() const
{
    return getLocalBounds().toFloat().reduced(0).withTrimmedBottom(72.0f);
}

//==============================================================================
void ParametricEQEditor::paint(juce::Graphics& g)
{
    auto& tm = ThemeManager::get();
    auto  da = displayArea();

    g.setColour(tm.surface());
    g.fillRect(getLocalBounds());

    drawGrid(g, da);
    drawSpectrumFill(g, da);
    drawResponseCurve(g, da);
    drawNodes(g, da);

    // Bottom bar
    auto bar = getLocalBounds().toFloat().removeFromBottom(72.0f);
    g.setColour(tm.panel());
    g.fillRect(bar);
    g.setColour(tm.border());
    g.drawHorizontalLine(static_cast<int>(bar.getY()), bar.getX(), bar.getRight());

    drawBandInfoBar(g, bar);
}

// Overloaded paint helper — takes Graphics as first param
void ParametricEQEditor::drawGrid(juce::Graphics& g, juce::Rectangle<float> area)
{
    auto& tm = ThemeManager::get();
    g.setColour(tm.bg());
    g.fillRect(area);

    // Frequency grid lines
    const float freqs[] = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    const char* labels[] = {"20","50","100","200","500","1k","2k","5k","10k","20k"};
    int n = static_cast<int>(sizeof(freqs)/sizeof(freqs[0]));

    g.setFont(PluginFonts::mono(9.0f));

    for (int i = 0; i < n; ++i)
    {
        float x = area.getX() + freqToX(freqs[i], area.getWidth());
        g.setColour(juce::Colour(PluginColors::pluginBorder).withAlpha(0.6f));
        g.drawVerticalLine(static_cast<int>(x), area.getY(), area.getBottom() - 1);

        g.setColour(tm.muted());
        g.drawText(labels[i], static_cast<int>(x) - 12, static_cast<int>(area.getBottom()) - 14, 24, 12,
                   juce::Justification::centred, false);
    }

    // Gain grid lines
    const float gains[] = {-24,-18,-12,-6,0,6,12,18,24};
    for (float gain : gains)
    {
        float y = area.getY() + gainToY(gain, area.getHeight());
        bool isZero = (gain == 0.0f);
        g.setColour(isZero ? juce::Colour(PluginColors::pluginBorderHi)
                           : juce::Colour(PluginColors::pluginBorder).withAlpha(0.4f));
        g.drawHorizontalLine(static_cast<int>(y), area.getX(), area.getRight());

        if (gain != 0.0f)
        {
            juce::String label = (gain > 0 ? "+" : "") + juce::String((int)gain);
            g.setColour(tm.muted());
            g.setFont(PluginFonts::mono(8.0f));
            g.drawText(label, static_cast<int>(area.getX()), static_cast<int>(y) - 6, 28, 12,
                       juce::Justification::centredLeft, false);
        }
    }
}


void ParametricEQEditor::drawSpectrumFill(juce::Graphics& g, juce::Rectangle<float> area)
{
    // Subtle background spectrum fill (decorative gradient)
    juce::ColourGradient grad(juce::Colour(PluginColors::accent).withAlpha(0.04f),
                               area.getX(), area.getBottom(),
                               juce::Colours::transparentBlack,
                               area.getX(), area.getY(), false);
    g.setGradientFill(grad);
    g.fillRect(area);
}

void ParametricEQEditor::drawResponseCurve(juce::Graphics& g, juce::Rectangle<float> area)
{
    const int steps = static_cast<int>(area.getWidth());
    if (steps <= 0) return;

    // Compute summed response
    juce::Path curve;
    juce::Path fill;
    float zeroY = area.getY() + gainToY(0.0f, area.getHeight());

    for (int px = 0; px <= steps; ++px)
    {
        float freq = xToFreq(static_cast<float>(px), area.getWidth());
        float totalDB = 0.0f;
        for (const auto& b : bands) totalDB += computeMagnitudeDB(freq, b);

        float y = area.getY() + gainToY(totalDB, area.getHeight());
        y = juce::jlimit(area.getY(), area.getBottom(), y);
        float x = area.getX() + static_cast<float>(px);

        if (px == 0) { curve.startNewSubPath(x, y); fill.startNewSubPath(x, y); }
        else         { curve.lineTo(x, y);           fill.lineTo(x, y); }
    }

    // Fill under curve
    fill.lineTo(area.getRight(), zeroY);
    fill.lineTo(area.getX(), zeroY);
    fill.closeSubPath();

    juce::ColourGradient fillGrad(juce::Colour(PluginColors::accent).withAlpha(0.15f),
                                   area.getX(), area.getY(),
                                   juce::Colour(PluginColors::accent).withAlpha(0.02f),
                                   area.getX(), area.getBottom(), false);
    g.setGradientFill(fillGrad);
    g.fillPath(fill);

    // Curve stroke
    g.setColour(juce::Colour(PluginColors::accent));
    g.strokePath(curve, juce::PathStrokeType(2.0f));
}

void ParametricEQEditor::drawNodes(juce::Graphics& g, juce::Rectangle<float> area)
{
    for (int i = 0; i < static_cast<int>(bands.size()); ++i)
    {
        const auto& b = bands[i];
        float x = area.getX() + freqToX(b.freq, area.getWidth());
        float y = area.getY() + gainToY(b.gain, area.getHeight());
        y = juce::jlimit(area.getY() + nodeRadius, area.getBottom() - nodeRadius, y);

        bool sel = (i == selectedBand);
        bool hov = (i == hovBand);

        // Glow
        if (sel || hov)
        {
            g.setColour(b.color.withAlpha(0.2f));
            g.fillEllipse(x - nodeRadius * 2, y - nodeRadius * 2,
                          nodeRadius * 4, nodeRadius * 4);
        }

        // Node circle
        g.setColour(sel ? b.color : b.color.withAlpha(0.8f));
        g.fillEllipse(x - nodeRadius, y - nodeRadius, nodeRadius * 2, nodeRadius * 2);
        g.setColour(juce::Colours::white.withAlpha(sel ? 1.0f : 0.6f));
        g.drawEllipse(x - nodeRadius, y - nodeRadius, nodeRadius * 2, nodeRadius * 2,
                      sel ? 2.0f : 1.0f);

        // Band number
        g.setFont(PluginFonts::mono(8.0f));
        g.setColour(juce::Colours::white);
        g.drawText(juce::String(i + 1),
                   static_cast<int>(x - nodeRadius), static_cast<int>(y - nodeRadius),
                   static_cast<int>(nodeRadius * 2), static_cast<int>(nodeRadius * 2),
                   juce::Justification::centred, false);
    }
}

void ParametricEQEditor::drawBandInfoBar(juce::Graphics& g, juce::Rectangle<float> area)
{
    auto& tm = ThemeManager::get();

    if (selectedBand >= 0 && selectedBand < static_cast<int>(bands.size()))
    {
        const auto& b = bands[selectedBand];

        // Color tab on left
        g.setColour(b.color);
        g.fillRect(area.removeFromLeft(4.0f));

        // Band info
        area = area.reduced(12.0f, 8.0f);

        g.setFont(PluginFonts::mono(11.0f));
        g.setColour(tm.text());

        juce::String freqStr = b.freq >= 1000.0f
            ? juce::String(b.freq / 1000.0f, 2) + " kHz"
            : juce::String(static_cast<int>(b.freq)) + " Hz";

        juce::String gainStr = (b.gain >= 0 ? "+" : "") + juce::String(b.gain, 1) + " dB";
        juce::String qStr    = "Q " + juce::String(b.q, 2);

        float colW = area.getWidth() / 5.0f;

        // Type buttons
        const char* typeNames[] = {"Peak","Lo Shelf","Hi Shelf","Lo Pass","Hi Pass"};
        for (int t = 0; t < 5; ++t)
        {
            auto btn = area.removeFromLeft(colW).reduced(2.0f, 2.0f);
            bool active = (b.type == static_cast<Band::Type>(t));
            g.setColour(active ? b.color : tm.surfaceHi());
            g.fillRoundedRectangle(btn, 3.0f);
            g.setColour(active ? juce::Colours::black : tm.muted());
            g.setFont(PluginFonts::label(8.5f));
            g.drawText(typeNames[t], btn, juce::Justification::centred, false);

            typeBtnRect[t] = btn.toNearestInt();
        }

        // Freq, gain, Q labels
        g.setFont(PluginFonts::mono(12.0f));
        g.setColour(b.color);
        auto valArea = getLocalBounds().toFloat().removeFromBottom(32.0f).reduced(12.0f, 2.0f);
        g.drawText("FREQ: " + freqStr + "   GAIN: " + gainStr + "   " + qStr
                   + "   TYPE: " + b.typeName(),
                   valArea, juce::Justification::centredLeft, false);
    }
    else
    {
        g.setFont(PluginFonts::label(9.0f));
        g.setColour(tm.muted());
        g.drawText("Click a band node to select. Drag to adjust frequency / gain. Scroll = Q.",
                   area.reduced(12.0f, 0.0f), juce::Justification::centredLeft, false);
    }
}

void ParametricEQEditor::resized() {}

int ParametricEQEditor::hitTestBand(juce::Point<float> pt) const
{
    auto da = displayArea();
    for (int i = 0; i < static_cast<int>(bands.size()); ++i)
    {
        const auto& b = bands[i];
        float x = da.getX() + freqToX(b.freq, da.getWidth());
        float y = da.getY() + gainToY(b.gain, da.getHeight());
        y = juce::jlimit(da.getY() + nodeRadius, da.getBottom() - nodeRadius, y);
        float dist = pt.getDistanceFrom({x, y});
        if (dist <= nodeRadius + 4.0f) return i;
    }
    return -1;
}

void ParametricEQEditor::mouseDown(const juce::MouseEvent& e)
{
    auto pt = e.position;

    // Check type buttons
    if (selectedBand >= 0)
    {
        for (int t = 0; t < 5; ++t)
        {
            if (typeBtnRect[t].contains(e.getPosition()))
            {
                bands[selectedBand].type = static_cast<Band::Type>(t);
                repaint();
                return;
            }
        }
    }

    int hit = hitTestBand(pt);
    if (hit >= 0)
    {
        dragBand  = hit;
        selectedBand = hit;
        dragStart = pt;
        dragStartFreq = bands[hit].freq;
        dragStartGain = bands[hit].gain;
    }
    else
    {
        // Deselect
        selectedBand = -1;
    }
    repaint();
}

void ParametricEQEditor::mouseDrag(const juce::MouseEvent& e)
{
    if (dragBand < 0) return;
    auto da = displayArea();
    float dx = e.position.x - dragStart.x;
    float dy = e.position.y - dragStart.y;

    float newFreq = dragStartFreq * std::pow(2.0f, dx / 120.0f);
    newFreq = juce::jlimit(minFreq, maxFreq, newFreq);

    float newGain = juce::jlimit(-maxGain, maxGain,
                                 dragStartGain - dy * (maxGain * 2.0f / da.getHeight()));

    bands[dragBand].freq = newFreq;
    bands[dragBand].gain = newGain;
    repaint();
}

void ParametricEQEditor::mouseUp(const juce::MouseEvent&)
{
    dragBand = -1;
}

void ParametricEQEditor::mouseMove(const juce::MouseEvent& e)
{
    int prev = hovBand;
    hovBand = hitTestBand(e.position);
    if (hovBand != prev) repaint();
}

void ParametricEQEditor::mouseDoubleClick(const juce::MouseEvent& e)
{
    auto da = displayArea();
    int hit = hitTestBand(e.position);
    if (hit < 0 && da.contains(e.position))
    {
        // Add new band at click position
        float freq = xToFreq(e.position.x - da.getX(), da.getWidth());
        float gain = yToGain(e.position.y - da.getY(), da.getHeight());
        addBand(freq, gain);
        selectedBand = static_cast<int>(bands.size()) - 1;
        repaint();
    }
}

void ParametricEQEditor::mouseWheelMove(const juce::MouseEvent&,
                                         const juce::MouseWheelDetails& w)
{
    if (selectedBand < 0) return;
    bands[selectedBand].q = juce::jlimit(0.1f, 12.0f,
                                          bands[selectedBand].q + w.deltaY * 0.5f);
    repaint();
}

//==============================================================================
// ParametricEQWindow
//==============================================================================
ParametricEQWindow::ParametricEQWindow(const juce::String& channelName)
    : juce::DocumentWindow("EQ — " + channelName,
                           juce::Colour(PluginColors::pluginPanel),
                           juce::DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(false);
    setContentNonOwned(&eqEditor, true);
    setResizable(true, false);
    setSize(800, 480);
    centreWithSize(800, 480);
}
