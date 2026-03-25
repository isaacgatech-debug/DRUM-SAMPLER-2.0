#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "BinaryData.h"
#include "PluginColors.h"
#include "InstrumentSettingsPanel.h"
#include "../Core/PluginProcessor.h"

class DrumKitView : public juce::Component
{
public:
    DrumKitView();

    void paint  (juce::Graphics& g) override;
    void resized() override;

    void setProcessor(DrumSampler2Processor* proc)
    {
        processor = proc;
        if (kickPiece)   kickPiece->setProcessor(proc);
        if (snarePiece)  snarePiece->setProcessor(proc);
        if (hihatPiece)  hihatPiece->setProcessor(proc);
        if (tom1Piece)   tom1Piece->setProcessor(proc);
        if (tom2Piece)   tom2Piece->setProcessor(proc);
        if (tom3Piece)   tom3Piece->setProcessor(proc);
        if (crashPiece)  crashPiece->setProcessor(proc);
        if (crash2Piece) crash2Piece->setProcessor(proc);
        if (ridePiece)   ridePiece->setProcessor(proc);
    }

    void triggerPieceVisual(int midiNote)
    {
        DrumPiece* piece = nullptr;
        if (midiNote == 36 && kickPiece)   piece = kickPiece.get();
        else if (midiNote == 38 && snarePiece) piece = snarePiece.get();
        else if (midiNote == 42 && hihatPiece) piece = hihatPiece.get();
        else if (midiNote == 45 && tom2Piece)  piece = tom2Piece.get();
        else if (midiNote == 48 && tom1Piece)  piece = tom1Piece.get();
        else if (midiNote == 50 && tom3Piece)  piece = tom3Piece.get();
        else if (midiNote == 49 && crashPiece) piece = crashPiece.get();
        else if (midiNote == 57 && crash2Piece) piece = crash2Piece.get();
        else if (midiNote == 51 && ridePiece)  piece = ridePiece.get();
        if (piece) piece->flash();
    }

private:
    /**
     * Invisible clickable hotspot with cyan glow ring on hover/select.
     */
    class DrumPiece : public juce::Component, private juce::Timer
    {
    public:
        enum class Type { Drum, Cymbal };

        DrumPiece(const juce::String& pName, int note, Type type, juce::Colour color)
            : pieceName(pName), midiNote(note), pieceType(type), baseColor(color)
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        }

        void paint(juce::Graphics& g) override
        {
            // Invisible hotspot - no visual rendering
            juce::ignoreUnused(g);
        }

        void mouseEnter(const juce::MouseEvent&) override { isHovered = true;  repaint(); }
        void mouseExit (const juce::MouseEvent&) override { isHovered = false; repaint(); }

        void mouseDown(const juce::MouseEvent&) override
        {
            isSelected = true;
            flash();
            if (processor) processor->triggerNote(midiNote, 100);
            repaint();
        }

        void setProcessor(DrumSampler2Processor* proc) { processor = proc; }

        void flash()
        {
            glowAlpha = 1.0f;
            startTimer(30);
            repaint();
        }

        void timerCallback() override
        {
            glowAlpha *= 0.82f;
            if (glowAlpha < 0.01f) { glowAlpha = 0.0f; stopTimer(); }
            repaint();
        }

    private:
        juce::String pieceName;
        int   midiNote;
        Type  pieceType;
        juce::Colour baseColor;
        float glowAlpha = 0.0f;
        bool  isHovered  = false;
        bool  isSelected = false;
        DrumSampler2Processor* processor = nullptr;
    };

    DrumSampler2Processor* processor = nullptr;
    juce::Image backdropImage;

    // Right settings panel (260px)
    InstrumentSettingsPanel settingsPanel;
    static constexpr int settingsPanelW = 260;

    std::unique_ptr<DrumPiece> kickPiece, snarePiece, hihatPiece;
    std::unique_ptr<DrumPiece> tom1Piece, tom2Piece, tom3Piece;
    std::unique_ptr<DrumPiece> crashPiece, crash2Piece, ridePiece;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumKitView)
};
