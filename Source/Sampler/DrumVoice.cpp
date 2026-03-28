#include "DrumVoice.h"
#include <cmath>

#ifndef DRUMTECH_AUDIO_DEBUG
#define DRUMTECH_AUDIO_DEBUG 0
#endif

#if !DRUMTECH_AUDIO_DEBUG
#undef DBG
#define DBG(x) do {} while (false)
#endif

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
    DBG("*** DrumVoice::trigger ENTER ***");
    currentSample = sample;
    position = 0;
    gain = gainValue;
    active = true;
    outputChannel = channel;
    
    DBG("*** DrumVoice::trigger: sampleRate=" << adsr.getSampleRate() << " ***");
    DBG("*** Sample buffer channels=" << (sample ? sample->buffer.getNumChannels() : 0) 
        << ", samples=" << (sample ? sample->buffer.getNumSamples() : 0) << " ***");
    
    juce::ADSR::Parameters params;
    params.attack = velocityCurve.attack / 1000.0f;
    params.decay = velocityCurve.decay / 1000.0f;
    params.sustain = velocityCurve.sustain;
    params.release = velocityCurve.release / 1000.0f;
    DBG("*** ADSR params: attack=" << params.attack << ", decay=" << params.decay 
        << ", sustain=" << params.sustain << ", release=" << params.release << " ***");
    adsr.setParameters(params);
    adsr.reset();
    adsr.noteOn();
    
    DBG("*** DrumVoice::trigger EXIT - active=" << active << " ***");
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
