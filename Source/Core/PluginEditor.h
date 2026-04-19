#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include "PluginProcessor.h"
#include "../UI/DrumKitView.h"
#include "../UI/MixerView.h"
#include "../UI/TransportBar.h"
#include "../Sequencer/StepSequencer.h"
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

        juce::ignoreUnused(processor);

        // Animate mixer slide
        const float mixerDelta = mixerSlideTarget - mixerSlideAmount;
        if (std::abs(mixerDelta) > 0.001f)
        {
            mixerSlideAmount += mixerDelta * 0.2f;
            if (std::abs(mixerSlideTarget - mixerSlideAmount) < 0.005f)
                mixerSlideAmount = mixerSlideTarget;
            resized();
        }
        
        // Animate kit builder slide
        const float kitBuilderDelta = kitBuilderSlideTarget - kitBuilderSlideAmount;
        if (std::abs(kitBuilderDelta) > 0.001f)
        {
            kitBuilderSlideAmount += kitBuilderDelta * 0.2f;
            if (std::abs(kitBuilderSlideTarget - kitBuilderSlideAmount) < 0.005f)
                kitBuilderSlideAmount = kitBuilderSlideTarget;
            kitView.setKitBuilderSlideAmount(kitBuilderSlideAmount);
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
    
    // Icon drawables (must be stored as members)
    std::unique_ptr<juce::Drawable> homeIcon;
    std::unique_ptr<juce::Drawable> kitIcon;
    std::unique_ptr<juce::Drawable> mixerIcon;
    std::unique_ptr<juce::Drawable> sequencerIcon;

    // Tab buttons (icon-only, using DrawableButton for SVG support)
    juce::DrawableButton tabHome       {"tabHome", juce::DrawableButton::ImageFitted};
    juce::DrawableButton tabKit       {"tabKit", juce::DrawableButton::ImageFitted};
    juce::DrawableButton tabMixer     {"tabMixer", juce::DrawableButton::ImageFitted};
    juce::DrawableButton tabSequencer {"tabSequencer", juce::DrawableButton::ImageFitted};

    // Top-right controls
    juce::TextButton kitSelectorBtn{"Kit: Default"};
    juce::TextButton kitLockBtn   {"LOCK: OFF"};
    juce::TextButton themeModeBtn  {"LIGHT"};

    // Content views
    DrumKitView   kitView;
    MixerView     mixerView;
    MixerDismissLayer mixerDismissLayer;
    StepSequencer stepSequencer;

    // Transport bar
    TransportBar   transportBar;

    // Debug console
    std::unique_ptr<DebugConsole> debugConsole;
    bool debugConsoleVisible = false;

    int          activeTab     = 0;
    bool         kitLocked     = false;
    juce::String currentKitName{"Default Kit"};
    float        mixerSlideAmount = 0.0f;
    float        mixerSlideTarget = 0.0f;
    float        kitBuilderSlideAmount = 0.0f;
    float        kitBuilderSlideTarget = 0.0f;

    static constexpr int topNavH       = 54;
    static constexpr int instrBarH     = 44;
    static constexpr int transportBarH = 56;

    DrumTechLookAndFeel appLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumTechEditor)
};
