#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

class DrumSampler2Processor;

class DrumKitView : public juce::Component
{
public:
    DrumKitView();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setProcessor(DrumSampler2Processor* proc) 
    { 
        DBG("=== DrumKitView::setProcessor called with " << (proc ? "VALID" : "NULL") << " pointer ===");
        processor = proc;
        
        // Update all pads with the processor pointer
        if (kickPad) { kickPad->setProcessor(proc); DBG("Set processor for KICK pad"); }
        if (snarePad) { snarePad->setProcessor(proc); DBG("Set processor for SNARE pad"); }
        if (hihatPad) hihatPad->setProcessor(proc);
        if (tom1Pad) tom1Pad->setProcessor(proc);
        if (tom2Pad) tom2Pad->setProcessor(proc);
        if (tom3Pad) tom3Pad->setProcessor(proc);
        if (crashPad) crashPad->setProcessor(proc);
        if (ridePad) ridePad->setProcessor(proc);
        if (clapPad) clapPad->setProcessor(proc);
        if (rimPad) rimPad->setProcessor(proc);
        if (cowbellPad) cowbellPad->setProcessor(proc);
        
        DBG("=== All pads updated with processor ===");
    }
    
    void triggerPadVisual(int midiNote)
    {
        // Find the pad that matches this MIDI note and flash it
        DrumPad* pad = nullptr;
        
        if (midiNote == 36 && kickPad) pad = kickPad.get();
        else if (midiNote == 38 && snarePad) pad = snarePad.get();
        else if (midiNote == 42 && hihatPad) pad = hihatPad.get();
        else if (midiNote == 45 && tom1Pad) pad = tom1Pad.get();
        else if (midiNote == 48 && tom2Pad) pad = tom2Pad.get();
        else if (midiNote == 50 && tom3Pad) pad = tom3Pad.get();
        else if (midiNote == 49 && crashPad) pad = crashPad.get();
        else if (midiNote == 51 && ridePad) pad = ridePad.get();
        else if (midiNote == 39 && clapPad) pad = clapPad.get();
        else if (midiNote == 37 && rimPad) pad = rimPad.get();
        else if (midiNote == 56 && cowbellPad) pad = cowbellPad.get();
        
        if (pad)
        {
            pad->flash();
        }
    }
    
private:
    class DrumPad : public juce::TextButton, private juce::Timer
    {
    public:
        DrumPad(const juce::String& name, int note, DrumSampler2Processor* proc)
            : juce::TextButton(name), padName(name), midiNote(note), processor(proc)
        {
            setClickingTogglesState(false);
            onClick = [this]() { triggerNote(); };
        }
        
        void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = getLocalBounds().toFloat();
            
            // Draw pad background with flash overlay
            juce::Colour baseColour;
            if (shouldDrawButtonAsDown || isDown())
                baseColour = juce::Colour(0xFFE8A020);
            else if (shouldDrawButtonAsHighlighted || isOver())
                baseColour = juce::Colour(0xFFE8A020).withAlpha(0.7f);
            else
                baseColour = juce::Colour(0xFF3A3A3A);
            
            // Blend with flash color
            if (flashAlpha > 0.0f)
            {
                baseColour = baseColour.interpolatedWith(juce::Colour(0xFFFFAA00), flashAlpha);
            }
            
            g.setColour(baseColour);
            g.fillRoundedRectangle(bounds, 8.0f);
            
            // Draw border
            g.setColour(juce::Colour(0xFF5A5A5A));
            g.drawRoundedRectangle(bounds, 8.0f, 2.0f);
            
            // Draw pad name
            g.setColour(juce::Colours::white);
            g.setFont(16.0f);
            g.drawText(padName, bounds, juce::Justification::centred);
            
            // Draw MIDI note number
            g.setFont(10.0f);
            g.setColour(juce::Colours::lightgrey);
            auto textBounds = bounds;
            textBounds.removeFromTop(bounds.getHeight() - 20);
            g.drawText("Note " + juce::String(midiNote), textBounds, juce::Justification::centred);
        }
        
        void setProcessor(DrumSampler2Processor* proc)
        {
            DBG("DrumPad '" << padName.toStdString() << "' setProcessor: " << (proc ? "VALID" : "NULL"));
            processor = proc;
        }
        
        void flash()
        {
            // Trigger visual flash
            flashAlpha = 1.0f;
            startTimer(30); // Update every 30ms
            repaint();
        }
        
        void timerCallback() override
        {
            flashAlpha *= 0.85f; // Fade out
            if (flashAlpha < 0.01f)
            {
                flashAlpha = 0.0f;
                stopTimer();
            }
            repaint();
        }
        
    private:
        void triggerNote();
        
        juce::String padName;
        int midiNote;
        DrumSampler2Processor* processor;
        float flashAlpha = 0.0f;
    };
    
    DrumSampler2Processor* processor = nullptr;
    
    std::unique_ptr<DrumPad> kickPad;
    std::unique_ptr<DrumPad> snarePad;
    std::unique_ptr<DrumPad> hihatPad;
    std::unique_ptr<DrumPad> tom1Pad;
    std::unique_ptr<DrumPad> tom2Pad;
    std::unique_ptr<DrumPad> tom3Pad;
    std::unique_ptr<DrumPad> crashPad;
    std::unique_ptr<DrumPad> ridePad;
    std::unique_ptr<DrumPad> clapPad;
    std::unique_ptr<DrumPad> rimPad;
    std::unique_ptr<DrumPad> cowbellPad;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumKitView)
};
