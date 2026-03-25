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
#include "../UI/ThemeManager.h"
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
    void applyTheme();
    void showKitPopupMenu();

    DrumSampler2Processor& processor;

    juce::Image logoImage;

    // Tab buttons
    juce::TextButton tabKit    {"KIT"};
    juce::TextButton tabMixer  {"MIXER"};
    juce::TextButton tabTrigger{"TRIGGER"};
    juce::TextButton tabGrooves{"GROOVES"};

    // Top-right controls
    juce::TextButton kitSelectorBtn{"Kit: Default"};
    juce::TextButton themeModeBtn  {"LIGHT"};

    // Content views
    DrumKitView   kitView;
    GrooveBrowser grooveBrowser;
    MixerView     mixerView;
    TriggerUI     triggerUI;

    // Bottom persistent bar
    GrooveTimeline grooveTimeline;
    TransportBar   transportBar;

    // Debug console
    std::unique_ptr<DebugConsole> debugConsole;
    bool debugConsoleVisible = false;

    int          activeTab     = 0;
    juce::String currentKitName{"Default Kit"};

    static constexpr int topNavH       = 44;
    static constexpr int instrBarH     = 36;
    static constexpr int midiRollH     = GrooveTimeline::trackBarH
                                       + GrooveTimeline::rulerH
                                       + GrooveTimeline::rollH;
    static constexpr int transportBarH = 44;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumSampler2Editor)
};
