#pragma once
#include "EffectProcessor.h"
#include <juce_dsp/juce_dsp.h>

class EQEffect : public EffectProcessor
{
public:
    struct Band {
        float frequency = 1000.0f;
        float gain = 0.0f;
        float Q = 0.707f;
    };
    
    EQEffect();
    
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    
    juce::String getName() const override { return "EQ"; }
    bool hasEditor() const override { return false; }
    juce::Component* createEditor() override { return nullptr; }
    
    void setLowBand(float freq, float gain) { lowBand.frequency = freq; lowBand.gain = gain; updateFilters(); }
    void setMidBand(float freq, float gain, float q) { midBand.frequency = freq; midBand.gain = gain; midBand.Q = q; updateFilters(); }
    void setHighBand(float freq, float gain) { highBand.frequency = freq; highBand.gain = gain; updateFilters(); }
    
private:
    void updateFilters();
    
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    
    juce::dsp::ProcessorChain<Filter, Filter, Filter> filterChain;
    
    Band lowBand{100.0f, 0.0f, 0.707f};
    Band midBand{1000.0f, 0.0f, 0.707f};
    Band highBand{10000.0f, 0.0f, 0.707f};
    
    double sampleRate = 44100.0;
};
