#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginColors.h"
#include "AnalogKnob.h"

/**
 * Bottom transport strip — height set by DrumTechEditor (transportBarH).
 * Text labels avoid Unicode font issues: LOOP STOP PLAY REC TAP.
 */
class TransportBar : public juce::Component
{
public:
    TransportBar();

    void paint  (juce::Graphics& g) override;
    void resized() override;

    void setPositionLabel(const juce::String& pos)
    {
        positionLabel.setText(pos, juce::dontSendNotification);
    }

    void setBPM(float bpm)
    {
        bpmLabel.setText(juce::String(static_cast<int>(bpm)), juce::dontSendNotification);
    }

    void setPlaying(bool playing)
    {
        isPlaying = playing;
        playButton.setToggleState(playing, juce::dontSendNotification);
        playButton.setColour(juce::TextButton::buttonColourId,
                             playing ? juce::Colour(PluginColors::accent)
                                     : juce::Colour(PluginColors::pluginSurface));
        repaint();
    }

    // Callbacks
    std::function<void(bool)>  onPlay;
    std::function<void()>      onStop;
    std::function<void(bool)>  onLoop;
    std::function<void(bool)>  onRecord;
    std::function<void()>      onTap;
    std::function<void(float)> onMasterVolume;

private:
    juce::TextButton loopButton  {"LOOP"};
    juce::TextButton stopButton  {"STOP"};
    juce::TextButton playButton  {"PLAY"};
    juce::TextButton recordButton{"REC"};
    juce::TextButton tapButton   {"TAP"};

    juce::Label timeSigLabel;
    juce::Label bpmLabel;
    juce::Label positionLabel;

    AnalogKnob masterVolKnob{AnalogKnob::Style::Standard};

    bool isPlaying  = false;
    bool isLooping  = false;
    bool isRecording= false;

    void styleTransportButton(juce::TextButton& btn, juce::Colour activeColor);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};
