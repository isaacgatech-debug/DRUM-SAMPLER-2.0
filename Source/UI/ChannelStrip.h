#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AnalogKnob.h"
#include "VUMeter.h"
#include "PluginColors.h"
#include "ParametricEQWindow.h"

class DrumTechProcessor;

/**
 * Pro Tools inspired channel strip (100px wide).
 * Sections (top → bottom):
 *   3px color bar | 24px header | INSERTS (4 slots) |
 *   SENDS (4 slots) | I/O rows | GROUPS | PAN knob |
 *   FADER + VU meter (fills) | dB readout | MUTE/SOLO
 *
 * Double-click EQ insert slot → opens ParametricEQWindow.
 * Right-click VU meter        → choose metering type.
 */
class ChannelStrip : public juce::Component, private juce::Button::Listener
{
public:
    ChannelStrip(int channelIndex, const juce::String& channelName);

    void paint         (juce::Graphics& g) override;
    void resized       () override;
    void mouseDown     (const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

    void setProcessor(DrumTechProcessor* proc);
    void updateMeter(float level, float peak);

    int          getChannelIndex() const { return index; }
    juce::String getChannelName()  const { return name;  }

    std::function<void(int, const juce::String&, float)> onParameterChanged;
    std::function<void(int, const juce::String&, bool)>  onButtonChanged;

    static constexpr int stripWidth = 100;

    enum class MeterType { Peak, VU, RMS, K14, K20 };

private:
    void buttonClicked(juce::Button* b) override;

    // Draw helpers
    void drawColorBar    (juce::Graphics& g, juce::Rectangle<float> b);
    void drawHeader      (juce::Graphics& g, juce::Rectangle<float> b);
    void drawSectionLabel(juce::Graphics& g, juce::Rectangle<float> b,
                          const juce::String& label);
    void drawInsertSlot  (juce::Graphics& g, juce::Rectangle<float> b,
                          int idx, bool isEQ);
    void drawSendSlot    (juce::Graphics& g, juce::Rectangle<float> b,
                          int idx);
    void drawIORow       (juce::Graphics& g, juce::Rectangle<float> b,
                          const juce::String& label, const juce::String& value);
    void drawGroupRow    (juce::Graphics& g, juce::Rectangle<float> b);

    void openEQWindow();
    void showMeterMenu();

    int          index;
    juce::String name;
    juce::String shortName;
    DrumTechProcessor* processor = nullptr;
    juce::Colour channelColor;

    MeterType meterType = MeterType::Peak;

    // Pan knob
    AnalogKnob panKnob{AnalogKnob::Style::Small};

    // Solo / Mute
    juce::TextButton soloBtn{"S"};
    juce::TextButton muteBtn{"M"};

    // VU meter
    VUMeter vuMeter{VUMeter::Orientation::Vertical};

    // Fader
    juce::Slider fader;

    // dB readout
    juce::Label dbReadout;

    // EQ pop-out window
    std::unique_ptr<ParametricEQWindow> eqWindow;

    // Layout rects computed in resized() — used for hit testing
    juce::Rectangle<int> eqSlotRect;     // double-click → EQ window
    juce::Rectangle<int> vuRect;         // right-click  → meter type menu
    juce::Rectangle<int> faderRect;
    juce::Rectangle<int> insertsStartY;  // not rect but tracked via resized

    // Insert slot names (slot 0 = always EQ)
    juce::String insertNames[4] = {"EQ", "—", "—", "—"};
    juce::String sendNames[4]   = {"—", "—", "—", "—"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStrip)
};
