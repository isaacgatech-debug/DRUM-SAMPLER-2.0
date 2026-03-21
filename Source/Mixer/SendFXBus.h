#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "../Effects/EffectProcessor.h"
#include <memory>

class SendFXBus
{
public:
    SendFXBus(int index);
    
    void prepare(double sampleRate, int blockSize);
    void processSend(const std::vector<juce::AudioBuffer<float>*>& channelBuffers,
                     const std::vector<float>& sendLevels);
    void clear();
    
    void setEffect(std::unique_ptr<EffectProcessor> effect);
    void removeEffect();
    
    void setReturnLevel(float level) { returnLevel = level; }
    void setOutputBus(int busIndex) { outputBus = busIndex; }
    void setName(const juce::String& name) { busName = name; }
    
    float getReturnLevel() const { return returnLevel; }
    int getOutputBus() const { return outputBus; }
    juce::String getName() const { return busName; }
    int getIndex() const { return sendIndex; }
    
    juce::AudioBuffer<float>& getOutputBuffer() { return outputBuffer; }
    EffectProcessor* getEffect() { return effect.get(); }
    
private:
    int sendIndex;
    juce::String busName;
    
    std::unique_ptr<EffectProcessor> effect;
    juce::AudioBuffer<float> inputBuffer;
    juce::AudioBuffer<float> outputBuffer;
    
    float returnLevel = 1.0f;
    int outputBus = 0;
};
