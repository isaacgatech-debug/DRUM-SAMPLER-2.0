#include "SettingsView.h"

SettingsView::SettingsView()
{
    titleLabel.setText("Kit & Import Settings", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    addAndMakeVisible(titleLabel);

    kitNameLabel.setText("Current Kit: Default", juce::dontSendNotification);
    kitNameLabel.setFont(PluginFonts::mono(13.0f));
    kitNameLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::accent));
    addAndMakeVisible(kitNameLabel);

    importStatusLabel.setText("Import status: idle", juce::dontSendNotification);
    importStatusLabel.setFont(PluginFonts::mono(12.0f));
    importStatusLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textMuted));
    addAndMakeVisible(importStatusLabel);

    drummerLabel.setText("Drummer Profile", juce::dontSendNotification);
    drummerLabel.setFont(PluginFonts::label(12.0f));
    drummerLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    addAndMakeVisible(drummerLabel);

    drummerProfileBox.addItem("Default", 1);
    drummerProfileBox.addItem("DrummerA", 2);
    drummerProfileBox.addItem("DrummerB", 3);
    drummerProfileBox.setSelectedId(1);
    drummerProfileBox.onChange = [this]
    {
        if (onDrummerProfileChanged) onDrummerProfileChanged(drummerProfileBox.getSelectedItemIndex());
    };
    addAndMakeVisible(drummerProfileBox);

    styleLabel.setText("Playing Style", juce::dontSendNotification);
    styleLabel.setFont(PluginFonts::label(12.0f));
    styleLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    addAndMakeVisible(styleLabel);

    playingStyleBox.addItem("Auto", 1);
    playingStyleBox.addItem("Sticks", 2);
    playingStyleBox.addItem("Brushes", 3);
    playingStyleBox.setSelectedId(1);
    playingStyleBox.onChange = [this]
    {
        if (onPlayingStyleChanged) onPlayingStyleChanged(playingStyleBox.getSelectedItemIndex());
    };
    addAndMakeVisible(playingStyleBox);

    importBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(PluginColors::pluginSurface));
    importBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textPrimary));
    importBtn.onClick = [this]
    {
        if (onImportFolder) onImportFolder();
    };
    addAndMakeVisible(importBtn);

    createKitBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(PluginColors::pluginSurface));
    createKitBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textPrimary));
    createKitBtn.onClick = [this]
    {
        if (onCreateKit) onCreateKit();
    };
    addAndMakeVisible(createKitBtn);
}

void SettingsView::setCurrentKitName(const juce::String& kitName)
{
    kitNameLabel.setText("Current Kit: " + kitName, juce::dontSendNotification);
}

void SettingsView::setImportStatus(const juce::String& text)
{
    importStatusLabel.setText("Import status: " + text, juce::dontSendNotification);
}

void SettingsView::setDrummerProfileIndex(int index)
{
    drummerProfileBox.setSelectedItemIndex(index, juce::dontSendNotification);
}

void SettingsView::setPlayingStyleIndex(int index)
{
    playingStyleBox.setSelectedItemIndex(index, juce::dontSendNotification);
}

void SettingsView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(PluginColors::pluginBg));

    auto card = getLocalBounds().reduced(30).toFloat();
    g.setColour(juce::Colour(PluginColors::pluginPanel));
    g.fillRoundedRectangle(card, 6.0f);

    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawRoundedRectangle(card, 6.0f, 1.0f);
}

void SettingsView::resized()
{
    auto area = getLocalBounds().reduced(48, 42);
    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(10);
    kitNameLabel.setBounds(area.removeFromTop(24));
    importStatusLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(14);
    drummerLabel.setBounds(area.removeFromTop(22).removeFromLeft(160));
    drummerProfileBox.setBounds(area.removeFromTop(28).removeFromLeft(220));
    area.removeFromTop(8);
    styleLabel.setBounds(area.removeFromTop(22).removeFromLeft(160));
    playingStyleBox.setBounds(area.removeFromTop(28).removeFromLeft(220));
    area.removeFromTop(14);
    importBtn.setBounds(area.removeFromTop(34).removeFromLeft(300));
    area.removeFromTop(8);
    createKitBtn.setBounds(area.removeFromTop(34).removeFromLeft(300));
}
