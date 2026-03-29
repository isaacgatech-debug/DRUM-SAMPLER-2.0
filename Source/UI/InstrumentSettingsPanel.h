#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include "PluginColors.h"

class DrumTechProcessor;
class MicTonesRows;

/**
 * Right-side instrument settings panel for DrumKitView (width from DrumKitView::settingsPanelW).
 * Pitch, per-mic tone trims (15 stems), Level, collapsible sections, MIDI mapping.
 */
class InstrumentSettingsPanel : public juce::Component
{
public:
    InstrumentSettingsPanel();
    ~InstrumentSettingsPanel() override;

    void paint  (juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    void setProcessor(DrumTechProcessor* proc) { processor = proc; syncControlsFromSampler(); }

    /** Called when a DrumPiece is clicked — updates the panel for that drum. */
    void setSelectedDrum(const juce::String& drumName, int midiNote);

private:
    void drawSectionHeader(juce::Graphics& g, juce::Rectangle<float> area,
                           const juce::String& label, bool collapsed);
    void drawRow(juce::Graphics& g, juce::Rectangle<float> area,
                 const juce::String& label, const juce::String& valueStr);
    void drawToggleRow(juce::Graphics& g, juce::Rectangle<float> area,
                       const juce::String& label, bool state);

    void syncControlsFromSampler();

    // Controls
    juce::Slider pitchSlider;
    juce::Label  pitchReadout;
    juce::ComboBox articulationBox;
    juce::ComboBox velocityCurveBox;

    juce::Slider levelSlider;
    juce::Label  levelReadout;
    juce::TextButton soloBtn  {"S"};
    juce::TextButton muteBtn  {"M"};

    juce::Slider softHitSlider;
    juce::Label  softHitReadout;

    juce::TextButton midiSettingsBtn;

    juce::Viewport micViewport;
    std::unique_ptr<MicTonesRows> micTones;

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

    // Selected drum state
    juce::String selectedDrumName = "KICK";
    int          selectedMidiNote = 36;

    DrumTechProcessor* processor = nullptr;

    // Damper slider (0 = dead, 1 = live)
    juce::Slider damperSlider;
    juce::Label  damperLabel;

    // Voice & Layer values
    int hardVal = 4, gradVal = 4, softVal = 4, voiceLimitVal = 8;

    // Clickable row rects (for toggles and section headers)
    juce::Rectangle<int> reverseRow, midiMonitorRow, smoothingRow,
                          hiHatRow, levelEnvRow, velocityGateRow,
                          voiceLayerHeader, midiMappingHeader;

    static constexpr int reverseSubLabelH = 12;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentSettingsPanel)
};
