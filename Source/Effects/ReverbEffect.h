#pragma once
#include "EffectProcessor.h"
#include <juce_dsp/juce_dsp.h>

class ReverbEffect : public EffectProcessor
{
public:
    ReverbEffect();
    
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    
    juce::String getName() const override { return "Reverb"; }
    bool hasEditor() const override { return false; }
    juce::Component* createEditor() override { return nullptr; }
    
private:
    juce::dsp::Reverb reverb;
};
