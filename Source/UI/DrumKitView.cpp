#include "DrumKitView.h"
#include "../Core/PluginProcessor.h"
#include "../Core/ErrorLogger.h"

void DrumKitView::DrumPad::triggerNote()
{
    DBG("=== PAD CLICKED: " << padName.toStdString() << " (note " << midiNote << ") ===");
    
    if (processor)
    {
        DBG("Triggering note via processor");
        processor->triggerNote(midiNote, 100);
    }
    else
    {
        DBG("ERROR: Processor is NULL!");
    }
}

DrumKitView::DrumKitView()
{
    DBG("=== DrumKitView constructor called ===");
    
    // Create drum pads (processor will be set later via setProcessor)
    kickPad = std::make_unique<DrumPad>("KICK", 36, nullptr);
    addAndMakeVisible(*kickPad);
    DBG("Created KICK pad");
    
    snarePad = std::make_unique<DrumPad>("SNARE", 38, nullptr);
    addAndMakeVisible(*snarePad);
    DBG("Created SNARE pad");
    
    hihatPad = std::make_unique<DrumPad>("HI-HAT", 42, nullptr);
    addAndMakeVisible(*hihatPad);
    
    tom1Pad = std::make_unique<DrumPad>("TOM 1", 45, nullptr);
    addAndMakeVisible(*tom1Pad);
    
    tom2Pad = std::make_unique<DrumPad>("TOM 2", 48, nullptr);
    addAndMakeVisible(*tom2Pad);
    
    tom3Pad = std::make_unique<DrumPad>("TOM 3", 50, nullptr);
    addAndMakeVisible(*tom3Pad);
    
    crashPad = std::make_unique<DrumPad>("CRASH", 49, nullptr);
    addAndMakeVisible(*crashPad);
    
    ridePad = std::make_unique<DrumPad>("RIDE", 51, nullptr);
    addAndMakeVisible(*ridePad);
    
    clapPad = std::make_unique<DrumPad>("CLAP", 39, nullptr);
    addAndMakeVisible(*clapPad);
    
    rimPad = std::make_unique<DrumPad>("RIM", 37, nullptr);
    addAndMakeVisible(*rimPad);
    
    cowbellPad = std::make_unique<DrumPad>("COWBELL", 56, nullptr);
    addAndMakeVisible(*cowbellPad);
}

void DrumKitView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF1A1A1A));
    
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("Drum Kit - Click Pads to Play", 10, 10, 300, 30, juce::Justification::centredLeft);
}

void DrumKitView::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    bounds.removeFromTop(40); // Space for title
    
    int padWidth = 140;
    int padHeight = 100;
    int spacing = 10;
    
    // Top row - cymbals
    auto topRow = bounds.removeFromTop(padHeight);
    crashPad->setBounds(topRow.removeFromLeft(padWidth));
    topRow.removeFromLeft(spacing);
    ridePad->setBounds(topRow.removeFromLeft(padWidth));
    topRow.removeFromLeft(spacing);
    hihatPad->setBounds(topRow.removeFromLeft(padWidth));
    
    bounds.removeFromTop(spacing);
    
    // Middle row - toms
    auto midRow = bounds.removeFromTop(padHeight);
    tom1Pad->setBounds(midRow.removeFromLeft(padWidth));
    midRow.removeFromLeft(spacing);
    tom2Pad->setBounds(midRow.removeFromLeft(padWidth));
    midRow.removeFromLeft(spacing);
    tom3Pad->setBounds(midRow.removeFromLeft(padWidth));
    
    bounds.removeFromTop(spacing);
    
    // Bottom row - kick, snare, percussion
    auto bottomRow = bounds.removeFromTop(padHeight);
    kickPad->setBounds(bottomRow.removeFromLeft(padWidth));
    bottomRow.removeFromLeft(spacing);
    snarePad->setBounds(bottomRow.removeFromLeft(padWidth));
    bottomRow.removeFromLeft(spacing);
    clapPad->setBounds(bottomRow.removeFromLeft(padWidth));
    
    bounds.removeFromTop(spacing);
    
    // Extra row - rim, cowbell
    auto extraRow = bounds.removeFromTop(padHeight);
    rimPad->setBounds(extraRow.removeFromLeft(padWidth));
    extraRow.removeFromLeft(spacing);
    cowbellPad->setBounds(extraRow.removeFromLeft(padWidth));
}
