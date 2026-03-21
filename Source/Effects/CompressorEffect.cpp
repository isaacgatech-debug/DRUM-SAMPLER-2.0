#include "CompressorEffect.h"

CompressorEffect::CompressorEffect()
{
}

void CompressorEffect::prepare(double sampleRate, int blockSize)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(blockSize);
    spec.numChannels = 2;
    compressor.prepare(spec);
}

void CompressorEffect::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressor.process(context);
}

void CompressorEffect::reset()
{
    compressor.reset();
}
