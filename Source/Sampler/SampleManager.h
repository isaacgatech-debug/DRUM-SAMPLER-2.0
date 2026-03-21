#pragma once
#include <juce_core/juce_core.h>

class SampleManager
{
public:
    SampleManager();
    
    void setFactoryFolder(const juce::File& folder);
    void setUserFolder(const juce::File& folder);
    
private:
    juce::File factorySampleFolder;
    juce::File userSampleFolder;
};
