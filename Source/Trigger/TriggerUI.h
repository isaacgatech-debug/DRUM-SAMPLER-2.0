#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AudioTriggerEngine.h"
#include "DrumClassifier.h"
#include <vector>

class TriggerUI : public juce::Component,
                  public juce::FileDragAndDropTarget
{
public:
    TriggerUI();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    
    void setAudioTriggerEngine(AudioTriggerEngine* engine) { triggerEngine = engine; }
    void loadAudioFile(const juce::File& file);
    void processCurrentFile();
    void setTargetDrumType(DrumType type) { targetDrumType = type; }
    
    void setThreshold(float threshold);
    void setBleedSuppression(float amount);
    
private:
    void drawWaveform(juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawTriggerMarkers(juce::Graphics& g, const juce::Rectangle<int>& area);
    void showDrumTypeSelector(const juce::File& file);
    juce::String formatDuration(double samples, double sampleRate);
    
    AudioTriggerEngine* triggerEngine = nullptr;
    DrumType targetDrumType = DrumType::Unknown;
    
    juce::AudioBuffer<float> audioBuffer;
    std::vector<TriggerResult> triggerResults;
    double audioSampleRate = 44100.0;
    
    juce::TextButton loadButton{"Load Audio"};
    juce::TextButton processButton{"Process"};
    juce::TextButton exportButton{"Export MIDI"};
    
    juce::Slider thresholdSlider;
    juce::Label thresholdLabel;
    
    juce::Slider bleedSlider;
    juce::Label bleedLabel;
    
    juce::Label statusLabel;
    juce::Label fileInfoLabel;
    
    bool isDragOver = false;
    
    juce::Colour bgColour{0xFF2A2A2A};
    juce::Colour waveformColour{0xFF4A9EFF};
    juce::Colour triggerColour{0xFFE8A020};
    juce::Colour dragHighlightColour{0xFF6B9EFF};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerUI)
};
