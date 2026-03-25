#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AudioTriggerEngine.h"
#include "DrumClassifier.h"
#include "../UI/FrequencyBandEditor.h"
#include "../UI/PluginColors.h"
#include <vector>

/**
 * Per-channel trigger grid.
 * 8 TriggerChannel cards (2-3 per row), each with:
 *   Header (mic name, Import Audio, Clear) |
 *   72px waveform area (drag/drop zone)    |
 *   Playback transport (play/pause + filename) |
 *   Volume Threshold slider               |
 *   FrequencyBandEditor                   |
 */
class TriggerUI : public juce::Component,
                  public juce::FileDragAndDropTarget
{
public:
    TriggerUI();

    void paint  (juce::Graphics& g) override;
    void resized() override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped  (const juce::StringArray& files, int x, int y) override;
    void fileDragEnter (const juce::StringArray& files, int x, int y) override;
    void fileDragExit  (const juce::StringArray& files) override;

    void setAudioTriggerEngine(AudioTriggerEngine* engine) { triggerEngine = engine; }

private:
    static const char* micNames[8];

    /**
     * Single trigger channel card.
     */
    class TriggerChannel : public juce::Component,
                           public juce::FileDragAndDropTarget
    {
    public:
        explicit TriggerChannel(const juce::String& micName);

        void paint  (juce::Graphics& g) override;
        void resized() override;

        void loadAudioFile(const juce::File& file);
        void setTriggerEngine(AudioTriggerEngine* engine) { triggerEngine = engine; }

        bool isInterestedInFileDrag(const juce::StringArray& files) override;
        void filesDropped  (const juce::StringArray& files, int x, int y) override;
        void fileDragEnter (const juce::StringArray& files, int x, int y) override;
        void fileDragExit  (const juce::StringArray& files) override;

    private:
        void drawWaveform(juce::Graphics& g, juce::Rectangle<int> area);

        juce::String channelName;
        AudioTriggerEngine* triggerEngine = nullptr;

        juce::TextButton importBtn{"Import Audio"};
        juce::TextButton clearBtn {"Clear"};
        juce::TextButton playBtn  {"▶"};

        juce::Slider thresholdSlider;
        juce::Label  thresholdLabel;
        juce::Label  filenameLabel;

        FrequencyBandEditor freqEditor;

        juce::AudioBuffer<float> audioBuffer;
        double audioSampleRate = 44100.0;
        bool   isDragOver      = false;
        bool   isPlaying       = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerChannel)
    };

    AudioTriggerEngine* triggerEngine = nullptr;

    std::array<std::unique_ptr<TriggerChannel>, 8> channels;
    juce::Viewport scrollView;

    struct ChannelsContainer : public juce::Component
    {
        void paint(juce::Graphics& g) override { g.fillAll(juce::Colour(PluginColors::pluginBg)); }
    };
    ChannelsContainer channelsContainer;

    bool isDragOver = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerUI)
};
