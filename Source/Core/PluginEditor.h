#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "../UI/DrumKitView.h"
#include "../UI/MixerView.h"
#include "../UI/GrooveBrowser.h"
#include "../UI/GrooveTimeline.h"
#include "../UI/TransportBar.h"
#include "../UI/PluginColors.h"
#include "../Trigger/TriggerUI.h"
#include "DebugConsole.h"
#include "ErrorLogger.h"

class DrumSampler2Editor : public juce::AudioProcessorEditor,
                           public ErrorLogger::Listener,
                           private juce::Timer,
                           private juce::KeyListener
{
public:
    explicit DrumSampler2Editor(DrumSampler2Processor&);
    ~DrumSampler2Editor() override;

    void paint  (juce::Graphics&) override;
    void resized() override;
    void logUpdated() override;

    bool keyPressed(const juce::KeyPress& key, juce::Component* originator) override;

    void timerCallback() override
    {
        for (int note = 0; note < 128; ++note)
            if (processor.checkAndClearMidiNote(note))
                kitView.triggerPieceVisual(note);
    }

private:
    void updateStatus();
    void toggleDebugConsole();
    void switchTab(int tab);
    void paintTabBar(juce::Graphics& g, juce::Rectangle<int> area);

    DrumSampler2Processor& processor;

    // Branding
    juce::Image logoImage;

    // Tab buttons
    juce::TextButton tabKit    {"KIT"};
    juce::TextButton tabMixer  {"MIXER"};
    juce::TextButton tabTrigger{"TRIGGER"};
    juce::TextButton tabGrooves{"GROOVES"};

    // Instrument selector bar (kit dropdown placeholder)
    juce::ComboBox kitSelector;

    // Tab content views
    DrumKitView   kitView;
    GrooveBrowser grooveBrowser;
    MixerView     mixerView;
    TriggerUI     triggerUI;

    // MIDI roll — always visible at bottom
    GrooveTimeline grooveTimeline;
    TransportBar   transportBar;

    // Debug console (Ctrl+Shift+D)
    std::unique_ptr<DebugConsole> debugConsole;
    bool debugConsoleVisible = false;

    int activeTab = 0;

    // New design tokens
    juce::Colour bg     {PluginColors::pluginBg};
    juce::Colour header {PluginColors::pluginPanel};
    juce::Colour accent {PluginColors::accent};
    juce::Colour textCol{PluginColors::textPrimary};

    static constexpr int topNavH       = 44;
    static constexpr int instrBarH     = 36;
    static constexpr int midiRollH     = GrooveTimeline::trackBarH
                                       + GrooveTimeline::rulerH
                                       + GrooveTimeline::rollH;
    static constexpr int transportBarH = 44;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumSampler2Editor)
};
