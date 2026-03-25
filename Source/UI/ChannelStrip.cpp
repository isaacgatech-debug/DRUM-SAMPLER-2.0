#include "ChannelStrip.h"
#include "../Core/PluginProcessor.h"
#include "../Mixer/MixerChannel.h"

ChannelStrip::ChannelStrip(int channelIndex, const juce::String& channelName)
    : index(channelIndex), name(channelName)
{
    // Channel color from design token
    channelColor = (channelIndex >= 0 && channelIndex < 12)
                 ? PluginColors::channelColors[channelIndex]
                 : PluginColors::masterColor;

    // Short name (first word / abbreviation)
    shortName = name.upToFirstOccurrenceOf(" ", false, false);
    if (shortName.isEmpty()) shortName = name.substring(0, 4).toUpperCase();

    // FX Send knobs
    fx1Knob.setRange(0.0, 1.0);
    fx1Knob.setValue(0.0);
    fx1Knob.setKnobColor(juce::Colour(PluginColors::accent));
    fx1Knob.setLabel("FX1");
    addAndMakeVisible(fx1Knob);

    fx2Knob.setRange(0.0, 1.0);
    fx2Knob.setValue(0.0);
    fx2Knob.setKnobColor(juce::Colour(PluginColors::accentDim));
    fx2Knob.setLabel("FX2");
    addAndMakeVisible(fx2Knob);

    // Solo button
    soloBtn.setClickingTogglesState(true);
    soloBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    soloBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(PluginColors::soloActive));
    soloBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textPrimary));
    soloBtn.setColour(juce::TextButton::textColourOnId,  juce::Colours::black);
    soloBtn.addListener(this);
    addAndMakeVisible(soloBtn);

    // Mute button
    muteBtn.setClickingTogglesState(true);
    muteBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    muteBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(PluginColors::muteActive));
    muteBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textPrimary));
    muteBtn.setColour(juce::TextButton::textColourOnId,  juce::Colours::black);
    muteBtn.addListener(this);
    addAndMakeVisible(muteBtn);

    // VU meter
    addAndMakeVisible(vuMeter);

    // Fader
    fader.setSliderStyle(juce::Slider::LinearVertical);
    fader.setRange(0.0, 1.0, 0.001);
    fader.setValue(0.8);
    fader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    fader.setColour(juce::Slider::thumbColourId,      juce::Colour(PluginColors::textPrimary));
    fader.setColour(juce::Slider::trackColourId,      juce::Colour(PluginColors::pluginBorder));
    fader.setColour(juce::Slider::backgroundColourId, juce::Colour(PluginColors::pluginBg));
    fader.onValueChange = [this]
    {
        // Convert 0-1 to dB: -inf to +6
        double v = fader.getValue();
        double db = (v > 0.0) ? (20.0 * std::log10(v)) : -100.0;
        juce::String label = (db <= -99.0) ? "-inf" : juce::String(db, 1);
        dbReadout.setText(label, juce::dontSendNotification);
        if (onParameterChanged)
            onParameterChanged(index, "Level", static_cast<float>(v));
    };
    addAndMakeVisible(fader);

    // dB readout
    dbReadout.setText("-2.0", juce::dontSendNotification);
    dbReadout.setFont(PluginFonts::mono(10.0f));
    dbReadout.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    dbReadout.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dbReadout);

    // Pan slider
    panSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    panSlider.setRange(-1.0, 1.0, 0.01);
    panSlider.setValue(0.0);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    panSlider.setColour(juce::Slider::thumbColourId,      channelColor);
    panSlider.setColour(juce::Slider::trackColourId,      juce::Colour(PluginColors::pluginBorder));
    panSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(PluginColors::pluginBg));
    panSlider.onValueChange = [this]
    {
        if (onParameterChanged)
            onParameterChanged(index, "Pan", static_cast<float>(panSlider.getValue()));
    };
    addAndMakeVisible(panSlider);
}

void ChannelStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Strip background
    g.setColour(juce::Colour(PluginColors::pluginSurface));
    g.fillRect(bounds);

    // Right border (1px separator)
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawVerticalLine(static_cast<int>(bounds.getRight()) - 1, bounds.getY(), bounds.getBottom());

    auto b = bounds;

    // 3px top color bar + soft glow
    drawColorBar(g, b.removeFromTop(3.0f));

    // Header: channel number + name
    drawHeader(g, b.removeFromTop(36.0f));

    // Mini EQ curve (56x24px)
    auto eqArea = b.removeFromTop(30.0f).reduced(4.0f, 3.0f);
    drawMiniEQ(g, eqArea);

    // Section label: SENDS (painted area above knobs — handled in resized layout)
    // Labels for bottom sections are drawn after child components paint
}

void ChannelStrip::drawColorBar(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(channelColor);
    g.fillRect(bounds);

    // Subtle glow below color bar
    juce::ColourGradient glow(channelColor.withAlpha(0.3f), bounds.getX(), bounds.getBottom(),
                               channelColor.withAlpha(0.0f), bounds.getX(), bounds.getBottom() + 8.0f,
                               false);
    g.setGradientFill(glow);
    g.fillRect(bounds.getX(), bounds.getBottom(), bounds.getWidth(), 8.0f);
}

void ChannelStrip::drawHeader(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Dark header bg
    g.setColour(juce::Colour(PluginColors::pluginPanel));
    g.fillRect(bounds);

    // Channel number (top-left, dim)
    g.setFont(PluginFonts::mono(8.0f));
    g.setColour(juce::Colour(PluginColors::textMuted));
    g.drawText(juce::String(index + 1), bounds.reduced(3.0f, 2.0f).removeFromTop(10.0f),
               juce::Justification::topLeft, false);

    // Channel name (centred)
    g.setFont(PluginFonts::label(9.0f));
    g.setColour(juce::Colour(PluginColors::textPrimary));
    g.drawText(shortName, bounds, juce::Justification::centred, false);

    // Full name smaller below short name
    g.setFont(PluginFonts::mono(7.5f));
    g.setColour(juce::Colour(PluginColors::textMuted));
    auto smallNameBounds = bounds.removeFromBottom(12.0f).reduced(2.0f, 0.0f);
    g.drawText(name, smallNameBounds, juce::Justification::centred, true);
}

void ChannelStrip::drawMiniEQ(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // EQ display background
    g.setColour(juce::Colour(PluginColors::pluginBg));
    g.fillRect(bounds);
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawRect(bounds, 0.75f);

    // Draw a simple 4-band EQ polyline (decorative)
    juce::Path eqCurve;
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float cy = bounds.getCentreY();
    float amp = h * 0.3f;

    // Use channel index to create a unique-looking curve per channel
    float phase = static_cast<float>(index) * 0.4f;
    bool first = true;
    int steps = static_cast<int>(w);

    for (int px = 0; px < steps; px += 2)
    {
        float nx = static_cast<float>(px) / w;
        float ny = cy - amp * (0.5f * std::sin(nx * 8.0f + phase)
                              + 0.3f * std::sin(nx * 20.0f + phase * 1.3f)
                              + 0.2f * std::sin(nx * 40.0f));
        ny = juce::jlimit(bounds.getY() + 1.0f, bounds.getBottom() - 1.0f, ny);
        float sx = bounds.getX() + static_cast<float>(px);
        if (first) { eqCurve.startNewSubPath(sx, ny); first = false; }
        else        eqCurve.lineTo(sx, ny);
    }

    g.setColour(channelColor.withAlpha(0.8f));
    g.strokePath(eqCurve, juce::PathStrokeType(1.0f));

    // Zero-line
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawHorizontalLine(static_cast<int>(cy), bounds.getX() + 1, bounds.getRight() - 1);
}

void ChannelStrip::drawSectionLabel(juce::Graphics& g, juce::Rectangle<float> bounds,
                                     const juce::String& label)
{
    g.setFont(PluginFonts::mono(8.0f));
    g.setColour(juce::Colour(PluginColors::textDim));
    g.drawText(label, bounds, juce::Justification::centred, false);
}

void ChannelStrip::resized()
{
    auto area = getLocalBounds();

    // Color bar (3px) + header (36px) + mini EQ area (30px) = 69px consumed by paint
    area.removeFromTop(3 + 36 + 30);

    // FX Sends section label (10px)
    area.removeFromTop(10);

    // FX knobs row: two 20px knobs side by side
    auto knobRow = area.removeFromTop(28);
    int knobW = knobRow.getWidth() / 2;
    fx1Knob.setBounds(knobRow.removeFromLeft(knobW).reduced(4, 2));
    fx2Knob.setBounds(knobRow.reduced(4, 2));

    // S/M buttons row
    auto smRow = area.removeFromTop(22);
    int smW = smRow.getWidth() / 2;
    soloBtn.setBounds(smRow.removeFromLeft(smW).reduced(2));
    muteBtn.setBounds(smRow.reduced(2));

    // VU meter (use remaining height minus fader 80px, dB 14px, pan 18px, gaps)
    int vuHeight = area.getHeight() - 80 - 14 - 18 - 4;
    vuHeight = juce::jmax(40, vuHeight);
    vuMeter.setBounds(area.removeFromTop(vuHeight).reduced(8, 2));

    // Vertical fader
    fader.setBounds(area.removeFromTop(80).reduced(16, 2));

    // dB readout
    dbReadout.setBounds(area.removeFromTop(14));

    // Pan slider
    panSlider.setBounds(area.removeFromTop(18).reduced(2, 2));
}

void ChannelStrip::setProcessor(DrumSampler2Processor* proc)
{
    processor = proc;
}

void ChannelStrip::updateMeter(float level, float peak)
{
    vuMeter.setLevel(level);
    vuMeter.setPeak(peak);
}

void ChannelStrip::buttonClicked(juce::Button* button)
{
    if (button == &soloBtn && onButtonChanged)
        onButtonChanged(index, "Solo", soloBtn.getToggleState());
    else if (button == &muteBtn && onButtonChanged)
        onButtonChanged(index, "Mute", muteBtn.getToggleState());
}
