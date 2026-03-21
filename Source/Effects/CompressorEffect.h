#pragma once
#include "EffectProcessor.h"
#include <juce_dsp/juce_dsp.h>

class CompressorEffect : public EffectProcessor
{
public:
    CompressorEffect();
    
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    
    juce::String getName() const override { return "Compressor"; }
    bool hasEditor() const override { return false; }
    juce::Component* createEditor() override { return nullptr; }
    
private:
    juce::dsp::Compressor<float> compressor;
};
