#include "MIDIPlayer.h"

MIDIPlayer::MIDIPlayer()
{
}

void MIDIPlayer::prepare(double sr)
{
    sampleRate = sr;
    updatePlaybackRate();
}

void MIDIPlayer::processBlock(juce::MidiBuffer& midiMessages, int numSamples)
{
    if (!playing || sequence.getNumEvents() == 0)
        return;
    
    double samplesPerBeat = (60.0 / currentTempo) * sampleRate;
    double positionIncrement = numSamples / samplesPerBeat;
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        double samplePosition = currentPosition + (sample / samplesPerBeat);
        
        while (nextEventIndex < sequence.getNumEvents())
        {
            auto* event = sequence.getEventPointer(nextEventIndex);
            double eventTime = event->message.getTimeStamp();
            
            if (eventTime <= samplePosition)
            {
                auto message = event->message;
                message.setTimeStamp(sample);
                midiMessages.addEvent(message, sample);
                nextEventIndex++;
            }
            else
            {
                break;
            }
        }
    }
    
    currentPosition += positionIncrement;
    
    if (nextEventIndex >= sequence.getNumEvents())
    {
        if (looping)
        {
            currentPosition = 0.0;
            nextEventIndex = 0;
        }
        else
        {
            stop();
        }
    }
}

void MIDIPlayer::loadGroove(const GrooveMetadata* groove)
{
    stop();
    currentGroove = groove;
    
    if (groove != nullptr)
    {
        sequence = groove->midiSequence;
        currentTempo = groove->tempoBPM;
        updatePlaybackRate();
    }
}

void MIDIPlayer::loadSequence(const juce::MidiMessageSequence& midiSequence, double sourceTempoBpm)
{
    stop();
    currentGroove = nullptr;
    sequence = midiSequence;
    currentTempo = juce::jlimit(30.0, 300.0, sourceTempoBpm);
    updatePlaybackRate();
}

void MIDIPlayer::play()
{
    playing = true;
}

void MIDIPlayer::stop()
{
    playing = false;
    currentPosition = 0.0;
    nextEventIndex = 0;
}

void MIDIPlayer::pause()
{
    playing = false;
}

void MIDIPlayer::setTempo(double bpm)
{
    currentTempo = juce::jlimit(30.0, 300.0, bpm);
    updatePlaybackRate();
}

void MIDIPlayer::setPosition(double positionInBeats)
{
    currentPosition = positionInBeats;
    
    nextEventIndex = 0;
    for (int i = 0; i < sequence.getNumEvents(); ++i)
    {
        auto* event = sequence.getEventPointer(i);
        if (event->message.getTimeStamp() > currentPosition)
        {
            nextEventIndex = i;
            break;
        }
    }
}

double MIDIPlayer::getLength() const
{
    if (sequence.getNumEvents() == 0)
        return 0.0;
    
    return sequence.getEndTime();
}

void MIDIPlayer::updatePlaybackRate()
{
    if (currentGroove != nullptr && currentGroove->tempoBPM > 0)
    {
        playbackRate = currentTempo / currentGroove->tempoBPM;
        return;
    }
    playbackRate = 1.0;
}
