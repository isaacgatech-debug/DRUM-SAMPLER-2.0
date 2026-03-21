#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

class AudioBus
{
public:
    enum class BusType {
        Mono,
        Stereo,
        Linked
    };
    
    AudioBus(int index = 0);
    
    void prepare(double sampleRate, int blockSize);
    void processAudio(juce::AudioBuffer<float>& input);
    void clear();
    
    void setType(BusType type) { busType = type; }
    void setName(const juce::String& name) { busName = name; }
    void setGain(float gainValue) { gain = gainValue; }
    void setMute(bool shouldMute) { mute = shouldMute; }
    void linkToDAW(bool shouldLink, int dawIndex);
    
    BusType getType() const { return busType; }
    juce::String getName() const { return busName; }
    float getGain() const { return gain; }
    bool isMuted() const { return mute; }
    bool isLinkedToDAW() const { return isLinkedToDAWFlag; }
    int getDAWOutputIndex() const { return dawOutputIndex; }
    int getBusIndex() const { return busIndex; }
    
    juce::AudioBuffer<float>& getBuffer() { return buffer; }
    
private:
    int busIndex;
    juce::String busName;
    BusType busType = BusType::Stereo;
    
    juce::AudioBuffer<float> buffer;
    float gain = 1.0f;
    bool mute = false;
    
    bool isLinkedToDAWFlag = false;
    int dawOutputIndex = 0;
};
