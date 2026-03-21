#pragma once
#include "EffectProcessor.h"
#include <juce_dsp/juce_dsp.h>

class DelayEffect : public EffectProcessor
{
public:
    DelayEffect();
    
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    
    juce::String getName() const override { return "Delay"; }
    bool hasEditor() const override { return false; }
    juce::Component* createEditor() override { return nullptr; }
    
    void setDelayTime(float timeMs) { delayTimeMS = timeMs; updateDelay(); }
    void setFeedback(float fb) { feedback = juce::jlimit(0.0f, 0.95f, fb); }
    void setWetLevel(float wet) { wetLevel = juce::jlimit(0.0f, 1.0f, wet); }
    void setDryLevel(float dry) { dryLevel = juce::jlimit(0.0f, 1.0f, dry); }
    void setTempoSync(bool sync) { syncToTempo = sync; }
    void setTempo(double bpm) { currentTempo = bpm; updateDelay(); }
    
private:
    void updateDelay();
    
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineLeft;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineRight;
    
    float delayTimeMS = 500.0f;
    float feedback = 0.5f;
    float wetLevel = 0.5f;
    float dryLevel = 1.0f;
    bool syncToTempo = false;
    double currentTempo = 120.0;
    
    double sampleRate = 44100.0;
    int currentDelaySamples = 0;
};
