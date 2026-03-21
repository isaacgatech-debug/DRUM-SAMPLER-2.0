#include "EQEffect.h"

EQEffect::EQEffect()
{
}

void EQEffect::prepare(double sr, int blockSize)
{
    sampleRate = sr;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = static_cast<juce::uint32>(blockSize);
    spec.numChannels = 2;
    
    filterChain.prepare(spec);
    updateFilters();
}

void EQEffect::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filterChain.process(context);
}

void EQEffect::reset()
{
    filterChain.reset();
}

void EQEffect::updateFilters()
{
    auto lowShelfCoeffs = Coefficients::makeLowShelf(sampleRate, lowBand.frequency, lowBand.Q, 
        juce::Decibels::decibelsToGain(lowBand.gain));
    *filterChain.get<0>().coefficients = *lowShelfCoeffs;
    
    auto peakCoeffs = Coefficients::makePeakFilter(sampleRate, midBand.frequency, midBand.Q,
        juce::Decibels::decibelsToGain(midBand.gain));
    *filterChain.get<1>().coefficients = *peakCoeffs;
    
    auto highShelfCoeffs = Coefficients::makeHighShelf(sampleRate, highBand.frequency, highBand.Q,
        juce::Decibels::decibelsToGain(highBand.gain));
    *filterChain.get<2>().coefficients = *highShelfCoeffs;
}
