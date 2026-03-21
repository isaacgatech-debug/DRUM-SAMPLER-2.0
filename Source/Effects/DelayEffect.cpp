#include "DelayEffect.h"

DelayEffect::DelayEffect()
{
}

void DelayEffect::prepare(double sr, int blockSize)
{
    sampleRate = sr;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = static_cast<juce::uint32>(blockSize);
    spec.numChannels = 1;
    
    delayLineLeft.prepare(spec);
    delayLineRight.prepare(spec);
    
    delayLineLeft.setMaximumDelayInSamples(static_cast<int>(sr * 2.0));
    delayLineRight.setMaximumDelayInSamples(static_cast<int>(sr * 2.0));
    
    updateDelay();
}

void DelayEffect::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    
    for (int i = 0; i < numSamples; ++i)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto& delayLine = (ch == 0) ? delayLineLeft : delayLineRight;
            
            float input = buffer.getSample(ch, i);
            float delayed = delayLine.popSample(0, currentDelaySamples);
            
            delayLine.pushSample(0, input + delayed * feedback);
            
            float output = input * dryLevel + delayed * wetLevel;
            buffer.setSample(ch, i, output);
        }
    }
}

void DelayEffect::reset()
{
    delayLineLeft.reset();
    delayLineRight.reset();
}

void DelayEffect::updateDelay()
{
    if (syncToTempo)
    {
        double beatDuration = 60.0 / currentTempo;
        delayTimeMS = static_cast<float>(beatDuration * 1000.0 * 0.25);
    }
    
    currentDelaySamples = static_cast<int>((delayTimeMS / 1000.0) * sampleRate);
    currentDelaySamples = juce::jlimit(1, static_cast<int>(sampleRate * 2.0), currentDelaySamples);
}
