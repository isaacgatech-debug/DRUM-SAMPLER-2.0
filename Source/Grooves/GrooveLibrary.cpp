#include "GrooveLibrary.h"

GrooveLibrary::GrooveLibrary()
{
    loadFavorites();
}

void GrooveLibrary::setFactoryFolder(const juce::File& folder)
{
    factoryFolder = folder;
}

void GrooveLibrary::setUserFolder(const juce::File& folder)
{
    userFolder = folder;
}

void GrooveLibrary::scanFolders()
{
    grooves.clear();
    
    auto scanFolder = [this](const juce::File& folder)
    {
        if (!folder.exists() || !folder.isDirectory())
            return;
        
        auto files = folder.findChildFiles(juce::File::findFiles, true, "*.mid;*.midi");
        
        for (const auto& file : files)
        {
            addGroove(file);
        }
    };
    
    if (factoryFolder.exists())
        scanFolder(factoryFolder);
    
    if (userFolder.exists())
        scanFolder(userFolder);
}

void GrooveLibrary::addGroove(const juce::File& midiFile)
{
    if (!midiFile.existsAsFile())
        return;
    
    GrooveMetadata metadata;
    metadata.name = midiFile.getFileNameWithoutExtension();
    metadata.filePath = midiFile;
    
    juce::FileInputStream fileStream(midiFile);
    if (fileStream.openedOk())
    {
        juce::MidiFile midiFileData;
        if (midiFileData.readFrom(fileStream))
        {
            if (midiFileData.getNumTracks() > 0)
            {
                metadata.midiSequence = *midiFileData.getTrack(0);
                extractMetadata(metadata);
            }
        }
    }
    
    grooves.push_back(metadata);
}

void GrooveLibrary::removeGroove(const juce::String& name)
{
    grooves.erase(
        std::remove_if(grooves.begin(), grooves.end(),
            [&name](const GrooveMetadata& g) { return g.name == name; }),
        grooves.end()
    );
}

std::vector<GrooveMetadata> GrooveLibrary::getFavorites() const
{
    std::vector<GrooveMetadata> favorites;
    for (const auto& groove : grooves)
    {
        if (groove.isFavorite)
            favorites.push_back(groove);
    }
    return favorites;
}

std::vector<GrooveMetadata> GrooveLibrary::searchByName(const juce::String& searchText) const
{
    std::vector<GrooveMetadata> results;
    juce::String searchLower = searchText.toLowerCase();
    
    for (const auto& groove : grooves)
    {
        if (groove.name.toLowerCase().contains(searchLower))
            results.push_back(groove);
    }
    
    return results;
}

std::vector<GrooveMetadata> GrooveLibrary::searchByTempo(int minBPM, int maxBPM) const
{
    std::vector<GrooveMetadata> results;
    
    for (const auto& groove : grooves)
    {
        if (groove.tempoBPM >= minBPM && groove.tempoBPM <= maxBPM)
            results.push_back(groove);
    }
    
    return results;
}

std::vector<GrooveMetadata> GrooveLibrary::searchByGenre(const juce::String& genre) const
{
    std::vector<GrooveMetadata> results;
    juce::String genreLower = genre.toLowerCase();
    
    for (const auto& groove : grooves)
    {
        for (const auto& tag : groove.genreTags)
        {
            if (tag.toLowerCase().contains(genreLower))
            {
                results.push_back(groove);
                break;
            }
        }
    }
    
    return results;
}

void GrooveLibrary::toggleFavorite(const juce::String& name)
{
    for (auto& groove : grooves)
    {
        if (groove.name == name)
        {
            groove.isFavorite = !groove.isFavorite;
            saveFavorites();
            break;
        }
    }
}

void GrooveLibrary::saveFavorites()
{
    auto favFile = getFavoritesFile();
    favFile.getParentDirectory().createDirectory();
    
    juce::StringArray favoriteNames;
    for (const auto& groove : grooves)
    {
        if (groove.isFavorite)
            favoriteNames.add(groove.name);
    }
    
    favFile.replaceWithText(favoriteNames.joinIntoString("\n"));
}

void GrooveLibrary::loadFavorites()
{
    auto favFile = getFavoritesFile();
    if (favFile.existsAsFile())
    {
        juce::StringArray favoriteNames;
        favoriteNames.addLines(favFile.loadFileAsString());
        
        for (auto& groove : grooves)
        {
            groove.isFavorite = favoriteNames.contains(groove.name);
        }
    }
}

const GrooveMetadata* GrooveLibrary::getGroove(const juce::String& name) const
{
    for (const auto& groove : grooves)
    {
        if (groove.name == name)
            return &groove;
    }
    return nullptr;
}

void GrooveLibrary::extractMetadata(GrooveMetadata& metadata)
{
    metadata.rhythmSignature = generateRhythmSignature(metadata.midiSequence);
    
    juce::String nameLower = metadata.name.toLowerCase();
    if (nameLower.contains("rock"))
        metadata.genreTags.add("Rock");
    if (nameLower.contains("jazz"))
        metadata.genreTags.add("Jazz");
    if (nameLower.contains("funk"))
        metadata.genreTags.add("Funk");
    if (nameLower.contains("latin"))
        metadata.genreTags.add("Latin");
    if (nameLower.contains("metal"))
        metadata.genreTags.add("Metal");
    
    if (metadata.midiSequence.getNumEvents() > 0)
    {
        double lastTime = metadata.midiSequence.getEndTime();
        metadata.lengthInBeats = static_cast<int>(lastTime / 0.5);
    }
}

juce::String GrooveLibrary::generateRhythmSignature(const juce::MidiMessageSequence& sequence)
{
    juce::String signature;
    const int gridSize = 16;
    
    bool kickPattern[16] = {false};
    bool snarePattern[16] = {false};
    bool hihatPattern[16] = {false};
    
    for (int i = 0; i < sequence.getNumEvents(); ++i)
    {
        auto* event = sequence.getEventPointer(i);
        auto message = event->message;
        
        if (message.isNoteOn())
        {
            int note = message.getNoteNumber();
            double time = message.getTimeStamp();
            int gridPos = static_cast<int>((time / 2.0) * gridSize) % gridSize;
            
            if (note == 36)
                kickPattern[gridPos] = true;
            else if (note == 38)
                snarePattern[gridPos] = true;
            else if (note == 42)
                hihatPattern[gridPos] = true;
        }
    }
    
    for (int i = 0; i < gridSize; ++i)
    {
        if (kickPattern[i])
            signature += "K";
        else if (snarePattern[i])
            signature += "S";
        else if (hihatPattern[i])
            signature += "H";
        else
            signature += ".";
    }
    
    return signature;
}

juce::File GrooveLibrary::getFavoritesFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("DrumSampler2")
        .getChildFile("Favorites.txt");
}
