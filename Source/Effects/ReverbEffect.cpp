#include "ReverbEffect.h"

ReverbEffect::ReverbEffect()
{
}

void ReverbEffect::prepare(double sampleRate, int blockSize)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(blockSize);
    spec.numChannels = 2;
    reverb.prepare(spec);
}

void ReverbEffect::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}

void ReverbEffect::reset()
{
    reverb.reset();
}
