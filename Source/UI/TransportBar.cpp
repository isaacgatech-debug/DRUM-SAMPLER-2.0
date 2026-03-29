#include "TransportBar.h"

TransportBar::TransportBar()
{
    auto styleTB = [](juce::TextButton& btn, juce::Colour offCol, juce::Colour onCol)
    {
        btn.setColour(juce::TextButton::buttonColourId,  offCol);
        btn.setColour(juce::TextButton::buttonOnColourId, onCol);
        btn.setColour(juce::TextButton::textColourOffId,  juce::Colour(PluginColors::textPrimary));
        btn.setColour(juce::TextButton::textColourOnId,   juce::Colours::black);
    };

    juce::Colour surfCol(PluginColors::pluginSurface);
    styleTB(loopButton,   surfCol, juce::Colour(PluginColors::accent));
    styleTB(stopButton,   surfCol, juce::Colour(PluginColors::meterRed));
    styleTB(playButton,   surfCol, juce::Colour(PluginColors::accent));
    styleTB(recordButton, surfCol, juce::Colour(PluginColors::meterRed));
    styleTB(tapButton,    surfCol, juce::Colour(PluginColors::pluginBorderHi));

    addAndMakeVisible(loopButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(recordButton);
    addAndMakeVisible(tapButton);

    loopButton.setClickingTogglesState(true);
    loopButton.onClick = [this] {
        isLooping = loopButton.getToggleState();
        if (onLoop) onLoop(isLooping);
    };

    stopButton.onClick = [this] {
        playButton.setToggleState(false, juce::dontSendNotification);
        isPlaying = false;
        playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(PluginColors::pluginSurface));
        if (onStop) onStop();
    };

    playButton.setClickingTogglesState(true);
    playButton.onClick = [this] {
        isPlaying = playButton.getToggleState();
        playButton.setColour(juce::TextButton::buttonColourId,
            isPlaying ? juce::Colour(PluginColors::accent) : juce::Colour(PluginColors::pluginSurface));
        if (onPlay) onPlay(isPlaying);
    };

    recordButton.setClickingTogglesState(true);
    recordButton.onClick = [this] {
        isRecording = recordButton.getToggleState();
        recordButton.setColour(juce::TextButton::buttonColourId,
            isRecording ? juce::Colour(PluginColors::meterRed) : juce::Colour(PluginColors::pluginSurface));
        if (onRecord) onRecord(isRecording);
    };

    tapButton.onClick = [this] { if (onTap) onTap(); };

    timeSigLabel.setText("4/4", juce::dontSendNotification);
    timeSigLabel.setFont(PluginFonts::mono(15.0f));
    timeSigLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    timeSigLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(timeSigLabel);

    bpmLabel.setText("120", juce::dontSendNotification);
    bpmLabel.setFont(PluginFonts::mono(17.0f));
    bpmLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::accent));
    bpmLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bpmLabel);

    positionLabel.setText("001:01:000", juce::dontSendNotification);
    positionLabel.setFont(PluginFonts::mono(14.5f));
    positionLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    positionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(positionLabel);

    masterVolKnob.setRange(0.0, 1.0);
    masterVolKnob.setValue(0.8);
    masterVolKnob.setKnobColor(juce::Colour(PluginColors::accent));
    masterVolKnob.setLabel("VOL");
    masterVolKnob.onValueChange = [this]
    {
        if (onMasterVolume)
            onMasterVolume(static_cast<float>(masterVolKnob.getValue()));
    };
    addAndMakeVisible(masterVolKnob);
}

void TransportBar::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(PluginColors::pluginPanel));
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));
}

void TransportBar::resized()
{
    auto area = getLocalBounds().reduced(8, 6);
    loopButton.setBounds  (area.removeFromLeft(56).reduced(2));
    stopButton.setBounds  (area.removeFromLeft(56).reduced(2));
    playButton.setBounds  (area.removeFromLeft(56).reduced(2));
    recordButton.setBounds(area.removeFromLeft(50).reduced(2));
    tapButton.setBounds   (area.removeFromLeft(56).reduced(2));
    area.removeFromLeft(12);
    timeSigLabel.setBounds(area.removeFromLeft(44));
    bpmLabel.setBounds    (area.removeFromLeft(58));
    area.removeFromLeft(12);
    positionLabel.setBounds(area.removeFromLeft(118));
    masterVolKnob.setBounds(area.removeFromRight(64).reduced(2));
}
