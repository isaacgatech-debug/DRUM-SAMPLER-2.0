#include "SendFXBus.h"

SendFXBus::SendFXBus(int index)
    : sendIndex(index)
{
    busName = "Send " + juce::String(index + 1);
}

void SendFXBus::prepare(double sampleRate, int blockSize)
{
    inputBuffer.setSize(2, blockSize);
    outputBuffer.setSize(2, blockSize);
    
    if (effect != nullptr)
    {
        effect->prepare(sampleRate, blockSize);
    }
}

void SendFXBus::processSend(const std::vector<juce::AudioBuffer<float>*>& channelBuffers,
                            const std::vector<float>& sendLevels)
{
    inputBuffer.clear();
    
    for (size_t i = 0; i < channelBuffers.size() && i < sendLevels.size(); ++i)
    {
        if (channelBuffers[i] != nullptr && sendLevels[i] > 0.0f)
        {
            for (int ch = 0; ch < juce::jmin(2, channelBuffers[i]->getNumChannels()); ++ch)
            {
                inputBuffer.addFrom(ch, 0, *channelBuffers[i], ch, 0,
                                   inputBuffer.getNumSamples(), sendLevels[i]);
            }
        }
    }
    
    outputBuffer.makeCopyOf(inputBuffer);
    
    if (effect != nullptr)
    {
        effect->process(outputBuffer);
    }
    
    outputBuffer.applyGain(returnLevel);
}

void SendFXBus::clear()
{
    inputBuffer.clear();
    outputBuffer.clear();
}

void SendFXBus::setEffect(std::unique_ptr<EffectProcessor> newEffect)
{
    effect = std::move(newEffect);
    if (effect != nullptr && inputBuffer.getNumSamples() > 0)
    {
        effect->prepare(44100.0, inputBuffer.getNumSamples());
    }
}

void SendFXBus::removeEffect()
{
    effect.reset();
}
