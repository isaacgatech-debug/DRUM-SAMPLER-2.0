#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class KitView : public juce::Component
{
public:
    KitView();
    void paint (juce::Graphics&) override;
    void setFlash (int midiNote);

private:
    struct Zone { const char* name; int note; float x, y, w, h; bool isCymbal; };
    static const Zone zones[];
    static const int  numZones;

    int   flashNote  = -1;
    float flashAlpha = 0.0f;
    juce::Colour accentColour { 0xFFE8A020 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KitView)
};

class DrumLibraryEditor : public juce::AudioProcessorEditor,
                           private juce::Timer
{
public:
    explicit DrumLibraryEditor (DrumLibraryProcessor&);
    ~DrumLibraryEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void loadSamplesClicked();
    void updateStatus();

    DrumLibraryProcessor& processor;

    // Header
    juce::Label  titleLabel;
    juce::TextButton loadButton   { "LOAD SAMPLES" };
    juce::Label  statusLabel;

    // Tab bar
    juce::TextButton tabKit     { "KIT" };
    juce::TextButton tabMixer   { "MIXER" };
    juce::TextButton tabFX      { "FX" };

    // Pages
    KitView kitView;

    // Colours
    juce::Colour bg       { 0xFF1A1A1A };
    juce::Colour header   { 0xFF111111 };
    juce::Colour accent   { 0xFFE8A020 };
    juce::Colour textCol  { 0xFFCCCCCC };

    int activeTab = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumLibraryEditor)
};
