#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "GrooveLibrary.h"
#include <vector>

struct RhythmPattern
{
    std::vector<double> onsetTimes;
    juce::String signature;
    
    void clear() { onsetTimes.clear(); signature.clear(); }
    int getNumOnsets() const { return static_cast<int>(onsetTimes.size()); }
};

struct MatchResult
{
    const GrooveMetadata* groove;
    float similarityScore;
    
    bool operator<(const MatchResult& other) const
    {
        return similarityScore > other.similarityScore;
    }
};

class RhythmMatcher
{
public:
    RhythmMatcher();
    
    void startRecording();
    void stopRecording();
    void recordOnset(double timeInBeats);
    void clearRecording();
    
    std::vector<MatchResult> findMatches(const GrooveLibrary& library, int maxResults = 10);
    
    const RhythmPattern& getRecordedPattern() const { return recordedPattern; }
    bool isRecording() const { return recording; }
    
private:
    float calculateSimilarity(const RhythmPattern& pattern1, const juce::String& signature2) const;
    float compareSignatures(const juce::String& sig1, const juce::String& sig2) const;
    juce::String normalizePattern(const RhythmPattern& pattern) const;
    
    RhythmPattern recordedPattern;
    bool recording = false;
    double recordingStartTime = 0.0;
};
