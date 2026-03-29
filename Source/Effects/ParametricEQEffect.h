#pragma once
#include "EffectProcessor.h"
#include <juce_dsp/juce_dsp.h>
#include <array>

class DrumTechProcessor;

/** 8-band IIR parametric EQ insert; parameters live on processor APVTS (eqCh*Band*). */
class ParametricEQEffect : public EffectProcessor
{
public:
    ParametricEQEffect(DrumTechProcessor& proc, int mixerChannelIndex);

    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    juce::String getName() const override { return "Parametric EQ"; }
    bool hasEditor() const override { return false; }
    juce::Component* createEditor() override { return nullptr; }
    bool isThirdParty() const override { return false; }

    static constexpr int numBands = 8;

private:
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using Filter = juce::dsp::IIR::Filter<float>;

    juce::String paramBase(int band) const;

    void updateBandIfNeeded(int bandIndex);

    DrumTechProcessor& processor;
    int channelIndex = 0;
    double sampleRate = 44100.0;

    std::array<Filter, numBands> filters;
    juce::dsp::ProcessSpec spec {};

    struct BandState
    {
        float freq = 1000.0f;
        float gainDb = 0.0f;
        float q = 1.0f;
        int type = 0;
        bool bypass = false;
        bool valid = false;
    };
    std::array<BandState, numBands> last {};
};
