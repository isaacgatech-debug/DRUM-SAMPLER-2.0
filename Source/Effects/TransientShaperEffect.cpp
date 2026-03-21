#include "TransientShaperEffect.h"
#include <cmath>

TransientShaperEffect::TransientShaperEffect()
{
}

void TransientShaperEffect::prepare(double sampleRate, int blockSize)
{
    attackDecay = std::exp(-1.0f / (attackTime * 0.001f * sampleRate));
    sustainAttack = 1.0f - std::exp(-1.0f / (attackTime * 0.001f * sampleRate));
    sustainDecay = std::exp(-1.0f / (releaseTime * 0.001f * sampleRate));
    
    attackEnvelope.resize(2, 0.0f);
    sustainEnvelope.resize(2, 0.0f);
}

void TransientShaperEffect::process(juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float input = channelData[i];
            float absInput = std::abs(input);
            
            if (absInput > attackEnvelope[ch])
                attackEnvelope[ch] = absInput;
            else
                attackEnvelope[ch] *= attackDecay;
            
            if (absInput > sustainEnvelope[ch])
                sustainEnvelope[ch] += (absInput - sustainEnvelope[ch]) * sustainAttack;
            else
                sustainEnvelope[ch] *= sustainDecay;
            
            float transientGain = 1.0f + (attack / 100.0f) * attackEnvelope[ch];
            float sustainGain = 1.0f + (sustain / 100.0f) * sustainEnvelope[ch];
            
            float shaped = input * transientGain * sustainGain;
            channelData[i] = input * (1.0f - mix) + shaped * mix;
        }
    }
}

void TransientShaperEffect::reset()
{
    for (auto& env : attackEnvelope)
        env = 0.0f;
    for (auto& env : sustainEnvelope)
        env = 0.0f;
}
