#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "../Effects/EffectProcessor.h"
#include <array>
#include <memory>

class MixerChannel
{
public:
    static constexpr int NUM_INSERT_SLOTS = 4;
    static constexpr int NUM_SEND_SLOTS = 4;
    
    MixerChannel(int midiNote, const juce::String& name);
    
    void prepare(double sampleRate, int blockSize);
    void processAudio(juce::AudioBuffer<float>& buffer);
    
    void setGain(float gainValue) { gain = gainValue; }
    void setPan(float panValue) { pan = juce::jlimit(-1.0f, 1.0f, panValue); }
    void setPhaseInvert(bool invert) { phaseInvert = invert; }
    void setMute(bool shouldMute) { mute = shouldMute; }
    void setSolo(bool shouldSolo) { solo = shouldSolo; }
    void setOutputBus(int busIndex) { outputBus = busIndex; }
    void setSendLevel(int sendIndex, float level);
    
    void loadPlugin(int slotIndex, std::unique_ptr<EffectProcessor> effect);
    void removePlugin(int slotIndex);
    
    float getGain() const { return gain; }
    float getPan() const { return pan; }
    bool isPhaseInverted() const { return phaseInvert; }
    bool isMuted() const { return mute; }
    bool isSoloed() const { return solo; }
    int getOutputBus() const { return outputBus; }
    float getSendLevel(int sendIndex) const;
    
    int getMidiNote() const { return midiNote; }
    juce::String getName() const { return channelName; }
    
    float getPeakLevel() const { return peakLevel; }
    float getRMSLevel() const { return rmsLevel; }
    
private:
    int midiNote;
    juce::String channelName;
    
    float gain = 1.0f;
    float pan = 0.0f;
    bool phaseInvert = false;
    bool mute = false;
    bool solo = false;
    
    std::array<std::unique_ptr<EffectProcessor>, NUM_INSERT_SLOTS> insertFX;
    std::array<float, NUM_SEND_SLOTS> sendLevels;
    
    int outputBus = 0;
    
    float peakLevel = 0.0f;
    float rmsLevel = 0.0f;
    
    double sampleRate = 44100.0;
};
