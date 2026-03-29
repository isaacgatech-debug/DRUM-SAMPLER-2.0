#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "GrooveLibrary.h"

class MIDIPlayer
{
public:
    MIDIPlayer();
    
    void prepare(double sampleRate);
    void processBlock(juce::MidiBuffer& midiMessages, int numSamples);
    
    void loadGroove(const GrooveMetadata* groove);
    void loadSequence(const juce::MidiMessageSequence& midiSequence, double sourceTempoBpm);
    void play();
    void stop();
    void pause();
    bool isPlaying() const { return playing; }
    
    void setTempo(double bpm);
    void setLoop(bool shouldLoop) { looping = shouldLoop; }
    void setPosition(double positionInBeats);
    
    double getTempo() const { return currentTempo; }
    double getPosition() const { return currentPosition; }
    double getLength() const;
    
private:
    void updatePlaybackRate();
    
    const GrooveMetadata* currentGroove = nullptr;
    juce::MidiMessageSequence sequence;
    
    double sampleRate = 44100.0;
    double currentTempo = 120.0;
    double currentPosition = 0.0;
    double playbackRate = 1.0;
    
    bool playing = false;
    bool looping = true;
    
    int nextEventIndex = 0;
};
