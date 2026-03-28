#include "AudioTriggerEngine.h"
#include <juce_audio_formats/juce_audio_formats.h>

AudioTriggerEngine::AudioTriggerEngine()
{
}

void AudioTriggerEngine::prepare(double sr, int blockSize)
{
    sampleRate = sr;
    onsetDetector.prepare(sr, blockSize);
    drumClassifier.prepare(sr);
}

void AudioTriggerEngine::processAudioFile(const juce::File& audioFile, std::vector<TriggerResult>& results)
{
    results.clear();
    
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile));
    
    if (reader == nullptr)
        return;
    
    juce::AudioBuffer<float> fileBuffer(static_cast<int>(reader->numChannels),
                                       static_cast<int>(reader->lengthInSamples));
    reader->read(&fileBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
    
    std::vector<OnsetEvent> onsets;
    onsetDetector.processBlock(fileBuffer, onsets);
    
    applyBleedSuppression(onsets);
    
    for (const auto& onset : onsets)
    {
        int segmentStart = juce::jmax(0, static_cast<int>(onset.timeInSamples) - 100);
        int segmentLength = juce::jmin(2048, fileBuffer.getNumSamples() - segmentStart);
        
        auto segment = extractSegment(fileBuffer, segmentStart, segmentLength);
        
        DrumType drumType = drumClassifier.classify(segment, onset);
        
        TriggerResult result;
        result.midiNote = drumClassifier.getDrumTypeToMIDINote(drumType);
        result.velocity = static_cast<int>(juce::jlimit(1.0f, 127.0f, onset.strength * 127.0f));
        result.timeInSamples = onset.timeInSamples;
        result.drumType = drumType;
        result.confidence = onset.strength;
        
        results.push_back(result);
    }
}

void AudioTriggerEngine::processBlock(const juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiOut)
{
    if (!realTimeMode)
        return;

    const double blockStartTime = currentTime;
    std::vector<OnsetEvent> onsets;
    onsetDetector.processBlock(buffer, onsets);
    
    applyBleedSuppression(onsets);
    
    for (const auto& onset : onsets)
    {
        const int localOnsetSample = static_cast<int>(onset.timeInSamples - blockStartTime);
        if (localOnsetSample < 0 || localOnsetSample >= buffer.getNumSamples())
            continue;

        int segmentStart = juce::jmax(0, localOnsetSample - 100);
        int segmentLength = juce::jmin(2048, buffer.getNumSamples() - segmentStart);
        if (segmentLength <= 0)
            continue;
        
        auto segment = extractSegment(buffer, segmentStart, segmentLength);
        
        DrumType drumType = drumClassifier.classify(segment, onset);
        int midiNote = drumClassifier.getDrumTypeToMIDINote(drumType);
        int velocity = static_cast<int>(juce::jlimit(1.0f, 127.0f, onset.strength * 127.0f));
        
        int sampleOffset = juce::jlimit(0, buffer.getNumSamples() - 1, localOnsetSample);
        midiOut.addEvent(juce::MidiMessage::noteOn(1, midiNote, static_cast<juce::uint8>(velocity)), 
                        sampleOffset);
    }
    
    currentTime += buffer.getNumSamples();
}

void AudioTriggerEngine::setThreshold(float thresh)
{
    threshold = juce::jlimit(0.0f, 1.0f, thresh);
    onsetDetector.setThreshold(threshold);
}

void AudioTriggerEngine::setBleedSuppression(float amount)
{
    bleedSuppression = juce::jlimit(0.0f, 1.0f, amount);
}

void AudioTriggerEngine::setSensitivity(float sens)
{
    sensitivity = juce::jlimit(0.1f, 10.0f, sens);
    onsetDetector.setSensitivity(sensitivity);
}

void AudioTriggerEngine::setTargetDrumType(DrumType type)
{
    targetDrumType = type;
    onsetDetector.setTargetDrumType(type);
}

void AudioTriggerEngine::setMinTimeBetweenHits(double timeMs)
{
    minTimeBetweenHits = timeMs;
    onsetDetector.setMinTimeBetweenOnsets(timeMs);
}

void AudioTriggerEngine::applyBleedSuppression(std::vector<OnsetEvent>& onsets)
{
    if (bleedSuppression < 0.1f || onsets.size() < 2)
        return;
    
    std::vector<OnsetEvent> filtered;
    
    for (size_t i = 0; i < onsets.size(); ++i)
    {
        bool isBleed = false;
        
        for (size_t j = 0; j < onsets.size(); ++j)
        {
            if (i == j) continue;
            
            double timeDiff = std::abs(onsets[i].timeInSamples - onsets[j].timeInSamples);
            double timeDiffMs = (timeDiff / sampleRate) * 1000.0;
            
            if (timeDiffMs < 30.0 && onsets[j].strength > onsets[i].strength * (1.0f + bleedSuppression))
            {
                isBleed = true;
                break;
            }
        }
        
        if (!isBleed)
        {
            filtered.push_back(onsets[i]);
        }
    }
    
    onsets = filtered;
}

juce::AudioBuffer<float> AudioTriggerEngine::extractSegment(const juce::AudioBuffer<float>& source,
                                                            int startSample, int length)
{
    juce::AudioBuffer<float> segment(source.getNumChannels(), length);
    
    for (int ch = 0; ch < source.getNumChannels(); ++ch)
    {
        segment.copyFrom(ch, 0, source, ch, startSample, length);
    }
    
    return segment;
}

void AudioTriggerEngine::exportToMIDI(const std::vector<TriggerResult>& results, const juce::File& outputFile)
{
    juce::MidiFile midiFile;
    juce::MidiMessageSequence sequence;
    
    for (const auto& result : results)
    {
        double timeInBeats = result.timeInSamples / sampleRate * 2.0;
        
        auto noteOn = juce::MidiMessage::noteOn(1, result.midiNote, static_cast<juce::uint8>(result.velocity));
        noteOn.setTimeStamp(timeInBeats);
        sequence.addEvent(noteOn);
        
        auto noteOff = juce::MidiMessage::noteOff(1, result.midiNote);
        noteOff.setTimeStamp(timeInBeats + 0.1);
        sequence.addEvent(noteOff);
    }
    
    midiFile.addTrack(sequence);
    midiFile.setTicksPerQuarterNote(480);
    
    juce::FileOutputStream outputStream(outputFile);
    if (outputStream.openedOk())
    {
        midiFile.writeTo(outputStream);
    }
}
