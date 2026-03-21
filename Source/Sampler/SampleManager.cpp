#include "SampleManager.h"

SampleManager::SampleManager()
{
}

void SampleManager::setFactoryFolder(const juce::File& folder)
{
    factorySampleFolder = folder;
}

void SampleManager::setUserFolder(const juce::File& folder)
{
    userSampleFolder = folder;
}
