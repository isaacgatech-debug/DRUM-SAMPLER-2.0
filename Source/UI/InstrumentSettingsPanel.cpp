#include "InstrumentSettingsPanel.h"

static constexpr int rowH = 24;

InstrumentSettingsPanel::InstrumentSettingsPanel()
{
    // Level slider
    levelSlider.setRange(0.0, 1.0, 0.01);
    levelSlider.setValue(0.0);
    levelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    levelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    levelSlider.setColour(juce::Slider::thumbColourId,      juce::Colour(PluginColors::accent));
    levelSlider.setColour(juce::Slider::trackColourId,      juce::Colour(PluginColors::accentDim));
    levelSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(PluginColors::pluginBg));
    addAndMakeVisible(levelSlider);

    levelReadout.setText("0.0", juce::dontSendNotification);
    levelReadout.setFont(PluginFonts::mono(10.0f));
    levelReadout.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    levelReadout.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(levelReadout);

    levelSlider.onValueChange = [this]
    {
        levelReadout.setText(juce::String(levelSlider.getValue(), 1), juce::dontSendNotification);
    };

    // Solo / Mute buttons
    auto styleBtn = [](juce::TextButton& btn, juce::Colour onCol)
    {
        btn.setClickingTogglesState(true);
        btn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
        btn.setColour(juce::TextButton::buttonOnColourId, onCol);
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textPrimary));
        btn.setColour(juce::TextButton::textColourOnId,  juce::Colours::black);
    };
    styleBtn(soloBtn, juce::Colour(PluginColors::soloActive));
    styleBtn(muteBtn, juce::Colour(PluginColors::muteActive));
    addAndMakeVisible(soloBtn);
    addAndMakeVisible(muteBtn);

    // Soft Hit Level slider
    softHitSlider.setRange(0.0, 1.0, 0.01);
    softHitSlider.setValue(0.0);
    softHitSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    softHitSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    softHitSlider.setColour(juce::Slider::thumbColourId,      juce::Colour(PluginColors::accent));
    softHitSlider.setColour(juce::Slider::trackColourId,      juce::Colour(PluginColors::accentDim));
    softHitSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(PluginColors::pluginBg));
    addAndMakeVisible(softHitSlider);

    softHitReadout.setText("0.0", juce::dontSendNotification);
    softHitReadout.setFont(PluginFonts::mono(10.0f));
    softHitReadout.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    softHitReadout.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(softHitReadout);

    softHitSlider.onValueChange = [this]
    {
        softHitReadout.setText(juce::String(softHitSlider.getValue(), 1), juce::dontSendNotification);
    };

    // MIDI Settings button
    midiSettingsBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    midiSettingsBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textPrimary));
    addAndMakeVisible(midiSettingsBtn);
}

void InstrumentSettingsPanel::paint(juce::Graphics& g)
{
    // Panel background + left border
    g.fillAll(juce::Colour(PluginColors::pluginPanel));
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawVerticalLine(0, 0, static_cast<float>(getHeight()));

    auto area = getLocalBounds().toFloat().reduced(4.0f, 0.0f);
    float y = 4.0f;
    float w = area.getWidth();
    float x = area.getX();

    auto nextRow = [&]() -> juce::Rectangle<float> {
        juce::Rectangle<float> r(x, y, w, rowH);
        y += rowH;
        return r;
    };

    // --- Level row (slider + readout handled by child components) ---
    g.setFont(PluginFonts::label(10.0f));
    g.setColour(juce::Colour(PluginColors::textPrimary));
    {
        auto r = juce::Rectangle<float>(x, y, w * 0.35f, rowH);
        g.drawText("Level", r, juce::Justification::centredLeft, false);
        y += rowH + 4;  // extra gap for slider row which has subcomponents
    }

    y += rowH; // slider row
    y += 4;

    // Reverse row
    reverseRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y),
                                       static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, rowH), "Reverse", reverseOn);
    g.setFont(PluginFonts::mono(9.0f));
    g.setColour(juce::Colour(PluginColors::textMuted));
    g.drawText("200ms", static_cast<int>(x + w - 50), static_cast<int>(y), 46, rowH,
               juce::Justification::centredRight, false);
    y += rowH;

    // MIDI Monitor row
    midiMonitorRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y),
                                           static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, rowH), "MIDI Monitor", midiMonitorOn);
    y += rowH;

    y += 4;

    // Voice & Layer collapsible header
    voiceLayerHeader = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y),
                                             static_cast<int>(w), rowH);
    drawSectionHeader(g, juce::Rectangle<float>(x, y, w, rowH), "Voice and Layer", !voiceLayerOpen);
    y += rowH;

    if (voiceLayerOpen)
    {
        g.setFont(PluginFonts::label(9.0f));
        g.setColour(juce::Colour(PluginColors::textMuted));
        // Hard / Gradient / Soft
        auto layerRow = juce::Rectangle<float>(x + 8, y, w - 8, rowH);
        float cw = (w - 8) / 3.0f;
        g.drawText("Hard " + juce::String(hardVal),   layerRow.removeFromLeft(cw), juce::Justification::centredLeft, false);
        g.drawText("Grad " + juce::String(gradVal),   layerRow.removeFromLeft(cw), juce::Justification::centredLeft, false);
        g.drawText("Soft " + juce::String(softVal),   layerRow,                    juce::Justification::centredLeft, false);
        y += rowH;

        g.setColour(juce::Colour(PluginColors::textMuted));
        g.drawText("Voice Limit  " + juce::String(voiceLimitVal),
                   static_cast<int>(x + 8), static_cast<int>(y), static_cast<int>(w - 8), rowH,
                   juce::Justification::centredLeft, false);
        y += rowH;
    }

    y += 4;

    // Soft Hit Level
    g.setFont(PluginFonts::label(10.0f));
    g.setColour(juce::Colour(PluginColors::textPrimary));
    g.drawText("Soft Hit Level", static_cast<int>(x), static_cast<int>(y),
               static_cast<int>(w * 0.6f), rowH, juce::Justification::centredLeft, false);
    y += rowH;
    y += rowH; // softHit slider row
    y += 4;

    // Toggle rows
    smoothingRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, rowH), "Smoothing", smoothingOn);
    y += rowH;

    hiHatRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, rowH), "Hi-Hat CC Edit", hiHatCCEditOn);
    y += rowH;

    levelEnvRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, rowH), "Level Envelope", levelEnvOn);
    y += rowH;

    velocityGateRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, rowH), "Velocity Gate", velocityGateOn);
    y += rowH;

    y += 4;

    // MIDI Mapping collapsible
    midiMappingHeader = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), rowH);
    drawSectionHeader(g, juce::Rectangle<float>(x, y, w, rowH), "MIDI Mapping", !midiMappingOpen);
    y += rowH;

    if (midiMappingOpen)
    {
        g.setFont(PluginFonts::mono(10.0f));
        g.setColour(juce::Colour(PluginColors::textPrimary));
        // Open row
        drawRow(g, juce::Rectangle<float>(x + 8, y, w - 8, rowH), "Open", "36");
        y += rowH;
        // Hit row
        drawRow(g, juce::Rectangle<float>(x + 8, y, w - 8, rowH), "Hit", "35");
        y += rowH;
    }
}

void InstrumentSettingsPanel::resized()
{
    auto area = getLocalBounds().reduced(4, 0);
    float y = 4.0f;

    // Level: label row + slider row
    y += rowH + 4;

    // Level slider + readout + S M buttons
    auto sliderRow = juce::Rectangle<int>(area.getX(), static_cast<int>(y),
                                          area.getWidth(), rowH);
    int btnW = 20;
    muteBtn.setBounds(sliderRow.removeFromRight(btnW + 2).reduced(1));
    soloBtn.setBounds(sliderRow.removeFromRight(btnW + 2).reduced(1));
    levelReadout.setBounds(sliderRow.removeFromRight(36));
    levelSlider.setBounds(sliderRow);
    y += rowH + 4;

    // Skip reverse, midiMonitor rows (painted)
    y += rowH * 2 + 4;

    // Skip voiceLayerHeader (painted)
    y += rowH;
    if (voiceLayerOpen)
        y += rowH * 2;

    y += 4 + rowH; // Soft Hit Level label

    // Soft Hit slider
    auto softRow = juce::Rectangle<int>(area.getX(), static_cast<int>(y), area.getWidth(), rowH);
    softHitReadout.setBounds(softRow.removeFromRight(36));
    softHitSlider.setBounds(softRow);
    y += rowH + 4;

    // Skip toggle rows (painted)
    y += rowH * 4 + 4;
    // Skip MIDI Mapping header + rows (painted)
    y += rowH;
    if (midiMappingOpen)
        y += rowH * 2;

    // MIDI button at bottom
    int remainingH = getHeight() - static_cast<int>(y) - 4;
    if (remainingH > 0)
    {
        midiSettingsBtn.setBounds(area.getX(), static_cast<int>(y),
                                  area.getWidth(), juce::jmin(28, remainingH));
    }
}

void InstrumentSettingsPanel::mouseDown(const juce::MouseEvent& e)
{
    auto pt = e.getPosition();

    if (reverseRow.contains(pt))      { reverseOn      = !reverseOn;      repaint(); return; }
    if (midiMonitorRow.contains(pt))  { midiMonitorOn  = !midiMonitorOn;  repaint(); return; }
    if (smoothingRow.contains(pt))    { smoothingOn     = !smoothingOn;    repaint(); return; }
    if (hiHatRow.contains(pt))        { hiHatCCEditOn  = !hiHatCCEditOn;  repaint(); return; }
    if (levelEnvRow.contains(pt))     { levelEnvOn     = !levelEnvOn;     repaint(); return; }
    if (velocityGateRow.contains(pt)) { velocityGateOn = !velocityGateOn; repaint(); return; }
    if (voiceLayerHeader.contains(pt)) { voiceLayerOpen = !voiceLayerOpen; resized(); repaint(); return; }
    if (midiMappingHeader.contains(pt)){ midiMappingOpen= !midiMappingOpen; resized(); repaint(); return; }
}

void InstrumentSettingsPanel::drawSectionHeader(juce::Graphics& g, juce::Rectangle<float> area,
                                                  const juce::String& label, bool collapsed)
{
    g.setColour(juce::Colour(PluginColors::pluginSurface));
    g.fillRect(area);
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawHorizontalLine(static_cast<int>(area.getY()), area.getX(), area.getRight());
    g.drawHorizontalLine(static_cast<int>(area.getBottom()) - 1, area.getX(), area.getRight());

    g.setFont(PluginFonts::label(10.0f));
    g.setColour(juce::Colour(PluginColors::accent));
    g.drawText((collapsed ? "▶ " : "▼ ") + label, area.reduced(6.0f, 0.0f),
               juce::Justification::centredLeft, false);
}

void InstrumentSettingsPanel::drawRow(juce::Graphics& g, juce::Rectangle<float> area,
                                       const juce::String& label, const juce::String& valueStr)
{
    g.setColour(juce::Colour(PluginColors::textPrimary));
    g.setFont(PluginFonts::label(10.0f));
    g.drawText(label, area.reduced(2.0f, 0.0f), juce::Justification::centredLeft, false);

    g.setFont(PluginFonts::mono(10.0f));
    g.setColour(juce::Colour(PluginColors::accent));
    g.drawText(valueStr, area.reduced(2.0f, 0.0f), juce::Justification::centredRight, false);

    // Edit icon placeholder
    g.setColour(juce::Colour(PluginColors::textMuted));
    g.drawText("=", static_cast<int>(area.getRight() - 14), static_cast<int>(area.getY()),
               12, static_cast<int>(area.getHeight()), juce::Justification::centred, false);
}

void InstrumentSettingsPanel::drawToggleRow(juce::Graphics& g, juce::Rectangle<float> area,
                                             const juce::String& label, bool state)
{
    // Subtle alternating bg
    g.setColour(juce::Colour(PluginColors::pluginBg).withAlpha(0.5f));
    g.fillRect(area);

    g.setFont(PluginFonts::label(10.0f));
    g.setColour(juce::Colour(PluginColors::textPrimary));
    g.drawText(label, area.reduced(4.0f, 0.0f), juce::Justification::centredLeft, false);

    // Toggle pill
    juce::Rectangle<float> pill(area.getRight() - 30.0f, area.getCentreY() - 7.0f, 26.0f, 14.0f);
    g.setColour(state ? juce::Colour(PluginColors::accent) : juce::Colour(PluginColors::pluginBorder));
    g.fillRoundedRectangle(pill, 7.0f);

    // Thumb
    float thumbX = state ? pill.getRight() - 12.0f : pill.getX() + 2.0f;
    g.setColour(state ? juce::Colours::black : juce::Colour(PluginColors::textMuted));
    g.fillEllipse(thumbX, pill.getY() + 2.0f, 10.0f, 10.0f);

    // Label: On/Off
    g.setFont(PluginFonts::mono(8.0f));
    g.setColour(state ? juce::Colour(PluginColors::textMuted) : juce::Colour(PluginColors::textDim));
    g.drawText(state ? "On" : "Off", pill.reduced(1.0f), juce::Justification::centred, false);
}
