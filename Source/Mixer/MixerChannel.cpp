#include "MixerChannel.h"
#include <cmath>

MixerChannel::MixerChannel(int note, const juce::String& name)
    : midiNote(note), channelName(name)
{
    sendLevels.fill(0.0f);
}

void MixerChannel::prepare(double sr, int blockSize)
{
    sampleRate = sr;
    
    for (auto& fx : insertFX)
    {
        if (fx != nullptr)
            fx->prepare(sr, blockSize);
    }
}

void MixerChannel::processAudio(juce::AudioBuffer<float>& buffer)
{
    if (mute)
    {
        buffer.clear();
        return;
    }
    
    if (phaseInvert)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.applyGain(ch, 0, buffer.getNumSamples(), -1.0f);
    }
    
    for (auto& fx : insertFX)
    {
        if (fx != nullptr)
            fx->process(buffer);
    }
    
    float leftGain = gain;
    float rightGain = gain;
    
    if (pan < 0.0f)
        rightGain *= (1.0f + pan);
    else if (pan > 0.0f)
        leftGain *= (1.0f - pan);
    
    if (buffer.getNumChannels() >= 2)
    {
        buffer.applyGain(0, 0, buffer.getNumSamples(), leftGain);
        buffer.applyGain(1, 0, buffer.getNumSamples(), rightGain);
    }
    else
    {
        buffer.applyGain(0, 0, buffer.getNumSamples(), gain);
    }
    
    peakLevel = buffer.getMagnitude(0, buffer.getNumSamples());
    rmsLevel = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
}

void MixerChannel::setSendLevel(int sendIndex, float level)
{
    if (sendIndex >= 0 && sendIndex < NUM_SEND_SLOTS)
        sendLevels[sendIndex] = level;
}

void MixerChannel::loadPlugin(int slotIndex, std::unique_ptr<EffectProcessor> effect)
{
    if (slotIndex >= 0 && slotIndex < NUM_INSERT_SLOTS)
    {
        insertFX[slotIndex] = std::move(effect);
        if (insertFX[slotIndex] != nullptr)
            insertFX[slotIndex]->prepare(sampleRate, 512);
    }
}

void MixerChannel::removePlugin(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < NUM_INSERT_SLOTS)
        insertFX[slotIndex].reset();
}

float MixerChannel::getSendLevel(int sendIndex) const
{
    if (sendIndex >= 0 && sendIndex < NUM_SEND_SLOTS)
        return sendLevels[sendIndex];
    return 0.0f;
}
