#include "RhythmMatcher.h"
#include <algorithm>
#include <cmath>

RhythmMatcher::RhythmMatcher()
{
}

void RhythmMatcher::startRecording()
{
    recording = true;
    recordedPattern.clear();
    recordingStartTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
}

void RhythmMatcher::stopRecording()
{
    recording = false;
    recordedPattern.signature = normalizePattern(recordedPattern);
}

void RhythmMatcher::recordOnset(double timeInBeats)
{
    if (recording)
    {
        recordedPattern.onsetTimes.push_back(timeInBeats);
    }
}

void RhythmMatcher::clearRecording()
{
    recordedPattern.clear();
    recording = false;
}

std::vector<MatchResult> RhythmMatcher::findMatches(const GrooveLibrary& library, int maxResults)
{
    std::vector<MatchResult> results;
    
    if (recordedPattern.getNumOnsets() < 2)
        return results;
    
    auto allGrooves = library.getAllGrooves();
    
    for (const auto& groove : allGrooves)
    {
        float similarity = calculateSimilarity(recordedPattern, groove.rhythmSignature);
        
        if (similarity > 0.1f)
        {
            MatchResult result;
            result.groove = &groove;
            result.similarityScore = similarity;
            results.push_back(result);
        }
    }
    
    std::sort(results.begin(), results.end());
    
    if (results.size() > static_cast<size_t>(maxResults))
        results.resize(maxResults);
    
    return results;
}

float RhythmMatcher::calculateSimilarity(const RhythmPattern& pattern1, const juce::String& signature2) const
{
    if (pattern1.signature.isEmpty() || signature2.isEmpty())
        return 0.0f;
    
    return compareSignatures(pattern1.signature, signature2);
}

float RhythmMatcher::compareSignatures(const juce::String& sig1, const juce::String& sig2) const
{
    int matches = 0;
    int total = juce::jmin(sig1.length(), sig2.length());
    
    if (total == 0)
        return 0.0f;
    
    for (int i = 0; i < total; ++i)
    {
        if (sig1[i] == sig2[i])
            matches++;
        else if ((sig1[i] == 'K' || sig1[i] == 'S' || sig1[i] == 'H') &&
                 (sig2[i] == 'K' || sig2[i] == 'S' || sig2[i] == 'H'))
            matches += 0.5f;
    }
    
    float exactMatch = static_cast<float>(matches) / total;
    
    int shiftedMatches = 0;
    for (int shift = 1; shift < 4; ++shift)
    {
        int currentMatches = 0;
        for (int i = 0; i < total - shift; ++i)
        {
            if (sig1[i] == sig2[i + shift])
                currentMatches++;
        }
        shiftedMatches = juce::jmax(shiftedMatches, currentMatches);
    }
    
    float shiftedScore = static_cast<float>(shiftedMatches) / total * 0.8f;
    
    return juce::jmax(exactMatch, shiftedScore);
}

juce::String RhythmMatcher::normalizePattern(const RhythmPattern& pattern) const
{
    if (pattern.getNumOnsets() < 2)
        return "";
    
    const int gridSize = 16;
    bool grid[16] = {false};
    
    double minTime = pattern.onsetTimes[0];
    double maxTime = pattern.onsetTimes[pattern.getNumOnsets() - 1];
    double duration = maxTime - minTime;
    
    if (duration < 0.1)
        return "";
    
    for (double onset : pattern.onsetTimes)
    {
        double normalized = (onset - minTime) / duration;
        int gridPos = static_cast<int>(normalized * (gridSize - 1));
        gridPos = juce::jlimit(0, gridSize - 1, gridPos);
        grid[gridPos] = true;
    }
    
    juce::String signature;
    for (int i = 0; i < gridSize; ++i)
    {
        signature += grid[i] ? "K" : ".";
    }
    
    return signature;
}
