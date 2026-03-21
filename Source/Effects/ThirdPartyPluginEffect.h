#pragma once
#include "EffectProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

class ThirdPartyPluginEffect : public EffectProcessor
{
public:
    ThirdPartyPluginEffect(std::unique_ptr<juce::AudioPluginInstance> instance);
    
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    
    juce::String getName() const override;
    bool hasEditor() const override;
    juce::Component* createEditor() override;
    bool isThirdParty() const override { return true; }
    
    juce::AudioPluginInstance* getPluginInstance() { return plugin.get(); }
    
private:
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};
