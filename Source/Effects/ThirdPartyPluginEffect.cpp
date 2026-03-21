#include "ThirdPartyPluginEffect.h"

ThirdPartyPluginEffect::ThirdPartyPluginEffect(std::unique_ptr<juce::AudioPluginInstance> instance)
    : plugin(std::move(instance))
{
}

void ThirdPartyPluginEffect::prepare(double sampleRate, int blockSize)
{
    currentSampleRate = sampleRate;
    currentBlockSize = blockSize;
    
    if (plugin != nullptr)
    {
        plugin->prepareToPlay(sampleRate, blockSize);
    }
}

void ThirdPartyPluginEffect::process(juce::AudioBuffer<float>& buffer)
{
    if (plugin == nullptr)
        return;
    
    juce::MidiBuffer emptyMidi;
    plugin->processBlock(buffer, emptyMidi);
}

void ThirdPartyPluginEffect::reset()
{
    if (plugin != nullptr)
    {
        plugin->reset();
    }
}

juce::String ThirdPartyPluginEffect::getName() const
{
    if (plugin != nullptr)
        return plugin->getName();
    return "Unknown Plugin";
}

bool ThirdPartyPluginEffect::hasEditor() const
{
    return plugin != nullptr && plugin->hasEditor();
}

juce::Component* ThirdPartyPluginEffect::createEditor()
{
    if (plugin != nullptr && plugin->hasEditor())
    {
        return plugin->createEditorIfNeeded();
    }
    return nullptr;
}
