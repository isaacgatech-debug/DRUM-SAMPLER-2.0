#include "ParametricEQEffect.h"
#include "../Core/PluginProcessor.h"

ParametricEQEffect::ParametricEQEffect(DrumTechProcessor& proc, int mixerChannelIndex)
    : processor(proc), channelIndex(mixerChannelIndex)
{
}

juce::String ParametricEQEffect::paramBase(int band) const
{
    return "eqCh" + juce::String(channelIndex) + "Band" + juce::String(band);
}

void ParametricEQEffect::prepare(double sr, int blockSize)
{
    sampleRate = sr;
    spec.sampleRate = sr;
    spec.maximumBlockSize = static_cast<juce::uint32>(juce::jmax(1, blockSize));
    spec.numChannels = 2;

    for (auto& f : filters)
    {
        f.reset();
        f.prepare(spec);
    }

    for (auto& s : last)
        s.valid = false;
}

void ParametricEQEffect::reset()
{
    for (auto& f : filters)
        f.reset();
}

void ParametricEQEffect::updateBandIfNeeded(int b)
{
    auto& ap = processor.getAPVTS();
    const juce::String pb = paramBase(b);

    float freq = 1000.0f;
    float gainDb = 0.0f;
    float q = 1.0f;
    int type = 0;
    bool bypass = false;

    if (auto* raw = ap.getRawParameterValue(pb + "Freq"))
        freq = raw->load();
    if (auto* raw = ap.getRawParameterValue(pb + "Gain"))
        gainDb = raw->load();
    if (auto* raw = ap.getRawParameterValue(pb + "Q"))
        q = raw->load();
    if (auto* pc = dynamic_cast<juce::AudioParameterChoice*>(ap.getParameter(pb + "Type")))
        type = pc->getIndex();
    if (auto* raw = ap.getRawParameterValue(pb + "Bypass"))
        bypass = raw->load() > 0.5f;

    freq = juce::jlimit(20.0f, 20000.0f, freq);
    gainDb = juce::jlimit(-24.0f, 24.0f, gainDb);
    q = juce::jlimit(0.1f, 24.0f, q);

    auto& L = last[static_cast<size_t>(b)];
    if (L.valid && L.freq == freq && L.gainDb == gainDb && L.q == q && L.type == type && L.bypass == bypass)
        return;

    L = { freq, gainDb, q, type, bypass, true };

    if (bypass)
    {
        auto unity = Coefficients::makePeakFilter(static_cast<float>(sampleRate), 1000.0f, 0.707f, 1.0f);
        *filters[static_cast<size_t>(b)].coefficients = *unity;
        return;
    }

    const float g = juce::Decibels::decibelsToGain(gainDb);
    juce::ReferenceCountedObjectPtr<Coefficients> c;

    switch (type)
    {
        case 0: // Bell / peak
            c = Coefficients::makePeakFilter(static_cast<float>(sampleRate), freq, q, g);
            break;
        case 1: // Low shelf
            c = Coefficients::makeLowShelf(static_cast<float>(sampleRate), freq, q, g);
            break;
        case 2: // High shelf
            c = Coefficients::makeHighShelf(static_cast<float>(sampleRate), freq, q, g);
            break;
        case 3: // Low pass
            c = Coefficients::makeLowPass(static_cast<float>(sampleRate), freq, q);
            break;
        case 4: // High pass
            c = Coefficients::makeHighPass(static_cast<float>(sampleRate), freq, q);
            break;
        default:
            c = Coefficients::makePeakFilter(static_cast<float>(sampleRate), freq, q, g);
            break;
    }

    if (c != nullptr)
        *filters[static_cast<size_t>(b)].coefficients = *c;
}

void ParametricEQEffect::process(juce::AudioBuffer<float>& buffer)
{
    for (int b = 0; b < numBands; ++b)
        updateBandIfNeeded(b);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);

    for (int b = 0; b < numBands; ++b)
    {
        if (!last[static_cast<size_t>(b)].bypass)
            filters[static_cast<size_t>(b)].process(ctx);
    }
}
