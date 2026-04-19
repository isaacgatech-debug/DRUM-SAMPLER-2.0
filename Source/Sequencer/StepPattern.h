#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

struct Step
{
    uint8_t velocity = 0;  // 0 = off, 1-127 = velocity
    bool accent = false;
    
    bool isActive() const { return velocity > 0; }
    void toggle(uint8_t defaultVelocity = 100)
    {
        velocity = isActive() ? 0 : defaultVelocity;
    }
};

class StepPattern
{
public:
    static constexpr int MAX_STEPS = 64;
    static constexpr int NUM_TRACKS = 11;
    
    StepPattern()
    {
        // Initialize track names for the 11 drum pieces
        trackNames[0] = "KICK";
        trackNames[1] = "SNARE";
        trackNames[2] = "HH CLOSED";
        trackNames[3] = "HH HALF";
        trackNames[4] = "HH OPEN";
        trackNames[5] = "TOM 1";
        trackNames[6] = "TOM 2";
        trackNames[7] = "TOM 3";
        trackNames[8] = "CRASH 1";
        trackNames[9] = "CRASH 2";
        trackNames[10] = "RIDE";
        
        // Initialize MIDI note numbers (GM drum map)
        midiNotes[0] = 36;  // Kick
        midiNotes[1] = 38;  // Snare
        midiNotes[2] = 42;  // Closed Hi-Hat
        midiNotes[3] = 44;  // Pedal Hi-Hat (Half-Open)
        midiNotes[4] = 46;  // Open Hi-Hat
        midiNotes[5] = 45;  // Low Tom
        midiNotes[6] = 47;  // Mid Tom
        midiNotes[7] = 48;  // High Tom
        midiNotes[8] = 49;  // Crash 1
        midiNotes[9] = 57;  // Crash 2
        midiNotes[10] = 51;  // Ride
    }
    
    Step& getStep(int track, int step)
    {
        jassert(track >= 0 && track < NUM_TRACKS);
        jassert(step >= 0 && step < MAX_STEPS);
        return steps[track][step];
    }
    
    const Step& getStep(int track, int step) const
    {
        jassert(track >= 0 && track < NUM_TRACKS);
        jassert(step >= 0 && step < MAX_STEPS);
        return steps[track][step];
    }
    
    void setStepCount(int count)
    {
        numSteps = juce::jlimit(16, MAX_STEPS, count);
    }
    
    int getStepCount() const { return numSteps; }
    
    void setSwing(float amount) { swing = juce::jlimit(0.0f, 1.0f, amount); }
    float getSwing() const { return swing; }
    
    void setTempo(double bpm) { tempo = juce::jlimit(30.0, 300.0, bpm); }
    double getTempo() const { return tempo; }
    
    juce::String getTrackName(int track) const
    {
        if (track >= 0 && track < NUM_TRACKS)
            return trackNames[track];
        return {};
    }
    
    int getMidiNote(int track) const
    {
        if (track >= 0 && track < NUM_TRACKS)
            return midiNotes[track];
        return 60;
    }
    
    void clear()
    {
        for (auto& track : steps)
            for (auto& step : track)
                step = Step();
    }
    
    juce::MidiMessageSequence toMidiSequence() const
    {
        juce::MidiMessageSequence sequence;
        
        const double beatsPerStep = 0.25;  // 16th notes
        const double swingAmount = swing * 0.5;  // Max 50% swing
        
        for (int step = 0; step < numSteps; ++step)
        {
            double stepTime = step * beatsPerStep;
            
            // Apply swing to every other step (8th note swing)
            if (step % 2 == 1)
                stepTime += swingAmount * beatsPerStep;
            
            for (int track = 0; track < NUM_TRACKS; ++track)
            {
                const auto& s = getStep(track, step);
                if (s.isActive())
                {
                    int note = getMidiNote(track);
                    int vel = s.velocity;
                    
                    auto noteOn = juce::MidiMessage::noteOn(1, note, (uint8_t)vel);
                    noteOn.setTimeStamp(stepTime);
                    sequence.addEvent(noteOn);
                    
                    auto noteOff = juce::MidiMessage::noteOff(1, note);
                    noteOff.setTimeStamp(stepTime + beatsPerStep * 0.9);
                    sequence.addEvent(noteOff);
                }
            }
        }
        
        // Add an end marker at the end of the pattern to ensure full length playback
        double endTime = numSteps * beatsPerStep;
        auto endMarker = juce::MidiMessage::allNotesOff(1);
        endMarker.setTimeStamp(endTime);
        sequence.addEvent(endMarker);
        
        sequence.updateMatchedPairs();
        return sequence;
    }
    
    bool exportToMidiFile(const juce::File& file) const
    {
        juce::MidiFile midiFile;
        
        // Create a MIDI track with the sequence
        auto sequence = toMidiSequence();
        midiFile.addTrack(sequence);
        
        // Set time format (480 ticks per quarter note is standard)
        midiFile.setTicksPerQuarterNote(480);
        
        // Add tempo track
        juce::MidiMessageSequence tempoTrack;
        auto tempoEvent = juce::MidiMessage::tempoMetaEvent(int(60000000.0 / tempo));
        tempoEvent.setTimeStamp(0.0);
        tempoTrack.addEvent(tempoEvent);
        midiFile.addTrack(tempoTrack);
        
        // Write to file
        juce::FileOutputStream stream(file);
        if (stream.openedOk())
        {
            midiFile.writeTo(stream);
            return true;
        }
        
        return false;
    }
    
private:
    std::array<std::array<Step, MAX_STEPS>, NUM_TRACKS> steps;
    std::array<juce::String, NUM_TRACKS> trackNames;
    std::array<int, NUM_TRACKS> midiNotes;
    
    int numSteps = 16;
    float swing = 0.0f;
    double tempo = 120.0;
};
