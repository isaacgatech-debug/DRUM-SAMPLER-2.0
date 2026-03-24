#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>

struct DrumSample
{
    juce::AudioBuffer<float> buffer;
    int sampleRate = 44100;
    int midiNote   = 0;
    int rrGroup    = 0;
    int velLow     = 0;
    int velHigh    = 127;
};

class DrumVoice
{
public:
    void trigger (const DrumSample* s, float gain)
    {
        sample    = s;
        position  = 0;
        active    = true;
        this->gain = gain;
    }

    void process (juce::AudioBuffer<float>& out, int startSample, int numSamples)
    {
        if (!active || sample == nullptr) return;

        int remaining = sample->buffer.getNumSamples() - position;
        int toRender  = juce::jmin (numSamples, remaining);

        for (int ch = 0; ch < out.getNumChannels(); ++ch)
        {
            int srcCh = ch % sample->buffer.getNumChannels();
            out.addFrom (ch, startSample,
                         sample->buffer, srcCh, position,
                         toRender, gain);
        }

        position += toRender;
        if (position >= sample->buffer.getNumSamples())
            active = false;
    }

    bool isActive() const { return active; }

private:
    const DrumSample* sample = nullptr;
    int   position = 0;
    float gain     = 1.0f;
    bool  active   = false;
};

class SamplerEngine
{
public:
    static constexpr int MAX_VOICES = 64;

    SamplerEngine();

    void loadSamplesFromFolder (const juce::File& folder);
    void prepareToPlay (double sampleRate, int blockSize);
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);

    int getNumSamples() const { return (int) samples.size(); }

private:
    void noteOn (int midiNote, int velocity);

    std::vector<DrumSample> samples;
    DrumVoice voices[MAX_VOICES];

    double currentSampleRate = 44100.0;
    int    rrCounters[128]   = {};

    juce::AudioFormatManager formatManager;
};
