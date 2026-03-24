#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "../UI/DrumKitView.h"
#include "../UI/MixerView.h"
#include "../UI/GrooveBrowser.h"
#include "../Trigger/TriggerUI.h"
#include "DebugConsole.h"
#include "ErrorLogger.h"

class DrumSampler2Editor : public juce::AudioProcessorEditor,
                           public ErrorLogger::Listener,
                           private juce::Timer
{
public:
    explicit DrumSampler2Editor(DrumSampler2Processor&);
    ~DrumSampler2Editor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void logUpdated() override;
    
    void timerCallback() override
    {
        // Check for MIDI notes and update piece visuals
        for (int note = 0; note < 128; ++note)
        {
            if (processor.checkAndClearMidiNote(note))
            {
                kitView.triggerPieceVisual(note);
            }
        }
    }
    
private:
    void loadSamplesClicked();
    void updateStatus();
    void toggleDebugConsole();
    void updateBugsButton();
    
    DrumSampler2Processor& processor;

    juce::Label titleLabel;
    juce::Image logoImage;
    juce::TextButton loadButton{"LOAD SAMPLES"};
    juce::TextButton bugsButton{"BUGS"};
    juce::Label statusLabel;

    juce::TextButton tabKit{"KIT"};
    juce::TextButton tabGrooves{"GROOVES"};
    juce::TextButton tabMixer{"MIXER"};
    juce::TextButton tabTrigger{"TRIGGER"};

    DrumKitView kitView;
    GrooveBrowser grooveBrowser;
    MixerView mixerView;
    TriggerUI triggerUI;
    
    std::unique_ptr<DebugConsole> debugConsole;
    bool debugConsoleVisible = false;

    juce::Colour bg{0xFF1A1A1A};
    juce::Colour header{0xFF111111};
    juce::Colour accent{0xFFE8A020};
    juce::Colour textCol{0xFFCCCCCC};

    int activeTab = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumSampler2Editor)
};
