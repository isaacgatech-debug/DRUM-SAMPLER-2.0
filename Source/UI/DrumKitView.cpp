#include "DrumKitView.h"
#include "../Core/PluginProcessor.h"
#include "../Core/ErrorLogger.h"

DrumKitView::DrumKitView()
{
    // Load backdrop from binary data
    backdropImage = juce::ImageFileFormat::loadFrom(BinaryData::drumbackdrop_png,
                                                    BinaryData::drumbackdrop_pngSize);
    if (!backdropImage.isValid())
        LOG_ERROR("Failed to load backdrop image!");

    // Create drum pieces
    auto makePiece = [&](std::unique_ptr<DrumPiece>& piece,
                          const juce::String& n, int note,
                          DrumPiece::Type type, juce::Colour col)
    {
        piece = std::make_unique<DrumPiece>(n, note, type, col);

        // When a piece is selected, notify settings panel (drum swap UI)
        piece->onSelected = [this](const juce::String& name, int note2)
        {
            settingsPanel.setSelectedDrum(name, note2);
            juce::ignoreUnused(note2);
        };

        addAndMakeVisible(*piece);
    };

    makePiece(kickPiece,   "KICK",    36, DrumPiece::Type::Drum,   juce::Colour(0xFF8B4513));
    makePiece(snarePiece,  "SNARE",   38, DrumPiece::Type::Drum,   juce::Colour(0xFFD3D3D3));
    makePiece(tom1Piece,   "TOM 1",   48, DrumPiece::Type::Drum,   juce::Colour(0xFF4169E1));
    makePiece(tom2Piece,   "TOM 2",   45, DrumPiece::Type::Drum,   juce::Colour(0xFF4169E1));
    makePiece(tom3Piece,   "TOM 3",   50, DrumPiece::Type::Drum,   juce::Colour(0xFF4169E1));
    makePiece(hihatPiece,  "HI-HAT",  42, DrumPiece::Type::Cymbal, juce::Colour(0xFFFFD700));
    makePiece(crashPiece,  "CRASH",   49, DrumPiece::Type::Cymbal, juce::Colour(0xFFC0C0C0));
    makePiece(crash2Piece, "CRASH 2", 57, DrumPiece::Type::Cymbal, juce::Colour(0xFFB8B8B8));
    makePiece(ridePiece,   "RIDE",    51, DrumPiece::Type::Cymbal, juce::Colour(0xFFDAA520));

    // Wire kick piece to beater animation
    if (kickPiece)
    {
        auto origOnSelected = kickPiece->onSelected;
        kickPiece->onSelected = [this, origOnSelected](const juce::String& n, int note)
        {
            kickBeater.trigger();
            if (origOnSelected) origOnSelected(n, note);
        };
    }

    // Child components
    addAndMakeVisible(settingsPanel);
    addAndMakeVisible(kickBeater);
    // pianoPanel disabled for build
    // addAndMakeVisible(pianoPanel);
}

//==============================================================================
void DrumKitView::paint(juce::Graphics& g)
{
    auto kitArea = getLocalBounds()
                       .withTrimmedRight(settingsPanelW)
                       // .withTrimmedBottom(pianoH)  // piano disabled
                       .toFloat();

    if (backdropImage.isValid())
    {
        g.drawImage(backdropImage,
                    kitArea,
                    juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);

        // Subtle vignette
        g.setColour(juce::Colours::black.withAlpha(0.1f));
        g.fillRect(kitArea);
    }
    else
    {
        g.setColour(juce::Colour(PluginColors::pluginBg));
        g.fillRect(kitArea);
    }
}

//==============================================================================
void DrumKitView::resized()
{
    auto area = getLocalBounds();

    // Settings panel (right)
    settingsPanel.setBounds(area.removeFromRight(settingsPanelW));

    // Piano keyboard disabled for build
    // pianoPanel.setBounds(area.removeFromBottom(pianoH));

    int w = area.getWidth();
    int h = area.getHeight();

    // Crash 1 (upper-left)
    crashPiece->setBounds(
        area.getX() + (int)(w * 0.17) - (int)(w * 0.095),
        (int)(h * 0.29) - (int)(h * 0.034),
        (int)(w * 0.19), (int)(h * 0.068));

    // Hi-hat
    hihatPiece->setBounds(
        area.getX() + (int)(w * 0.17) - (int)(w * 0.083),
        (int)(h * 0.41) - (int)(h * 0.031),
        (int)(w * 0.166), (int)(h * 0.061));

    // Tom 1
    tom1Piece->setBounds(
        area.getX() + (int)(w * 0.29) - (int)(w * 0.080),
        (int)(h * 0.43) - (int)(h * 0.031),
        (int)(w * 0.161), (int)(h * 0.061));

    // Crash 2
    crash2Piece->setBounds(
        area.getX() + (int)(w * 0.52) - (int)(w * 0.122),
        (int)(h * 0.37) - (int)(h * 0.039),
        (int)(w * 0.244), (int)(h * 0.078));

    // Ride
    ridePiece->setBounds(
        area.getX() + (int)(w * 0.70) - (int)(w * 0.110),
        (int)(h * 0.32) - (int)(h * 0.038),
        (int)(w * 0.220), (int)(h * 0.076));

    // Snare
    snarePiece->setBounds(
        area.getX() + (int)(w * 0.32) - (int)(w * 0.098),
        (int)(h * 0.52) - (int)(h * 0.071),
        (int)(w * 0.195), (int)(h * 0.142));

    // Kick
    auto kickBounds = juce::Rectangle<int>(
        area.getX() + (int)(w * 0.45) - (int)(w * 0.110),
        (int)(h * 0.61) - (int)(h * 0.105),
        (int)(w * 0.220), (int)(h * 0.210));
    kickPiece->setBounds(kickBounds);

    // Kick beater overlay — positioned over kick, tall enough for pedal arm
    kickBeater.setBounds(kickBounds.expanded(20, 40).translated(0, 30));

    // Tom 2
    tom2Piece->setBounds(
        area.getX() + (int)(w * 0.58) - (int)(w * 0.095),
        (int)(h * 0.52) - (int)(h * 0.037),
        (int)(w * 0.190), (int)(h * 0.073));

    // Tom 3
    tom3Piece->setBounds(
        area.getX() + (int)(w * 0.77) - (int)(w * 0.120),
        (int)(h * 0.56) - (int)(h * 0.046),
        (int)(w * 0.239), (int)(h * 0.091));
}
