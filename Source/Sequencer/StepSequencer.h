#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "StepPattern.h"
#include "../UI/PluginColors.h"

class DrumTechProcessor;

class StepSequencer : public juce::Component,
                      public juce::DragAndDropContainer,
                      private juce::Timer
{
public:
    StepSequencer();
    ~StepSequencer() override = default;
    
    void setProcessor(DrumTechProcessor* proc) { processor = proc; }
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    bool keyPressed(const juce::KeyPress& key) override;
    
    void setCurrentPattern(int index);
    int getCurrentPattern() const { return currentPatternIndex; }
    
    StepPattern& getPattern(int index);
    const StepPattern& getPattern(int index) const;
    
    void setPlaybackPosition(int step);
    int getPlaybackPosition() const { return playbackStep; }
    
    void play();
    void stop();
    bool isPlaying() const { return playing; }
    
    std::function<void(int patternIndex)> onPatternChanged;
    std::function<void(bool shouldPlay)> onPlayStateChanged;
    
private:
    struct StepButton : public juce::Component
    {
        int track = 0;
        int step = 0;
        StepSequencer* owner = nullptr;
        
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
    };
    
    void timerCallback() override;
    void updateStepButtons();
    void updateLayout();
    juce::Rectangle<int> getStepBounds(int track, int step) const;
    
    DrumTechProcessor* processor = nullptr;
    
    static constexpr int NUM_PATTERNS = 8;
    std::array<StepPattern, NUM_PATTERNS> patterns;
    int currentPatternIndex = 0;
    
    std::vector<std::unique_ptr<StepButton>> stepButtons;
    
    // Transport controls
    juce::DrawableButton playBtn{"play", juce::DrawableButton::ImageFitted};
    juce::DrawableButton stopBtn{"stop", juce::DrawableButton::ImageFitted};
    juce::Label barBeatLabel;
    juce::Label bpmLabel;
    juce::Slider bpmSlider;
    juce::Label midiDragBtn;
    std::unique_ptr<juce::Drawable> playIcon;
    std::unique_ptr<juce::Drawable> stopIcon;
    
    // Sequence length controls
    juce::TextButton step16Btn{"16"};
    juce::TextButton step32Btn{"32"};
    juce::TextButton step64Btn{"64"};
    juce::Label lengthLabel;
    
    std::array<juce::TextButton, NUM_PATTERNS> patternButtons;
    
    juce::Slider swingSlider;
    juce::Label swingLabel;
    
    juce::Viewport viewport;
    juce::Component gridContainer;
    juce::Component beatRuler;
    
    int playbackStep = -1;
    int scrollOffset = 0;
    bool playing = false;
    
    // Painting mode for click-and-drag step editing
    bool isPainting = false;
    bool paintMode = false;  // true = adding, false = removing
    
    static constexpr int CELL_SIZE = 48;
    static constexpr int CELL_GAP = 4;
    static constexpr int TRACK_LABEL_WIDTH = 100;
    static constexpr int TOP_CONTROLS_HEIGHT = 50;
    static constexpr int RULER_HEIGHT = 30;
    static constexpr int BOTTOM_CONTROLS_HEIGHT = 80;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepSequencer)
};
