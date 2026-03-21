#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

struct GrooveMetadata
{
    juce::String name;
    juce::File filePath;
    int tempoBPM = 120;
    juce::String timeSignature = "4/4";
    juce::StringArray genreTags;
    bool isFavorite = false;
    juce::String rhythmSignature;
    int lengthInBeats = 0;
    
    juce::MidiMessageSequence midiSequence;
};

class GrooveLibrary
{
public:
    GrooveLibrary();
    
    void setFactoryFolder(const juce::File& folder);
    void setUserFolder(const juce::File& folder);
    
    void scanFolders();
    void addGroove(const juce::File& midiFile);
    void removeGroove(const juce::String& name);
    
    std::vector<GrooveMetadata> getAllGrooves() const { return grooves; }
    std::vector<GrooveMetadata> getFavorites() const;
    std::vector<GrooveMetadata> searchByName(const juce::String& searchText) const;
    std::vector<GrooveMetadata> searchByTempo(int minBPM, int maxBPM) const;
    std::vector<GrooveMetadata> searchByGenre(const juce::String& genre) const;
    
    void toggleFavorite(const juce::String& name);
    void saveFavorites();
    void loadFavorites();
    
    const GrooveMetadata* getGroove(const juce::String& name) const;
    
private:
    void extractMetadata(GrooveMetadata& metadata);
    juce::String generateRhythmSignature(const juce::MidiMessageSequence& sequence);
    juce::File getFavoritesFile() const;
    
    std::vector<GrooveMetadata> grooves;
    juce::File factoryFolder;
    juce::File userFolder;
};
