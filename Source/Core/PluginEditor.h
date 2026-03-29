#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include "PluginProcessor.h"
#include "../UI/DrumKitView.h"
#include "../UI/MixerView.h"
#include "../UI/GrooveBrowser.h"
#include "../UI/GrooveTimeline.h"
#include "../UI/SettingsView.h"
#include "../UI/TransportBar.h"
#include "../UI/PluginColors.h"
#include "../UI/DrumTechLookAndFeel.h"
#include "../UI/ThemeManager.h"
#include "DebugConsole.h"
#include "ErrorLogger.h"

class DrumTechEditor : public juce::AudioProcessorEditor,
                           public ErrorLogger::Listener,
                           private juce::Timer,
                           private juce::KeyListener
{
public:
    explicit DrumTechEditor(DrumTechProcessor&);
    ~DrumTechEditor() override;

    void paint  (juce::Graphics&) override;
    void resized() override;
    void logUpdated() override;

    bool keyPressed(const juce::KeyPress& key, juce::Component* originator) override;

    void timerCallback() override
    {
        for (int note = 0; note < 128; ++note)
            if (processor.checkAndClearMidiNote(note))
                kitView.triggerPieceVisual(note);

        const auto& player = processor.getMIDIPlayer();
        transportBar.setBPM(static_cast<float>(player.getTempo()));
        transportBar.setPositionLabel(
            juce::String(static_cast<int>(player.getPosition()) + 1).paddedLeft('0', 3)
            + ":01:000");

        if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(
                processor.getAPVTS().getParameter("samplerDrummerProfile")))
            settingsView.setDrummerProfileIndex(p->getIndex());
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(
                processor.getAPVTS().getParameter("samplerPlayingStyle")))
            settingsView.setPlayingStyleIndex(p->getIndex());

        const float delta = mixerSlideTarget - mixerSlideAmount;
        if (std::abs(delta) > 0.001f)
        {
            mixerSlideAmount += delta * 0.2f;
            if (std::abs(mixerSlideTarget - mixerSlideAmount) < 0.005f)
                mixerSlideAmount = mixerSlideTarget;
            resized();
        }
    }

private:
    struct MixerDismissLayer : public juce::Component
    {
        std::function<void()> onDismiss;
        void paint(juce::Graphics&) override {}
        void mouseDown(const juce::MouseEvent&) override
        {
            if (onDismiss) onDismiss();
        }
    };

    void updateStatus();
    void toggleDebugConsole();
    void switchTab(int tab);
    void paintTabBar(juce::Graphics& g, juce::Rectangle<int> area);
    void paintInstrumentBar(juce::Graphics& g, juce::Rectangle<int> instrBar);
    juce::String activeTabContextTitle() const;
    void applyTheme();
    void showKitPopupMenu();

    DrumTechProcessor& processor;

    juce::Image logoImage;

    // Tab buttons
    juce::TextButton tabKit    {"HOME"};
    juce::TextButton tabMixer  {"MIXER"};
    juce::TextButton tabSettings{"SETTINGS"};

    // Top-right controls
    juce::TextButton kitSelectorBtn{"Kit: Default"};
    juce::TextButton kitLockBtn   {"LOCK: OFF"};
    juce::TextButton themeModeBtn  {"LIGHT"};

    // Content views
    DrumKitView   kitView;
    GrooveBrowser grooveBrowser;
    MixerView     mixerView;
    MixerDismissLayer mixerDismissLayer;
    SettingsView  settingsView;

    // Bottom persistent bar
    GrooveTimeline grooveTimeline;
    TransportBar   transportBar;

    // Debug console
    std::unique_ptr<DebugConsole> debugConsole;
    bool debugConsoleVisible = false;

    int          activeTab     = 0;
    bool         kitLocked     = false;
    juce::String currentKitName{"Default Kit"};
    float        mixerSlideAmount = 0.0f;
    float        mixerSlideTarget = 0.0f;

    static constexpr int topNavH       = 54;
    static constexpr int instrBarH     = 44;
    static constexpr int midiRollH     = GrooveTimeline::trackBarH
                                       + GrooveTimeline::rulerH
                                       + GrooveTimeline::rollH;
    static constexpr int transportBarH = 56;

    DrumTechLookAndFeel appLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumTechEditor)
};
