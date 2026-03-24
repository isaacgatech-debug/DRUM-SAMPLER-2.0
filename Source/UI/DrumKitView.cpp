#include "DrumKitView.h"
#include "../Core/PluginProcessor.h"
#include "../Core/ErrorLogger.h"

void DrumKitView::DrumPiece::mouseDown(const juce::MouseEvent& e)
{
    DBG("*** CLICK: " << pieceName << " (note " << midiNote << ") ***");
    flash();
    if (processor)
    {
        DBG("*** Calling triggerNote for " << pieceName << " ***");
        processor->triggerNote(midiNote, 100);
    }
    else
        DBG("*** ERROR: No processor set! ***");
}

DrumKitView::DrumKitView()
{
    // Load backdrop image from binary data
    auto imageData = BinaryData::drumbackdrop_png;
    auto imageSize = BinaryData::drumbackdrop_pngSize;
    
    LOG_INFO("=== DrumKitView: Loading backdrop image ===");
    LOG_INFO("Image data pointer: " + juce::String(imageData ? "valid" : "null"));
    LOG_INFO("Image size: " + juce::String(imageSize) + " bytes");
    
    backdropImage = juce::ImageFileFormat::loadFrom(imageData, imageSize);
    
    if (backdropImage.isValid())
    {
        LOG_INFO("Backdrop image loaded successfully!");
        LOG_INFO("Image dimensions: " + juce::String(backdropImage.getWidth()) + "x" + juce::String(backdropImage.getHeight()));
    }
    else
    {
        LOG_ERROR("Failed to load backdrop image!");
    }
    
    // Create drum pieces with colors and types
    // Kick - large brown drum
    kickPiece = std::make_unique<DrumPiece>("KICK", 36, 
        DrumPiece::Type::Drum, juce::Colour(0xFF8B4513));
    addAndMakeVisible(*kickPiece);
    
    // Snare - light gray drum
    snarePiece = std::make_unique<DrumPiece>("SNARE", 38,
        DrumPiece::Type::Drum, juce::Colour(0xFFD3D3D3));
    addAndMakeVisible(*snarePiece);
    
    // Tom 1 - high tom (blue)
    tom1Piece = std::make_unique<DrumPiece>("TOM 1", 48,
        DrumPiece::Type::Drum, juce::Colour(0xFF4169E1));
    addAndMakeVisible(*tom1Piece);
    
    // Tom 2 - floor tom right (blue)
    tom2Piece = std::make_unique<DrumPiece>("TOM 2", 45,
        DrumPiece::Type::Drum, juce::Colour(0xFF4169E1));
    addAndMakeVisible(*tom2Piece);
    
    // Tom 3 - floor tom left (blue)
    tom3Piece = std::make_unique<DrumPiece>("TOM 3", 50,
        DrumPiece::Type::Drum, juce::Colour(0xFF4169E1));
    addAndMakeVisible(*tom3Piece);
    
    // Hi-Hat - gold cymbal
    hihatPiece = std::make_unique<DrumPiece>("HI-HAT", 42,
        DrumPiece::Type::Cymbal, juce::Colour(0xFFFFD700));
    addAndMakeVisible(*hihatPiece);
    
    // Crash - silver cymbal
    crashPiece = std::make_unique<DrumPiece>("CRASH", 49,
        DrumPiece::Type::Cymbal, juce::Colour(0xFFC0C0C0));
    addAndMakeVisible(*crashPiece);
    
    // Crash 2 - silver cymbal (second crash)
    crash2Piece = std::make_unique<DrumPiece>("CRASH 2", 57,
        DrumPiece::Type::Cymbal, juce::Colour(0xFFB8B8B8));
    addAndMakeVisible(*crash2Piece);
    
    // Ride - goldenrod cymbal
    ridePiece = std::make_unique<DrumPiece>("RIDE", 51,
        DrumPiece::Type::Cymbal, juce::Colour(0xFFDAA520));
    addAndMakeVisible(*ridePiece);
}

void DrumKitView::paint(juce::Graphics& g)
{
    if (backdropImage.isValid())
    {
        // Stretch image to fill entire component
        g.drawImage(backdropImage,
                    0, 0, getWidth(), getHeight(),
                    0, 0, backdropImage.getWidth(), backdropImage.getHeight());
        
        // Slight dark overlay to make drum pieces more visible
        g.setColour(juce::Colours::black.withAlpha(0.15f));
        g.fillAll();
    }
    else
    {
        g.setGradientFill(juce::ColourGradient(
            juce::Colour(0xFF2A2A2A), getWidth() / 2.0f, 0,
            juce::Colour(0xFF1A1A1A), getWidth() / 2.0f, (float)getHeight(),
            false));
        g.fillAll();
    }
}

void DrumKitView::resized()
{
    auto area = getLocalBounds();
    int w = area.getWidth();
    int h = area.getHeight();
    
    // Positions from red X anchor points in annotated image (1024x1024 stretched to fill)
    // Each piece is centered on its anchor point
    
    // All hit zones sized and centered to match shapes in the annotated image
    // Percentages derived from 1024x1024 reference image
    
    // Crash 1 (upper-left wide ellipse over left crash cymbal)
    crashPiece->setBounds(
        (int)(w * 0.17) - (int)(w * 0.095),
        (int)(h * 0.29) - (int)(h * 0.034),
        (int)(w * 0.19), (int)(h * 0.068));
    
    // Hi-hat (smaller ellipse below crash, over hi-hat)
    hihatPiece->setBounds(
        (int)(w * 0.17) - (int)(w * 0.083),
        (int)(h * 0.41) - (int)(h * 0.031),
        (int)(w * 0.166), (int)(h * 0.061));
    
    // Tom 1 / TOM (ellipse center-left, over rack tom)
    tom1Piece->setBounds(
        (int)(w * 0.29) - (int)(w * 0.080),
        (int)(h * 0.43) - (int)(h * 0.031),
        (int)(w * 0.161), (int)(h * 0.061));
    
    // Crash 2 (large wide ellipse center, over center cymbal)
    crash2Piece->setBounds(
        (int)(w * 0.52) - (int)(w * 0.122),
        (int)(h * 0.37) - (int)(h * 0.039),
        (int)(w * 0.244), (int)(h * 0.078));
    
    // Ride (large ellipse upper-right, over ride cymbal)
    ridePiece->setBounds(
        (int)(w * 0.70) - (int)(w * 0.110),
        (int)(h * 0.32) - (int)(h * 0.038),
        (int)(w * 0.220), (int)(h * 0.076));
    
    // Snare (rectangle over snare drum head)
    snarePiece->setBounds(
        (int)(w * 0.32) - (int)(w * 0.098),
        (int)(h * 0.52) - (int)(h * 0.071),
        (int)(w * 0.195), (int)(h * 0.142));
    
    // Kick (large circle over kick drum batter head)
    kickPiece->setBounds(
        (int)(w * 0.45) - (int)(w * 0.110),
        (int)(h * 0.61) - (int)(h * 0.105),
        (int)(w * 0.220), (int)(h * 0.210));
    
    // Tom 2 (ellipse right of kick, over right floor tom)
    tom2Piece->setBounds(
        (int)(w * 0.58) - (int)(w * 0.095),
        (int)(h * 0.52) - (int)(h * 0.037),
        (int)(w * 0.190), (int)(h * 0.073));
    
    // Tom 3 (large ellipse far right, over large floor tom)
    tom3Piece->setBounds(
        (int)(w * 0.77) - (int)(w * 0.120),
        (int)(h * 0.56) - (int)(h * 0.046),
        (int)(w * 0.239), (int)(h * 0.091));
}
