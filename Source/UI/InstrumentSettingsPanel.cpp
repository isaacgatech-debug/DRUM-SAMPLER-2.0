#include "InstrumentSettingsPanel.h"
#include "../Core/PluginProcessor.h"
#include "../Core/MicBusLayout.h"
#include <array>
#include <functional>

static constexpr int rowH           = UIDesign::instrumentRowH;
static constexpr int sliderControlH = UIDesign::instrumentSliderHeight;
static constexpr int micViewportH   = 168;

static int curveTypeToIndex(VelocityCurve::CurveType t)
{
    switch (t)
    {
        case VelocityCurve::CurveType::Exponential: return 1;
        case VelocityCurve::CurveType::Logarithmic: return 2;
        case VelocityCurve::CurveType::SCurve:      return 3;
        case VelocityCurve::CurveType::Linear:
        default: return 0;
    }
}

static VelocityCurve::CurveType indexToCurveType(int idx)
{
    switch (idx)
    {
        case 1: return VelocityCurve::CurveType::Exponential;
        case 2: return VelocityCurve::CurveType::Logarithmic;
        case 3: return VelocityCurve::CurveType::SCurve;
        default: return VelocityCurve::CurveType::Linear;
    }
}

//==============================================================================
class MicTonesRows : public juce::Component
{
public:
    std::function<void(int micIndex, float value)> onMicTrimChanged;

    MicTonesRows()
    {
        for (int i = 0; i < MicBus::count; ++i)
        {
            labels[static_cast<size_t>(i)].setText(MicBus::getName(i), juce::dontSendNotification);
            labels[static_cast<size_t>(i)].setFont(PluginFonts::mono(9.5f));
            labels[static_cast<size_t>(i)].setColour(juce::Label::textColourId,
                                                      juce::Colour(PluginColors::textMuted));
            labels[static_cast<size_t>(i)].setJustificationType(juce::Justification::centredLeft);
            addAndMakeVisible(labels[static_cast<size_t>(i)]);

            auto& sl = sliders[static_cast<size_t>(i)];
            sl.setRange(0.0, 4.0, 0.01);
            sl.setValue(1.0);
            sl.setSliderStyle(juce::Slider::LinearHorizontal);
            sl.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            sl.setColour(juce::Slider::thumbColourId, juce::Colour(PluginColors::accent));
            sl.setColour(juce::Slider::trackColourId, juce::Colour(PluginColors::accentDim));
            sl.setColour(juce::Slider::backgroundColourId, juce::Colour(PluginColors::pluginBg));
            const int idx = i;
            sl.onValueChange = [this, idx]
            {
                if (onMicTrimChanged)
                    onMicTrimChanged(idx, static_cast<float>(sliders[static_cast<size_t>(idx)].getValue()));
            };
            addAndMakeVisible(sl);
        }
    }

    void setMicTrim(int micIndex, float v)
    {
        if (micIndex >= 0 && micIndex < MicBus::count)
            sliders[static_cast<size_t>(micIndex)].setValue(static_cast<double>(v),
                                                            juce::dontSendNotification);
    }

    void resized() override
    {
        auto a = getLocalBounds();
        const int rh = 22;
        for (int i = 0; i < MicBus::count; ++i)
        {
            auto row = a.removeFromTop(rh);
            labels[static_cast<size_t>(i)].setBounds(row.removeFromLeft(78));
            sliders[static_cast<size_t>(i)].setBounds(row.reduced(2, 3));
        }
    }

    int getIdealHeight() const { return MicBus::count * 22; }

private:
    std::array<juce::Label, MicBus::count> labels;
    std::array<juce::Slider, MicBus::count> sliders;
};

//==============================================================================
InstrumentSettingsPanel::InstrumentSettingsPanel()
{
    micTones = std::make_unique<MicTonesRows>();
    micTones->onMicTrimChanged = [this](int micIndex, float v)
    {
        if (processor != nullptr)
            processor->getSamplerEngine().setMicTrim(selectedMidiNote, micIndex, v);
    };

    micViewport.setViewedComponent(micTones.get(), false);
    micViewport.setScrollBarsShown(true, false);
    micViewport.setScrollBarThickness(8);
    addAndMakeVisible(micViewport);

    pitchSlider.setRange(-24.0, 24.0, 0.01);
    pitchSlider.setValue(0.0);
    pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pitchSlider.setColour(juce::Slider::thumbColourId, juce::Colour(PluginColors::accent));
    pitchSlider.setColour(juce::Slider::trackColourId, juce::Colour(PluginColors::accentDim));
    pitchSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(PluginColors::pluginBg));
    pitchSlider.onValueChange = [this]
    {
        const float v = static_cast<float>(pitchSlider.getValue());
        pitchReadout.setText(juce::String(v, 1), juce::dontSendNotification);
        if (processor != nullptr)
            processor->getSamplerEngine().setPitchForNote(selectedMidiNote, v);
    };
    addAndMakeVisible(pitchSlider);

    pitchReadout.setText("0.0", juce::dontSendNotification);
    pitchReadout.setFont(PluginFonts::mono(12.0f));
    pitchReadout.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    pitchReadout.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(pitchReadout);

    articulationBox.addItem("Auto", 1);
    articulationBox.addItem("Tip", 2);
    articulationBox.addItem("Side", 3);
    articulationBox.addItem("Closed", 4);
    articulationBox.addItem("Open", 5);
    articulationBox.addItem("Bell", 6);
    articulationBox.addItem("Rimshot", 7);
    articulationBox.addItem("Rim", 8);
    articulationBox.addItem("Crossstick", 9);
    articulationBox.addItem("Flam", 10);
    articulationBox.addItem("Roll", 11);
    articulationBox.addItem("Muted", 12);
    articulationBox.addItem("Swirl", 13);
    articulationBox.setSelectedId(1);
    articulationBox.onChange = [this]
    {
        if (processor == nullptr)
            return;
        const auto text = articulationBox.getText().toLowerCase();
        processor->getSamplerEngine().setArticulationHintForNote(
            selectedMidiNote, text == "auto" ? juce::String() : text);
    };
    addAndMakeVisible(articulationBox);

    velocityCurveBox.addItem("Linear", 1);
    velocityCurveBox.addItem("Exponential", 2);
    velocityCurveBox.addItem("Logarithmic", 3);
    velocityCurveBox.addItem("S-Curve", 4);
    velocityCurveBox.setSelectedId(1);
    velocityCurveBox.onChange = [this]
    {
        if (processor == nullptr)
            return;
        auto curve = processor->getSamplerEngine().getVelocityCurveForNote(selectedMidiNote);
        curve.responseType = indexToCurveType(velocityCurveBox.getSelectedItemIndex());
        processor->getSamplerEngine().setVelocityCurveForNote(selectedMidiNote, curve);
    };
    addAndMakeVisible(velocityCurveBox);

    // Level slider
    levelSlider.setRange(0.0, 1.0, 0.01);
    levelSlider.setValue(0.0);
    levelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    levelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    levelSlider.setColour(juce::Slider::thumbColourId, juce::Colour(PluginColors::accent));
    levelSlider.setColour(juce::Slider::trackColourId, juce::Colour(PluginColors::accentDim));
    levelSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(PluginColors::pluginBg));
    addAndMakeVisible(levelSlider);

    levelReadout.setText("0.0", juce::dontSendNotification);
    levelReadout.setFont(PluginFonts::mono(12.0f));
    levelReadout.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    levelReadout.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(levelReadout);

    levelSlider.onValueChange = [this]
    {
        levelReadout.setText(juce::String(levelSlider.getValue(), 1), juce::dontSendNotification);
    };

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

    softHitSlider.setRange(0.0, 1.0, 0.01);
    softHitSlider.setValue(0.0);
    softHitSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    softHitSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    softHitSlider.setColour(juce::Slider::thumbColourId, juce::Colour(PluginColors::accent));
    softHitSlider.setColour(juce::Slider::trackColourId, juce::Colour(PluginColors::accentDim));
    softHitSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(PluginColors::pluginBg));
    addAndMakeVisible(softHitSlider);

    softHitReadout.setText("0.0", juce::dontSendNotification);
    softHitReadout.setFont(PluginFonts::mono(12.0f));
    softHitReadout.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    softHitReadout.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(softHitReadout);

    softHitSlider.onValueChange = [this]
    {
        softHitReadout.setText(juce::String(softHitSlider.getValue(), 1), juce::dontSendNotification);
    };

    midiSettingsBtn.setButtonText("MIDI & E-drum setup");
    midiSettingsBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginPanel));
    midiSettingsBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textMuted));
    midiSettingsBtn.onClick = [this]
    {
        midiMappingOpen = !midiMappingOpen;
        resized();
        repaint();
    };
    addAndMakeVisible(midiSettingsBtn);
}

InstrumentSettingsPanel::~InstrumentSettingsPanel() = default;

void InstrumentSettingsPanel::setSelectedDrum(const juce::String& drumName, int midiNote)
{
    selectedDrumName = drumName;
    selectedMidiNote = midiNote;
    syncControlsFromSampler();
    repaint();
}

void InstrumentSettingsPanel::syncControlsFromSampler()
{
    if (processor == nullptr || micTones == nullptr)
        return;

    const float pit = processor->getSamplerEngine().getPitchForNote(selectedMidiNote);
    pitchSlider.setValue(static_cast<double>(pit), juce::dontSendNotification);
    pitchReadout.setText(juce::String(pit, 1), juce::dontSendNotification);

    const auto artHint = processor->getSamplerEngine().getArticulationHintForNote(selectedMidiNote);
    if (artHint.isEmpty())
        articulationBox.setSelectedId(1, juce::dontSendNotification);
    else
    {
        const juce::String k = artHint.toLowerCase();
        int id = 1;
        if (k == "tip") id = 2;
        else if (k == "side") id = 3;
        else if (k == "closed") id = 4;
        else if (k == "open") id = 5;
        else if (k == "bell") id = 6;
        else if (k == "rimshot") id = 7;
        else if (k == "rim") id = 8;
        else if (k == "crossstick") id = 9;
        else if (k == "flam") id = 10;
        else if (k == "roll") id = 11;
        else if (k == "muted") id = 12;
        else if (k == "swirl") id = 13;
        articulationBox.setSelectedId(id, juce::dontSendNotification);
    }

    const auto curve = processor->getSamplerEngine().getVelocityCurveForNote(selectedMidiNote);
    velocityCurveBox.setSelectedItemIndex(curveTypeToIndex(curve.responseType), juce::dontSendNotification);

    for (int m = 0; m < MicBus::count; ++m)
        micTones->setMicTrim(m, processor->getSamplerEngine().getMicTrim(selectedMidiNote, m));
}

void InstrumentSettingsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(PluginColors::pluginPanel));
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawVerticalLine(0, 0, static_cast<float>(getHeight()));

    auto area = getLocalBounds().toFloat().reduced(4.0f, 0.0f);
    float y = 4.0f;
    float w = area.getWidth();
    float x = area.getX();

    g.setFont(PluginFonts::label(12.0f));
    g.setColour(juce::Colour(PluginColors::textPrimary));
    g.drawText("Level", juce::Rectangle<float>(x, y, w * 0.35f, static_cast<float>(rowH)),
               juce::Justification::centredLeft, false);
    y += static_cast<float>(rowH + 4 + sliderControlH + 4);

    g.drawText("Pitch (semitones)", juce::Rectangle<float>(x, y, w * 0.55f, static_cast<float>(rowH)),
               juce::Justification::centredLeft, false);
    y += static_cast<float>(rowH + 4 + sliderControlH + 4);

    g.drawText("Articulation Hint", juce::Rectangle<float>(x, y, w * 0.55f, static_cast<float>(rowH)),
               juce::Justification::centredLeft, false);
    y += static_cast<float>(rowH + 2 + 26 + 4);

    g.drawText("Velocity Curve", juce::Rectangle<float>(x, y, w * 0.55f, static_cast<float>(rowH)),
               juce::Justification::centredLeft, false);
    y += static_cast<float>(rowH + 2 + 26 + 4);

    g.setColour(juce::Colour(PluginColors::textMuted));
    g.setFont(PluginFonts::label(11.0f));
    g.drawText("Mic tone (per stem)", juce::Rectangle<float>(x, y, w, static_cast<float>(rowH)),
               juce::Justification::centredLeft, false);
    y += static_cast<float>(rowH + 2 + micViewportH + 6);

    reverseRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y),
                                       static_cast<int>(w), rowH + reverseSubLabelH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, static_cast<float>(rowH)), "Reverse", reverseOn);
    g.setFont(PluginFonts::mono(10.0f));
    g.setColour(juce::Colour(PluginColors::textMuted));
    g.drawText("~200 ms tail", static_cast<int>(x + 8), static_cast<int>(y + rowH),
               static_cast<int>(w - 16), reverseSubLabelH, juce::Justification::centredLeft, false);
    y += static_cast<float>(rowH + reverseSubLabelH);

    midiMonitorRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y),
                                            static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, static_cast<float>(rowH)), "MIDI Monitor", midiMonitorOn);
    y += static_cast<float>(rowH);

    y += 4.0f;

    voiceLayerHeader = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y),
                                             static_cast<int>(w), rowH);
    drawSectionHeader(g, juce::Rectangle<float>(x, y, w, static_cast<float>(rowH)), "Voice and Layer", !voiceLayerOpen);
    y += static_cast<float>(rowH);

    if (voiceLayerOpen)
    {
        g.setFont(PluginFonts::label(11.0f));
        g.setColour(juce::Colour(PluginColors::textMuted));
        auto layerRow = juce::Rectangle<float>(x + 8.0f, y, w - 8.0f, static_cast<float>(rowH));
        float cw = (w - 8.0f) / 3.0f;
        g.drawText("Hard " + juce::String(hardVal), layerRow.removeFromLeft(cw), juce::Justification::centredLeft, false);
        g.drawText("Grad " + juce::String(gradVal), layerRow.removeFromLeft(cw), juce::Justification::centredLeft, false);
        g.drawText("Soft " + juce::String(softVal), layerRow, juce::Justification::centredLeft, false);
        y += static_cast<float>(rowH);

        g.drawText("Voice Limit  " + juce::String(voiceLimitVal),
                   static_cast<int>(x + 8), static_cast<int>(y), static_cast<int>(w - 8), rowH,
                   juce::Justification::centredLeft, false);
        y += static_cast<float>(rowH);
    }

    y += 4.0f;

    g.setFont(PluginFonts::label(12.0f));
    g.setColour(juce::Colour(PluginColors::textPrimary));
    g.drawText("Soft Hit Level", static_cast<int>(x), static_cast<int>(y),
               static_cast<int>(w * 0.6f), rowH, juce::Justification::centredLeft, false);
    y += static_cast<float>(rowH + sliderControlH + 4);

    smoothingRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, static_cast<float>(rowH)), "Smoothing", smoothingOn);
    y += static_cast<float>(rowH);

    hiHatRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, static_cast<float>(rowH)), "Hi-Hat CC Edit", hiHatCCEditOn);
    y += static_cast<float>(rowH);

    levelEnvRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, static_cast<float>(rowH)), "Level Envelope", levelEnvOn);
    y += static_cast<float>(rowH);

    velocityGateRow = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), rowH);
    drawToggleRow(g, juce::Rectangle<float>(x, y, w, static_cast<float>(rowH)), "Velocity Gate", velocityGateOn);
    y += static_cast<float>(rowH + 4);

    midiMappingHeader = juce::Rectangle<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), rowH);
    drawSectionHeader(g, juce::Rectangle<float>(x, y, w, static_cast<float>(rowH)), "MIDI Mapping", !midiMappingOpen);
    y += static_cast<float>(rowH);

    if (midiMappingOpen)
    {
        g.setFont(PluginFonts::mono(12.0f));
        g.setColour(juce::Colour(PluginColors::textPrimary));
        drawRow(g, juce::Rectangle<float>(x + 8, y, w - 8, static_cast<float>(rowH)), "Open", "36");
        y += static_cast<float>(rowH);
        drawRow(g, juce::Rectangle<float>(x + 8, y, w - 8, static_cast<float>(rowH)), "Hit", "35");
    }
}

void InstrumentSettingsPanel::resized()
{
    auto area = getLocalBounds().reduced(4, 0);
    int y = 4;

    y += rowH + 4;
    {
        auto sliderRow = juce::Rectangle<int>(area.getX(), y, area.getWidth(), sliderControlH);
        int btnW = 36;
        muteBtn.setBounds(sliderRow.removeFromRight(btnW + 2).reduced(1));
        soloBtn.setBounds(sliderRow.removeFromRight(btnW + 2).reduced(1));
        levelReadout.setBounds(sliderRow.removeFromRight(44));
        levelSlider.setBounds(sliderRow);
    }
    y += sliderControlH + 4;

    y += rowH + 4;
    {
        auto pitchRow = juce::Rectangle<int>(area.getX(), y, area.getWidth(), sliderControlH);
        pitchReadout.setBounds(pitchRow.removeFromRight(44));
        pitchSlider.setBounds(pitchRow);
    }
    y += sliderControlH + 4;

    y += rowH + 2;
    articulationBox.setBounds(area.getX(), y, area.getWidth(), 26);
    y += 26 + 4;

    y += rowH + 2;
    velocityCurveBox.setBounds(area.getX(), y, area.getWidth(), 26);
    y += 26 + 4;

    y += rowH + 2;
    micViewport.setBounds(area.getX(), y, area.getWidth(), micViewportH);
    if (micTones != nullptr)
    {
        const int ih = micTones->getIdealHeight();
        micTones->setSize(area.getWidth() - 10, juce::jmax(ih, micViewportH));
    }
    y += micViewportH + 6;

    y += rowH + reverseSubLabelH;
    y += rowH;
    y += 4;
    y += rowH;
    if (voiceLayerOpen)
        y += rowH * 2;
    y += 4 + rowH + sliderControlH + 4;
    y += rowH * 4 + 4;
    y += rowH;
    if (midiMappingOpen)
        y += rowH * 2;

    int remainingH = getHeight() - y - 4;
    if (remainingH > 0)
        midiSettingsBtn.setBounds(area.getX(), y, area.getWidth(), juce::jmin(30, remainingH));
    else
        midiSettingsBtn.setBounds(area.getX(), juce::jmax(0, getHeight() - 34), area.getWidth(), 30);
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

    g.setFont(PluginFonts::label(12.0f));
    g.setColour(juce::Colour(PluginColors::accent));
    g.drawText(juce::String(collapsed ? "[+] " : "[-] ") + label, area.reduced(8.0f, 0.0f),
               juce::Justification::centredLeft, false);
}

void InstrumentSettingsPanel::drawRow(juce::Graphics& g, juce::Rectangle<float> area,
                                       const juce::String& label, const juce::String& valueStr)
{
    g.setColour(juce::Colour(PluginColors::textPrimary));
    g.setFont(PluginFonts::label(12.0f));
    g.drawText(label, area.reduced(2.0f, 0.0f), juce::Justification::centredLeft, false);

    g.setFont(PluginFonts::mono(12.0f));
    g.setColour(juce::Colour(PluginColors::accent));
    g.drawText(valueStr, area.reduced(2.0f, 0.0f), juce::Justification::centredRight, false);

    g.setColour(juce::Colour(PluginColors::textMuted));
    g.drawText("=", static_cast<int>(area.getRight() - 14), static_cast<int>(area.getY()),
               12, static_cast<int>(area.getHeight()), juce::Justification::centred, false);
}

void InstrumentSettingsPanel::drawToggleRow(juce::Graphics& g, juce::Rectangle<float> area,
                                             const juce::String& label, bool state)
{
    g.setColour(juce::Colour(PluginColors::pluginBg).withAlpha(0.5f));
    g.fillRect(area);

    g.setFont(PluginFonts::label(12.0f));
    g.setColour(juce::Colour(PluginColors::textPrimary));
    g.drawText(label, area.reduced(6.0f, 0.0f), juce::Justification::centredLeft, false);

    juce::Rectangle<float> pill(area.getRight() - 48.0f, area.getCentreY() - 11.0f, 44.0f, 22.0f);
    g.setColour(state ? juce::Colour(PluginColors::accent) : juce::Colour(PluginColors::pluginBorder));
    g.fillRoundedRectangle(pill, 11.0f);

    float thumbX = state ? pill.getRight() - 17.0f : pill.getX() + 3.0f;
    g.setColour(state ? juce::Colours::black : juce::Colour(PluginColors::textMuted));
    g.fillEllipse(thumbX, pill.getY() + 3.0f, 14.0f, 14.0f);

    g.setFont(PluginFonts::mono(10.5f));
    g.setColour(state ? juce::Colour(PluginColors::textMuted) : juce::Colour(PluginColors::textDim));
    g.drawText(state ? "On" : "Off", pill.reduced(1.0f), juce::Justification::centred, false);
}
