#include "ChannelStrip.h"
#include "../Core/PluginProcessor.h"
#include "../Mixer/MixerChannel.h"
#include "ThemeManager.h"

ChannelStrip::ChannelStrip(int channelIndex, const juce::String& channelName)
    : index(channelIndex), name(channelName)
{
    channelColor = (channelIndex >= 0 && channelIndex < 12)
                 ? PluginColors::channelColors[channelIndex]
                 : PluginColors::masterColor;

    shortName = name.upToFirstOccurrenceOf(" ", false, false);
    if (shortName.isEmpty()) shortName = name.substring(0, 6).toUpperCase();

    // Pan knob
    panKnob.setRange(-1.0, 1.0);
    panKnob.setValue(0.0);
    panKnob.setKnobColor(channelColor);
    panKnob.setLabel("PAN");
    panKnob.onValueChange = [this]
    {
        if (onParameterChanged) onParameterChanged(index, "Pan", (float)panKnob.getValue());
    };
    addAndMakeVisible(panKnob);

    // Solo
    soloBtn.setClickingTogglesState(true);
    soloBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    soloBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(PluginColors::soloActive));
    soloBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textPrimary));
    soloBtn.setColour(juce::TextButton::textColourOnId,  juce::Colours::black);
    soloBtn.addListener(this);
    addAndMakeVisible(soloBtn);

    // Mute
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
        double v = fader.getValue();
        double db = (v > 0.0) ? (20.0 * std::log10(v)) : -100.0;
        juce::String label = (db <= -99.0) ? "-inf" : juce::String(db, 1);
        dbReadout.setText(label, juce::dontSendNotification);
        if (onParameterChanged) onParameterChanged(index, "Level", (float)v);
    };
    addAndMakeVisible(fader);

    // dB readout
    dbReadout.setText("-2.0", juce::dontSendNotification);
    dbReadout.setFont(PluginFonts::mono(9.0f));
    dbReadout.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    dbReadout.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dbReadout);
}

//==============================================================================
void ChannelStrip::paint(juce::Graphics& g)
{
    auto& tm = ThemeManager::get();
    auto  b  = getLocalBounds().toFloat();

    // Strip bg
    g.setColour(tm.surface());
    g.fillRect(b);

    // Right border
    g.setColour(tm.border());
    g.drawVerticalLine(getWidth() - 1, b.getY(), b.getBottom());

    auto row = b;

    // 3px color bar + glow
    drawColorBar(g, row.removeFromTop(3.0f));

    // 24px header
    drawHeader(g, row.removeFromTop(24.0f));

    // ── INSERTS ──
    drawSectionLabel(g, row.removeFromTop(13.0f), "INSERTS");
    for (int i = 0; i < 4; ++i)
    {
        auto slotR = row.removeFromTop(18.0f);
        bool isEQ = (i == 0);
        if (isEQ) eqSlotRect = slotR.toNearestInt();
        drawInsertSlot(g, slotR, i, isEQ);
    }
    row.removeFromTop(2.0f);

    // ── SENDS ──
    drawSectionLabel(g, row.removeFromTop(13.0f), "SENDS");
    for (int i = 0; i < 4; ++i)
        drawSendSlot(g, row.removeFromTop(16.0f), i);
    row.removeFromTop(2.0f);

    // ── I/O ──
    drawIORow(g, row.removeFromTop(18.0f), "IN", "Main In");
    drawIORow(g, row.removeFromTop(18.0f), "OUT", "Main Out");
    row.removeFromTop(2.0f);

    // ── GROUPS ──
    drawGroupRow(g, row.removeFromTop(20.0f));
    row.removeFromTop(2.0f);

    // Pan / fader / VU / mute-solo are child components — painted by JUCE
}

//==============================================================================
void ChannelStrip::drawColorBar(juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour(channelColor);
    g.fillRect(r);
    juce::ColourGradient glow(channelColor.withAlpha(0.35f), r.getX(), r.getBottom(),
                               channelColor.withAlpha(0.0f),  r.getX(), r.getBottom() + 8.0f, false);
    g.setGradientFill(glow);
    g.fillRect(r.getX(), r.getBottom(), r.getWidth(), 8.0f);
}

void ChannelStrip::drawHeader(juce::Graphics& g, juce::Rectangle<float> r)
{
    auto& tm = ThemeManager::get();
    g.setColour(tm.panel());
    g.fillRect(r);

    // Channel number (small, top-left)
    g.setFont(PluginFonts::mono(7.5f));
    g.setColour(tm.muted());
    g.drawText(juce::String(index + 1), r.reduced(3, 1).removeFromTop(10),
               juce::Justification::topLeft, false);

    // Channel name (centred)
    g.setFont(PluginFonts::label(9.0f));
    g.setColour(tm.text());
    g.drawText(shortName, r, juce::Justification::centred, false);
}

void ChannelStrip::drawSectionLabel(juce::Graphics& g, juce::Rectangle<float> r,
                                     const juce::String& label)
{
    auto& tm = ThemeManager::get();
    g.setColour(tm.panel().darker(0.05f));
    g.fillRect(r);
    g.setFont(PluginFonts::mono(7.5f));
    g.setColour(tm.dim());
    g.drawText(label, r.reduced(4.0f, 0.0f), juce::Justification::centredLeft, false);
    g.setColour(tm.border());
    g.drawHorizontalLine((int)r.getBottom() - 1, r.getX(), r.getRight());
}

void ChannelStrip::drawInsertSlot(juce::Graphics& g, juce::Rectangle<float> r,
                                   int idx, bool isEQ)
{
    auto& tm = ThemeManager::get();
    bool empty = (insertNames[idx] == "—");

    g.setColour(empty ? tm.bg() : tm.surfaceHi());
    g.fillRect(r.reduced(1.0f, 0.5f));

    g.setColour(tm.border().withAlpha(0.6f));
    g.drawRect(r.reduced(1.0f, 0.5f), 0.5f);

    if (isEQ)
    {
        g.setColour(juce::Colour(PluginColors::accent).withAlpha(0.15f));
        g.fillRect(r.reduced(1.0f, 0.5f));
    }

    g.setFont(PluginFonts::mono(8.5f));
    g.setColour(isEQ ? juce::Colour(PluginColors::accent) : tm.muted());
    g.drawText(insertNames[idx], r.reduced(4.0f, 0.0f),
               juce::Justification::centredLeft, false);

    if (isEQ)
    {
        g.setFont(PluginFonts::mono(7.0f));
        g.setColour(tm.muted());
        g.drawText("dbl-click", r, juce::Justification::centredRight, false);
    }
}

void ChannelStrip::drawSendSlot(juce::Graphics& g, juce::Rectangle<float> r, int idx)
{
    auto& tm = ThemeManager::get();
    g.setColour(tm.bg());
    g.fillRect(r.reduced(1.0f, 0.5f));
    g.setColour(tm.border().withAlpha(0.4f));
    g.drawRect(r.reduced(1.0f, 0.5f), 0.5f);

    g.setFont(PluginFonts::mono(8.0f));
    g.setColour(tm.muted());
    g.drawText(sendNames[idx], r.reduced(4.0f, 0.0f),
               juce::Justification::centredLeft, false);

    if (sendNames[idx] != "—")
    {
        g.setColour(juce::Colour(PluginColors::accent));
        g.drawText("0.0", r.reduced(0, 0), juce::Justification::centredRight, false);
    }
    juce::ignoreUnused(idx);
}

void ChannelStrip::drawIORow(juce::Graphics& g, juce::Rectangle<float> r,
                              const juce::String& label, const juce::String& value)
{
    auto& tm = ThemeManager::get();
    g.setColour(tm.surface().darker(0.03f));
    g.fillRect(r);
    g.setColour(tm.border().withAlpha(0.3f));
    g.drawHorizontalLine((int)r.getBottom() - 1, r.getX(), r.getRight());

    g.setFont(PluginFonts::mono(8.0f));
    g.setColour(tm.dim());
    g.drawText(label + ":", r.reduced(4.0f, 1.0f).removeFromLeft(20.0f),
               juce::Justification::centredLeft, false);
    g.setColour(tm.muted());
    g.drawText(value, r.reduced(26.0f, 1.0f), juce::Justification::centredLeft, true);
}

void ChannelStrip::drawGroupRow(juce::Graphics& g, juce::Rectangle<float> r)
{
    auto& tm = ThemeManager::get();
    g.setColour(tm.bg());
    g.fillRect(r);

    g.setFont(PluginFonts::mono(7.5f));
    g.setColour(tm.dim());
    g.drawText("GRP", r.reduced(3.0f, 2.0f).removeFromLeft(26.0f),
               juce::Justification::centredLeft, false);

    // A, B, C group buttons (drawn)
    float btnW = 18.0f;
    float btnY = r.getY() + 2.0f;
    float btnH = r.getHeight() - 4.0f;
    float startX = r.getX() + 30.0f;
    const char* grps[] = {"A", "B", "C"};
    for (int i = 0; i < 3; ++i)
    {
        juce::Rectangle<float> btn(startX + i * (btnW + 2), btnY, btnW, btnH);
        g.setColour(tm.surfaceHi());
        g.fillRoundedRectangle(btn, 2.0f);
        g.setColour(tm.border());
        g.drawRoundedRectangle(btn, 2.0f, 0.5f);
        g.setFont(PluginFonts::mono(8.0f));
        g.setColour(tm.muted());
        g.drawText(grps[i], btn, juce::Justification::centred, false);
    }
}

//==============================================================================
void ChannelStrip::resized()
{
    auto area = getLocalBounds();

    // Skip the painted sections
    int paintedTop = 3 + 24 + 13 + (18 * 4) + 2 + 13 + (16 * 4) + 2 + 18 + 18 + 2 + 20 + 2;
    area.removeFromTop(paintedTop);

    // Pan knob (44px square)
    panKnob.setBounds(area.removeFromTop(44).reduced(12, 2));

    // Remaining: fader + VU side-by-side, then dB, then mute/solo
    int bottomH = 14 + 26; // dB + S/M
    auto middleArea = area.removeFromTop(area.getHeight() - bottomH);

    // VU on the left (14px), fader fills rest
    vuRect = middleArea.removeFromLeft(14);
    vuMeter.setBounds(vuRect);
    faderRect = middleArea.reduced(8, 2);
    fader.setBounds(faderRect);

    // dB readout
    dbReadout.setBounds(area.removeFromTop(14));

    // Mute / Solo
    auto smRow = area.removeFromTop(26);
    int smW = smRow.getWidth() / 2;
    soloBtn.setBounds(smRow.removeFromLeft(smW).reduced(1));
    muteBtn.setBounds(smRow.reduced(1));
}

//==============================================================================
void ChannelStrip::mouseDown(const juce::MouseEvent& e)
{
    // Right-click on VU meter → meter type menu
    if (e.mods.isRightButtonDown() && vuRect.contains(e.getPosition()))
    {
        showMeterMenu();
        return;
    }
    juce::Component::mouseDown(e);
}

void ChannelStrip::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (eqSlotRect.contains(e.getPosition()))
        openEQWindow();
}

void ChannelStrip::openEQWindow()
{
    if (!eqWindow)
        eqWindow = std::make_unique<ParametricEQWindow>(name);

    eqWindow->setVisible(true);
    eqWindow->toFront(true);
}

void ChannelStrip::showMeterMenu()
{
    juce::PopupMenu menu;
    menu.addSectionHeader("Metering Type");
    menu.addItem(1, "Peak",          true, meterType == MeterType::Peak);
    menu.addItem(2, "VU",            true, meterType == MeterType::VU);
    menu.addItem(3, "RMS",           true, meterType == MeterType::RMS);
    menu.addItem(4, "K-14",          true, meterType == MeterType::K14);
    menu.addItem(5, "K-20",          true, meterType == MeterType::K20);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
        [this](int result)
        {
            if (result == 1) meterType = MeterType::Peak;
            else if (result == 2) meterType = MeterType::VU;
            else if (result == 3) meterType = MeterType::RMS;
            else if (result == 4) meterType = MeterType::K14;
            else if (result == 5) meterType = MeterType::K20;
        });
}

//==============================================================================
void ChannelStrip::setProcessor(DrumTechProcessor* proc) { processor = proc; }

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
