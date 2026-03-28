#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AudioTriggerEngine.h"
#include "DrumClassifier.h"
#include "../UI/FrequencyBandEditor.h"
#include "../UI/PluginColors.h"
#include <vector>

/**
 * Per-channel trigger view — HORIZONTAL stacked rows.
 * Each TriggerChannel is a full-width row (100px tall):
 *   130px left panel: name + Import/Clear/Play buttons
 *   (fills)  center: waveform spanning full width
 *   Threshold slider below waveform inside the row
 *
 * Rows scroll vertically via a Viewport.
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

    void setAudioTriggerEngine(AudioTriggerEngine* engine);

private:
    static const char* micNames[8];

    /**
     * Full-width horizontal channel row.
     * Left panel (130px): name + buttons.
     * Center: waveform fills remaining width.
     * Below waveform: threshold + freq editor (collapsed by default).
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
        bool isExpanded() const { return freqEditorVisible; }

        bool isInterestedInFileDrag(const juce::StringArray& files) override;
        void filesDropped  (const juce::StringArray& files, int x, int y) override;
        void fileDragEnter (const juce::StringArray& files, int x, int y) override;
        void fileDragExit  (const juce::StringArray& files) override;

        static constexpr int rowH      = 110;  // collapsed row height
        static constexpr int expandedH = 260;  // row height with freq editor
        static constexpr int leftW     = 130;  // left button panel width

    private:
        void drawLeftPanel  (juce::Graphics& g, juce::Rectangle<int> area);
        void drawWaveform   (juce::Graphics& g, juce::Rectangle<int> area);
        void drawChannelInfo(juce::Graphics& g, juce::Rectangle<int> area);

        juce::String channelName;
        AudioTriggerEngine* triggerEngine = nullptr;

        juce::TextButton importBtn{"Import"};
        juce::TextButton clearBtn {"Clear"};
        juce::TextButton playBtn  {"▶"};
        juce::TextButton expandBtn{"∨ EQ"};

        juce::Slider thresholdSlider;
        juce::Label  thresholdLabel;
        juce::Label  filenameLabel;

        FrequencyBandEditor freqEditor;
        bool freqEditorVisible = false;

        juce::AudioBuffer<float> audioBuffer;
        double audioSampleRate = 44100.0;
        bool   isDragOver      = false;
        bool   isPlaying       = false;
        std::vector<TriggerResult> triggerPreviewResults;

        // Channel color (by index)
        juce::Colour chColor{PluginColors::accent};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerChannel)
    };

    AudioTriggerEngine* triggerEngine = nullptr;

    std::array<std::unique_ptr<TriggerChannel>, 8> channels;
    juce::Viewport scrollView;

    struct ChannelsContainer : public juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(PluginColors::pluginBg));
        }
    };
    ChannelsContainer channelsContainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerUI)
};
