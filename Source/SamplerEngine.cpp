#include "SamplerEngine.h"

SamplerEngine::SamplerEngine()
{
    formatManager.registerBasicFormats();
    std::memset (rrCounters, 0, sizeof (rrCounters));
}

void SamplerEngine::prepareToPlay (double sampleRate, int /*blockSize*/)
{
    currentSampleRate = sampleRate;
}

void SamplerEngine::loadSamplesFromFolder (const juce::File& folder)
{
    samples.clear();
    DBG ("Loading samples from: " + folder.getFullPathName());

    // Expected filename convention:
    //   <InstrumentName>_n<midiNote>_v<velLow>-<velHigh>_rr<rrGroup>.wav
    //   e.g. Kick_n36_v0-63_rr1.wav
    // Also supports flat naming: any .wav with note info parsed from name.
    // Fallback: scan all .wav and assign by GM name keywords.

    struct NoteKeyword { const char* keyword; int note; };
    static const NoteKeyword keywords[] = {
        {"kick",   36}, {"bd",     36},
        {"snare",  38}, {"sd",     38},
        {"hihat",  42}, {"hh",     42}, {"hat",    42},
        {"tom1",   50}, {"racktom",50},
        {"tom2",   48},
        {"tom3",   45}, {"floortom",43},
        {"tom4",   43},
        {"crash1", 49}, {"crash",  49},
        {"ride",   51},
        {"splash", 55},
        {"crash2", 57},
    };

    auto wavFiles = folder.findChildFiles (juce::File::findFiles, true, "*.wav");
    wavFiles.addArray (folder.findChildFiles (juce::File::findFiles, true, "*.aif"));
    wavFiles.addArray (folder.findChildFiles (juce::File::findFiles, true, "*.aiff"));

    for (auto& f : wavFiles)
    {
        std::unique_ptr<juce::AudioFormatReader> reader (
            formatManager.createReaderFor (f));
        if (reader == nullptr) continue;

        DrumSample s;
        s.sampleRate = (int) reader->sampleRate;

        // Parse MIDI note from filename
        juce::String name = f.getFileNameWithoutExtension().toLowerCase();

        // Try _n<note>_ pattern first
        int noteFromName = -1;
        int rrFromName   = 0;
        int velLow       = 0;
        int velHigh      = 127;

        int nIdx = name.indexOf ("_n");
        if (nIdx >= 0)
        {
            noteFromName = name.substring (nIdx + 2).getIntValue();
        }

        int rrIdx = name.indexOf ("_rr");
        if (rrIdx >= 0)
            rrFromName = name.substring (rrIdx + 3).getIntValue() - 1;

        int vIdx = name.indexOf ("_v");
        if (vIdx >= 0)
        {
            juce::String velPart = name.substring (vIdx + 2);
            int dashIdx = velPart.indexOf ("-");
            if (dashIdx >= 0)
            {
                velLow  = velPart.substring (0, dashIdx).getIntValue();
                velHigh = velPart.substring (dashIdx + 1).getIntValue();
            }
        }

        // Fall back to keyword matching
        if (noteFromName < 0)
        {
            for (auto& kw : keywords)
            {
                if (name.contains (kw.keyword))
                {
                    noteFromName = kw.note;
                    break;
                }
            }
        }

        if (noteFromName < 0) continue; // skip unrecognised files

        s.midiNote = noteFromName;
        s.rrGroup  = rrFromName;
        s.velLow   = velLow;
        s.velHigh  = velHigh;

        s.buffer.setSize ((int) reader->numChannels,
                          (int) reader->lengthInSamples);
        reader->read (&s.buffer, 0, (int) reader->lengthInSamples, 0, true, true);

        samples.push_back (std::move (s));
        DBG ("Loaded sample: " + f.getFileName() + " -> Note " + juce::String (s.midiNote));
    }
    DBG ("Total samples loaded: " + juce::String (samples.size()));
}

void SamplerEngine::noteOn (int midiNote, int velocity)
{
    DBG ("noteOn called: note=" + juce::String (midiNote) + " vel=" + juce::String (velocity));
    // Collect matching samples for this note + velocity
    std::vector<int> candidates;
    for (int i = 0; i < (int) samples.size(); ++i)
    {
        auto& s = samples[i];
        if (s.midiNote == midiNote &&
            velocity >= s.velLow &&
            velocity <= s.velHigh)
        {
            candidates.push_back (i);
        }
    }

    if (candidates.empty())
    {
        DBG ("No candidates found for note " + juce::String (midiNote));
        return;
    }
    DBG ("Found " + juce::String (candidates.size()) + " candidates");

    // Round-robin within candidates
    int& counter = rrCounters[midiNote];
    int  picked  = candidates[counter % (int) candidates.size()];
    counter++;

    // Find a free voice
    DrumVoice* voice = nullptr;
    for (auto& v : voices)
    {
        if (!v.isActive()) { voice = &v; break; }
    }
    if (voice == nullptr) voice = &voices[0]; // steal oldest

    float gain = velocity / 127.0f;
    voice->trigger (&samples[picked], gain);
    DBG ("Triggered voice with sample " + juce::String (picked) + " gain=" + juce::String (gain));
}

void SamplerEngine::processBlock (juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midi)
{
    for (const auto& metadata : midi)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn() && msg.getVelocity() > 0)
            noteOn (msg.getNoteNumber(), msg.getVelocity());
    }

    for (auto& v : voices)
        v.process (buffer, 0, buffer.getNumSamples());
}
