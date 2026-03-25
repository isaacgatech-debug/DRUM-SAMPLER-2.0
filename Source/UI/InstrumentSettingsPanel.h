#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginColors.h"

/**
 * Right-side 260px instrument settings panel for DrumKitView.
 * Contains: Level, Reverse, MIDI Monitor, Voice&Layer (collapsible),
 * per-param toggles, MIDI Mapping (collapsible), and bottom button.
 */
class InstrumentSettingsPanel : public juce::Component
{
public:
    InstrumentSettingsPanel();

    void paint  (juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    void drawSectionHeader(juce::Graphics& g, juce::Rectangle<float> area,
                           const juce::String& label, bool collapsed);
    void drawRow(juce::Graphics& g, juce::Rectangle<float> area,
                 const juce::String& label, const juce::String& valueStr);
    void drawToggleRow(juce::Graphics& g, juce::Rectangle<float> area,
                       const juce::String& label, bool state);

    void layoutRows();

    // Controls
    juce::Slider levelSlider;
    juce::Label  levelReadout;
    juce::TextButton soloBtn  {"S"};
    juce::TextButton muteBtn  {"M"};

    juce::Slider softHitSlider;
    juce::Label  softHitReadout;

    juce::TextButton midiSettingsBtn{"MIDI In/E-Drums Settings"};

    // Toggle states
    bool reverseOn      = false;
    bool midiMonitorOn  = false;
    bool smoothingOn    = false;
    bool hiHatCCEditOn  = false;
    bool levelEnvOn     = false;
    bool velocityGateOn = false;

    // Collapsible sections
    bool voiceLayerOpen = true;
    bool midiMappingOpen= true;

    // Voice & Layer values
    int hardVal = 4, gradVal = 4, softVal = 4, voiceLimitVal = 8;

    // Clickable row rects (for toggles and section headers)
    juce::Rectangle<int> reverseRow, midiMonitorRow, smoothingRow,
                          hiHatRow, levelEnvRow, velocityGateRow,
                          voiceLayerHeader, midiMappingHeader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentSettingsPanel)
};
