#include "DrumVoice.h"
#include <cmath>

DrumVoice::DrumVoice()
{
    juce::ADSR::Parameters params;
    params.attack = 0.005f;
    params.decay = 0.15f;
    params.sustain = 0.8f;
    params.release = 0.2f;
    adsr.setParameters(params);
}

void DrumVoice::trigger(const DrumSample* sample, float gainValue, int channel)
{
    currentSample = sample;
    position = 0;
    gain = gainValue;
    active = true;
    outputChannel = channel;
    
    juce::ADSR::Parameters params;
    params.attack = velocityCurve.attack / 1000.0f;
    params.decay = velocityCurve.decay / 1000.0f;
    params.sustain = velocityCurve.sustain;
    params.release = velocityCurve.release / 1000.0f;
    adsr.setParameters(params);
    adsr.noteOn();
    
    DBG("Voice triggered - gain: " << gain << ", samples: " << sample->buffer.getNumSamples() 
        << ", channels: " << sample->buffer.getNumChannels());
}

void DrumVoice::process(juce::AudioBuffer<float>& output, int startSample, int numSamples)
{
    if (!active || currentSample == nullptr)
        return;
    
    static int debugCounter = 0;
    if (debugCounter++ % 100 == 0)
    {
        DBG("Voice processing - active: " << active << ", position: " << position 
            << ", gain: " << gain << ", output channels: " << output.getNumChannels());
    }
    
    int remaining = currentSample->buffer.getNumSamples() - position;
    int toRender = juce::jmin(numSamples, remaining);
    
    for (int i = 0; i < toRender; ++i)
    {
        float envelope = adsr.getNextSample();
        
        int srcPos = position + static_cast<int>(i * pitchRatio);
        if (srcPos >= currentSample->buffer.getNumSamples())
        {
            active = false;
            break;
        }
        
        for (int ch = 0; ch < output.getNumChannels(); ++ch)
        {
            int srcCh = ch % currentSample->buffer.getNumChannels();
            float sample = currentSample->buffer.getSample(srcCh, srcPos);
            output.addSample(ch, startSample + i, sample * gain * envelope);
        }
    }
    
    position += static_cast<int>(toRender * pitchRatio);
    
    if (position >= currentSample->buffer.getNumSamples())
        active = false;
}

void DrumVoice::setPitch(float semitones)
{
    pitchRatio = std::pow(2.0f, semitones / 12.0f);
}

void DrumVoice::setVelocityCurve(const VelocityCurve& curve)
{
    velocityCurve = curve;
}

void DrumVoice::stop()
{
    adsr.noteOff();
}
