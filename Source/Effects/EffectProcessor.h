#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class EffectProcessor
{
public:
    virtual ~EffectProcessor() = default;
    
    virtual void prepare(double sampleRate, int blockSize) = 0;
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;
    virtual void reset() = 0;
    
    virtual juce::String getName() const = 0;
    virtual bool hasEditor() const = 0;
    virtual juce::Component* createEditor() = 0;
    
    virtual bool isThirdParty() const { return false; }
};
