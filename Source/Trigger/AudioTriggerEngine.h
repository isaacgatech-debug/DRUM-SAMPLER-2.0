#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "OnsetDetector.h"
#include "DrumClassifier.h"
#include <vector>

struct TriggerResult
{
    int midiNote;
    int velocity;
    double timeInSamples;
    DrumType drumType;
    float confidence;
};

class AudioTriggerEngine
{
public:
    AudioTriggerEngine();
    
    void prepare(double sampleRate, int blockSize);
    void processAudioFile(const juce::File& audioFile, std::vector<TriggerResult>& results);
    void processBlock(const juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiOut);
    
    void setThreshold(float threshold);
    void setBleedSuppression(float amount);
    void setSensitivity(float sensitivity);
    void setMinTimeBetweenHits(double timeMs);
    
    void enableRealTimeMode(bool enable) { realTimeMode = enable; }
    bool isRealTimeMode() const { return realTimeMode; }
    
    float getThreshold() const { return threshold; }
    float getBleedSuppression() const { return bleedSuppression; }
    
    void exportToMIDI(const std::vector<TriggerResult>& results, const juce::File& outputFile);
    
private:
    void applyBleedSuppression(std::vector<OnsetEvent>& onsets);
    juce::AudioBuffer<float> extractSegment(const juce::AudioBuffer<float>& source, 
                                            int startSample, int length);
    
    OnsetDetector onsetDetector;
    DrumClassifier drumClassifier;
    
    float threshold = 0.3f;
    float bleedSuppression = 0.5f;
    float sensitivity = 1.0f;
    double minTimeBetweenHits = 50.0;
    
    bool realTimeMode = false;
    double sampleRate = 44100.0;
    
    std::vector<OnsetEvent> recentOnsets;
    double currentTime = 0.0;
};
