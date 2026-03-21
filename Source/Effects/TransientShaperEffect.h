#pragma once
#include "EffectProcessor.h"
#include <vector>

class TransientShaperEffect : public EffectProcessor
{
public:
    TransientShaperEffect();
    
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    
    juce::String getName() const override { return "Transient Shaper"; }
    bool hasEditor() const override { return false; }
    juce::Component* createEditor() override { return nullptr; }
    
    void setAttack(float value) { attack = value; }
    void setSustain(float value) { sustain = value; }
    
private:
    float attack = 0.0f;
    float sustain = 0.0f;
    float attackTime = 5.0f;
    float releaseTime = 50.0f;
    float mix = 1.0f;
    
    std::vector<float> attackEnvelope;
    std::vector<float> sustainEnvelope;
    float attackDecay = 0.99f;
    float sustainAttack = 0.01f;
    float sustainDecay = 0.999f;
};
