#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
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

    void setKitBuilderMode(bool shouldShow) { kitBuilderMode = shouldShow; resized(); repaint(); }
    bool isKitBuilderMode() const noexcept { return kitBuilderMode; }

    std::function<void()> onMixerPressed;
    std::function<void()> onHomePressed;

    void setProcessor(DrumTechProcessor* proc)
    {
        processor = proc;
        settingsPanel.setProcessor(proc);
        for (auto* p : allPieces())
            if (p) p->setProcessor(proc);
    }

    void triggerPieceVisual(int midiNote)
    {
        for (auto* p : allPieces())
            if (p && p->getMidiNote() == midiNote) { p->flash(); return; }
    }

private:
    class AnimatedNavButton : public juce::Component, private juce::Timer
    {
    public:
        enum class Orientation { Horizontal, Vertical };

        AnimatedNavButton(const juce::String& buttonText, Orientation o, bool leftFacing = false)
            : text(buttonText), orientation(o), leftEdge(leftFacing)
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            startTimerHz(60);
        }

        std::function<void()> onPressed;

        void paint(juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat().reduced(1.0f);
            const float t = static_cast<float>(juce::Time::getMillisecondCounterHiRes() * 0.001);
            const float pulse = 0.65f + 0.35f * std::sin(t * 2.2f);
            const float hover = hovered ? 1.0f : 0.0f;

            auto base = juce::Colour(0xFF3B434B).withAlpha(0.86f + 0.08f * hover);
            auto outline = juce::Colour(PluginColors::accent).withAlpha(0.38f + 0.32f * hover);
            auto glow = juce::Colour(PluginColors::accent).withAlpha((0.08f + 0.12f * hover) * pulse);

            g.setColour(glow);
            g.fillRoundedRectangle(r.expanded(3.0f), 12.0f);

            g.setColour(base);
            g.fillRoundedRectangle(r, 10.0f);
            g.setColour(outline);
            g.drawRoundedRectangle(r, 10.0f, 1.2f);

            g.setColour(juce::Colours::white.withAlpha(pressed ? 0.95f : 0.86f));
            g.setFont(PluginFonts::label(orientation == Orientation::Horizontal ? 24.0f : 20.0f));

            if (orientation == Orientation::Horizontal)
            {
                g.drawText(text, r, juce::Justification::centred, false);
            }
            else
            {
                const auto letters = juce::StringArray::fromTokens(text, "", "");
                int glyphH = juce::jmax(12, static_cast<int>(r.getHeight() / juce::jmax(1, text.length())));
                int y = static_cast<int>(r.getY() + 8.0f);
                for (int i = 0; i < text.length(); ++i)
                {
                    const juce::String ch(text.substring(i, i + 1));
                    g.drawText(ch, static_cast<int>(r.getX()), y, static_cast<int>(r.getWidth()), glyphH,
                               juce::Justification::centred, false);
                    y += glyphH - 2;
                }
            }

            if (leftEdge)
            {
                g.setColour(juce::Colours::black.withAlpha(0.15f));
                g.fillRect(r.removeFromRight(2.0f));
            }
        }

        void mouseEnter(const juce::MouseEvent&) override { hovered = true; repaint(); }
        void mouseExit(const juce::MouseEvent&) override { hovered = false; pressed = false; repaint(); }
        void mouseDown(const juce::MouseEvent&) override { pressed = true; repaint(); }
        void mouseUp(const juce::MouseEvent&) override
        {
            const bool trigger = pressed && getLocalBounds().contains(getMouseXYRelative());
            pressed = false;
            repaint();
            if (trigger && onPressed)
                onPressed();
        }

    private:
        void timerCallback() override
        {
            if (hovered)
                repaint();
        }

        juce::String text;
        Orientation orientation;
        bool leftEdge = false;
        bool hovered = false;
        bool pressed = false;
    };

    // -----------------------------------------------------------------------
    // Kick beater animation overlay
    // -----------------------------------------------------------------------
    class KickBeaterOverlay : public juce::Component, private juce::Timer
    {
    public:
        KickBeaterOverlay()  { setInterceptsMouseClicks(false, false); }

        void trigger()
        {
            beaterAngle = 0.0f;
            phase = Phase::Strike;
            startTimerHz(60);
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            if (phase == Phase::Rest && beaterAngle < 0.01f) return;

            auto b = getLocalBounds().toFloat();
            float cx = b.getCentreX();
            float cy = b.getBottom() - 4.0f;

            // Pedal base
            g.setColour(juce::Colour(0xFF888888));
            g.fillRect(cx - 10.0f, cy, 20.0f, 4.0f);

            // Beater arm — rotates from rest (-30°) to strike (-90°)
            float restDeg  = -30.0f;
            float hitDeg   = -92.0f;
            float angle    = juce::degreesToRadians(restDeg + beaterAngle * (hitDeg - restDeg));
            float armLen   = b.getHeight() * 0.75f;

            float ax = cx + std::cos(angle) * armLen;
            float ay = cy + std::sin(angle) * armLen;

            juce::Path arm;
            arm.startNewSubPath(cx, cy);
            arm.lineTo(ax, ay);

            g.setColour(juce::Colour(0xFFBBBBBB));
            g.strokePath(arm, juce::PathStrokeType(3.0f,
                juce::PathStrokeType::JointStyle::curved,
                juce::PathStrokeType::EndCapStyle::rounded));

            // Beater head
            float headR = 7.0f;
            g.setColour(juce::Colour(0xFFDDAA55));
            g.fillEllipse(ax - headR, ay - headR, headR * 2.0f, headR * 2.0f);
        }

    private:
        void timerCallback() override
        {
            if (phase == Phase::Strike)
            {
                beaterAngle += 0.12f;
                if (beaterAngle >= 1.0f) { beaterAngle = 1.0f; phase = Phase::Return; }
            }
            else
            {
                beaterAngle -= 0.06f;
                if (beaterAngle <= 0.0f) { beaterAngle = 0.0f; phase = Phase::Rest; stopTimer(); }
            }
            repaint();
        }

        enum class Phase { Rest, Strike, Return } phase = Phase::Rest;
        float beaterAngle = 0.0f;
    };

    // -----------------------------------------------------------------------
    // Invisible clickable hotspot with accent glow + selection
    // -----------------------------------------------------------------------
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
            bool showRing = (glowAlpha > 0.01f) || isHovered || isSelected;
            if (!showRing) return;

            float alpha = juce::jmax(isHovered  ? 0.45f : 0.0f,
                                     isSelected ? 0.9f  : 0.0f,
                                     glowAlpha);

            auto b = getLocalBounds().toFloat().reduced(2.0f);
            juce::Colour ring = juce::Colour(PluginColors::accent).withAlpha(alpha);

            // Glow fill
            g.setColour(ring.withAlpha(alpha * 0.2f));
            g.fillEllipse(b);

            // Ring outline
            g.setColour(ring);
            g.drawEllipse(b, isSelected ? 2.5f : 1.5f);

            // Label on selected
            if (isSelected)
            {
                g.setFont(PluginFonts::label(11.0f));
                g.setColour(ring);
                g.drawText(pieceName, b, juce::Justification::centred, false);
            }
        }

        void mouseEnter(const juce::MouseEvent&) override { isHovered = true;  repaint(); }
        void mouseExit (const juce::MouseEvent&) override { isHovered = false; repaint(); }

        void mouseDown(const juce::MouseEvent& e) override
        {
            isSelected = true;
            flash();
            if (processor) processor->triggerNote(midiNote, 100);
            if (onSelected) onSelected(pieceName, midiNote);
            repaint();
            juce::ignoreUnused(e);
        }

        void setProcessor(DrumTechProcessor* proc) { processor = proc; }
        int getMidiNote() const { return midiNote; }

        void flash()
        {
            glowAlpha = 1.0f;
            startTimer(30);
            repaint();
        }

        void timerCallback() override
        {
            glowAlpha *= 0.8f;
            if (glowAlpha < 0.01f) { glowAlpha = 0.0f; stopTimer(); }
            repaint();
        }

        // Callback when this drum is clicked (name, midi note)
        std::function<void(const juce::String&, int)> onSelected;

    private:
        juce::String pieceName;
        int   midiNote;
        Type  pieceType;
        juce::Colour baseColor;
        float glowAlpha  = 0.0f;
        bool  isHovered  = false;
        bool  isSelected = false;
        DrumTechProcessor* processor = nullptr;
    };

/*  // DISABLED: Piano keyboard roll at the bottom of drum view
    class PianoKeyboardPanel : public juce::Component
    {
    public:
        PianoKeyboardPanel()
        {
            keyState.addListener(nullptr);
            keyboard = std::make_unique<juce::MidiKeyboardComponent>(
                keyState, juce::MidiKeyboardComponent::horizontalKeyboard);
            keyboard->setAvailableRange(24, 84);
            keyboard->setScrollButtonsVisible(true);
            keyboard->setColour(juce::MidiKeyboardComponent::whiteNoteColourId,
                                juce::Colour(0xFFDDE1E7));
            keyboard->setColour(juce::MidiKeyboardComponent::blackNoteColourId,
                                juce::Colour(0xFF1C1C1F));
            keyboard->setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId,
                                juce::Colour(0xFF2C2C32));
            keyboard->setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
                                juce::Colour(PluginColors::accent).withAlpha(0.4f));
            keyboard->setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId,
                                juce::Colour(PluginColors::accent));
            addAndMakeVisible(*keyboard);
        }

        void resized() override { keyboard->setBounds(getLocalBounds()); }
        juce::MidiKeyboardState& getKeyboardState() { return keyState; }

    private:
        juce::MidiKeyboardState keyState;
        std::unique_ptr<juce::MidiKeyboardComponent> keyboard;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoKeyboardPanel)
    };
*/

    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------
    std::vector<DrumPiece*> allPieces()
    {
        return { kickPiece.get(), snarePiece.get(), hihatPiece.get(),
                 tom1Piece.get(), tom2Piece.get(), tom3Piece.get(),
                 crashPiece.get(), crash2Piece.get(), ridePiece.get() };
    }

    // -----------------------------------------------------------------------
    // Members
    // -----------------------------------------------------------------------
    DrumTechProcessor* processor = nullptr;
    juce::Image backdropImage;
    bool kitBuilderMode = false;

    AnimatedNavButton kitBuilderBtn { "KIT BUILDER", AnimatedNavButton::Orientation::Horizontal };
    AnimatedNavButton homeBtn       { "HOME",        AnimatedNavButton::Orientation::Horizontal };
    AnimatedNavButton triggerBtn    { "TRIGGER",     AnimatedNavButton::Orientation::Vertical, true };
    AnimatedNavButton mixerBtn      { "MIXER",       AnimatedNavButton::Orientation::Vertical };

    // Right settings panel
    InstrumentSettingsPanel settingsPanel;
    static constexpr int settingsPanelW = 336;

    // Kick beater animation overlay
    KickBeaterOverlay kickBeater;

    // Piano keyboard (bottom of kit view) - DISABLED for build
    // PianoKeyboardPanel pianoPanel;
    static constexpr int pianoH = 0;

    std::unique_ptr<DrumPiece> kickPiece, snarePiece, hihatPiece;
    std::unique_ptr<DrumPiece> tom1Piece, tom2Piece, tom3Piece;
    std::unique_ptr<DrumPiece> crashPiece, crash2Piece, ridePiece;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumKitView)
};
