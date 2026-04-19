#include "StepSequencer.h"
#include "../UI/ThemeManager.h"
#include "../Core/PluginProcessor.h"
#include "BinaryData.h"

StepSequencer::StepSequencer()
{
    // Start timer for playback position updates (30 fps)
    startTimer(33);
    
    // Load transport icons
    auto loadSVG = [](const char* data, int size) -> std::unique_ptr<juce::Drawable> {
        if (data == nullptr || size <= 0) return nullptr;
        return juce::Drawable::createFromImageData(data, size);
    };
    
    playIcon = loadSVG(BinaryData::play_svg, BinaryData::play_svgSize);
    stopIcon = loadSVG(BinaryData::stop_svg, BinaryData::stop_svgSize);
    
    // Transport controls
    if (playIcon)
        playBtn.setImages(playIcon.get(), playIcon.get(), playIcon.get(), nullptr);
    if (stopIcon)
        stopBtn.setImages(stopIcon.get(), stopIcon.get(), stopIcon.get(), nullptr);
    
    playBtn.onClick = [this] { play(); };
    stopBtn.onClick = [this] { stop(); };
    addAndMakeVisible(playBtn);
    addAndMakeVisible(stopBtn);
    
    barBeatLabel.setText("1.1.1", juce::dontSendNotification);
    barBeatLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(barBeatLabel);
    
    // BPM controls
    bpmLabel.setText("BPM:", juce::dontSendNotification);
    bpmLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(bpmLabel);
    
    bpmSlider.setRange(30.0, 300.0, 1.0);
    bpmSlider.setValue(120.0, juce::dontSendNotification);
    bpmSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    bpmSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 60, 20);
    bpmSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    bpmSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    bpmSlider.onValueChange = [this]
    {
        auto& pattern = getPattern(currentPatternIndex);
        pattern.setTempo(bpmSlider.getValue());
        
        // If playing, update the player tempo
        if (playing && processor)
        {
            auto& player = processor->getMIDIPlayer();
            player.setTempo(bpmSlider.getValue());
        }
    };
    addAndMakeVisible(bpmSlider);
    
    // MIDI drag label - styled for visibility and dragging
    midiDragBtn.setText("⬇ MIDI", juce::dontSendNotification);
    midiDragBtn.setTooltip("Drag to export pattern as MIDI file");
    midiDragBtn.setJustificationType(juce::Justification::centred);
    midiDragBtn.setColour(juce::Label::backgroundColourId, juce::Colour(0xff4a4a4a));
    midiDragBtn.setColour(juce::Label::textColourId, juce::Colours::white);
    midiDragBtn.setColour(juce::Label::outlineColourId, juce::Colours::grey);
    addAndMakeVisible(midiDragBtn);
    
    // Sequence length controls
    lengthLabel.setText("Length:", juce::dontSendNotification);
    addAndMakeVisible(lengthLabel);
    
    step16Btn.onClick = [this] 
    { 
        bool wasPlaying = playing;
        getPattern(currentPatternIndex).setStepCount(16); 
        updateStepButtons(); 
        resized(); 
        repaint();  // Force repaint to update label sizes
        if (wasPlaying) play();  // Resume playback with new length
    };
    step32Btn.onClick = [this] 
    { 
        bool wasPlaying = playing;
        getPattern(currentPatternIndex).setStepCount(32); 
        updateStepButtons(); 
        resized(); 
        repaint();
        if (wasPlaying) play();
    };
    step64Btn.onClick = [this] 
    { 
        bool wasPlaying = playing;
        getPattern(currentPatternIndex).setStepCount(64); 
        updateStepButtons(); 
        resized(); 
        repaint();
        if (wasPlaying) play();
    };
    
    addAndMakeVisible(step16Btn);
    addAndMakeVisible(step32Btn);
    addAndMakeVisible(step64Btn);
    
    // Pattern buttons (A-H)
    const juce::String patternLabels[] = {"A", "B", "C", "D", "E", "F", "G", "H"};
    // Pattern buttons - hidden for now until we implement pattern functionality
    /*for (int i = 0; i < NUM_PATTERNS; ++i)
    {
        patternButtons[i].setButtonText(patternLabels[i]);
        patternButtons[i].onClick = [this, i] { setCurrentPattern(i); };
        addAndMakeVisible(patternButtons[i]);
    }*/
    
    // Swing slider
    swingSlider.setRange(0.0, 1.0, 0.01);
    swingSlider.setValue(0.0);
    swingSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    swingSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    swingSlider.onValueChange = [this]
    {
        getPattern(currentPatternIndex).setSwing((float)swingSlider.getValue());
    };
    addAndMakeVisible(swingSlider);
    
    swingLabel.setText("Swing", juce::dontSendNotification);
    addAndMakeVisible(swingLabel);
    
    // Grid container (no viewport - will scale to fit)
    addAndMakeVisible(gridContainer);
    gridContainer.setVisible(true);
    
    // Initialize the grid
    updateStepButtons();
    resized();
    
    // Enable keyboard focus for spacebar support
    setWantsKeyboardFocus(true);
}

void StepSequencer::paint(juce::Graphics& g)
{
    auto& tm = ThemeManager::get();
    
    // Dark background like reference
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Calculate dynamic cell size first
    auto& pattern = getPattern(currentPatternIndex);
    const int numSteps = pattern.getStepCount();
    const int availableWidth = gridContainer.getWidth();
    const int availableHeight = gridContainer.getHeight();
    
    int cellSize = CELL_SIZE;
    int rowHeight = CELL_SIZE + CELL_GAP;
    float fontSize = 13.0f;
    
    if (availableWidth > 0 && availableHeight > 0)
    {
        const int totalGaps = (numSteps - 1) * CELL_GAP;
        const int dynamicCellWidth = (availableWidth - totalGaps) / numSteps;
        
        const int trackGaps = (StepPattern::NUM_TRACKS - 1) * CELL_GAP;
        const int dynamicCellHeight = (availableHeight - trackGaps) / StepPattern::NUM_TRACKS;
        
        cellSize = juce::jmin(dynamicCellWidth, dynamicCellHeight, CELL_SIZE);
        rowHeight = cellSize + CELL_GAP;
        
        // Scale font size based on cell size (between 9px and 13px)
        fontSize = juce::jmap((float)cellSize, 20.0f, 48.0f, 9.0f, 13.0f);
        fontSize = juce::jlimit(9.0f, 13.0f, fontSize);
    }
    
    auto bounds = getLocalBounds();
    bounds.removeFromTop(TOP_CONTROLS_HEIGHT);
    
    // Draw track labels
    auto labelArea = bounds.removeFromLeft(TRACK_LABEL_WIDTH);
    auto rulerArea = labelArea.removeFromTop(RULER_HEIGHT);
    labelArea.removeFromBottom(BOTTOM_CONTROLS_HEIGHT);
    
    // Draw label background - darker
    g.setColour(juce::Colour(0xff1e1e1e));
    g.fillRect(labelArea);
    g.fillRect(rulerArea);
    
    // Draw track labels with dynamic font size
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(fontSize, juce::Font::bold));
    
    for (int track = 0; track < StepPattern::NUM_TRACKS; ++track)
    {
        auto rowBounds = labelArea.removeFromTop(rowHeight);
        g.drawText(pattern.getTrackName(track),
                  rowBounds.reduced(8, 0), juce::Justification::centredLeft);
    }
    
    // Draw beat ruler above grid
    auto rulerBounds = getLocalBounds();
    rulerBounds.removeFromTop(TOP_CONTROLS_HEIGHT);
    rulerBounds.removeFromLeft(TRACK_LABEL_WIDTH);
    auto ruler = rulerBounds.removeFromTop(RULER_HEIGHT);
    
    g.setColour(juce::Colour(0xff1e1e1e));
    g.fillRect(ruler);
    
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    
    // Draw beat numbers (every 4th step: 1, 2, 3, 4...) using already-calculated cellSize
    for (int step = 0; step < numSteps; ++step)
    {
        if (step % 4 == 0)  // Every 4th step
        {
            int beatNum = (step / 4) + 1;  // Beat numbers: 1, 2, 3, 4...
            int x = step * (cellSize + CELL_GAP);
            
            juce::Rectangle<int> stepRect(ruler.getX() + x, ruler.getY(), cellSize, RULER_HEIGHT);
            g.drawText(juce::String(beatNum), stepRect, juce::Justification::centred);
        }
    }
}

void StepSequencer::resized()
{
    auto bounds = getLocalBounds();
    
    // Top controls
    auto topArea = bounds.removeFromTop(TOP_CONTROLS_HEIGHT);
    
    playBtn.setBounds(topArea.removeFromLeft(50).reduced(5));
    stopBtn.setBounds(topArea.removeFromLeft(50).reduced(5));
    barBeatLabel.setBounds(topArea.removeFromLeft(80).reduced(5));
    
    topArea.removeFromLeft(10); // Spacing
    
    bpmLabel.setBounds(topArea.removeFromLeft(45).reduced(5));
    bpmSlider.setBounds(topArea.removeFromLeft(70).reduced(5));
    
    topArea.removeFromLeft(20); // Spacing
    
    lengthLabel.setBounds(topArea.removeFromLeft(60));
    step16Btn.setBounds(topArea.removeFromLeft(40).reduced(2));
    step32Btn.setBounds(topArea.removeFromLeft(40).reduced(2));
    step64Btn.setBounds(topArea.removeFromLeft(40).reduced(2));
    
    // Pattern buttons layout - hidden for now
    /*topArea.removeFromLeft(20); // Spacing
    
    for (int i = 0; i < NUM_PATTERNS; ++i)
    {
        patternButtons[i].setBounds(topArea.removeFromLeft(40).reduced(2));
    }*/
    
    // Bottom controls
    auto bottomArea = bounds.removeFromBottom(BOTTOM_CONTROLS_HEIGHT);
    bottomArea.removeFromLeft(TRACK_LABEL_WIDTH);
    
    swingLabel.setBounds(bottomArea.removeFromLeft(60));
    swingSlider.setBounds(bottomArea.removeFromLeft(200).reduced(4));
    
    // MIDI drag button in bottom right corner
    auto midiButtonArea = bottomArea.removeFromRight(100);
    midiDragBtn.setBounds(midiButtonArea.reduced(10));
    
    // Grid area (below ruler)
    bounds.removeFromLeft(TRACK_LABEL_WIDTH);
    bounds.removeFromTop(RULER_HEIGHT);
    gridContainer.setBounds(bounds);
    
    updateLayout();
}

void StepSequencer::mouseDown(const juce::MouseEvent& e)
{
    // Grab keyboard focus for spacebar support
    grabKeyboardFocus();
}

void StepSequencer::mouseDrag(const juce::MouseEvent& e)
{
    // Check if drag started from MIDI button
    if (midiDragBtn.getBounds().contains(e.getMouseDownPosition()))
    {
        if (e.getDistanceFromDragStart() > 10)
        {
            // Export current pattern to temporary MIDI file
            auto& pattern = getPattern(currentPatternIndex);
            
            // Create temp file with pattern name
            juce::String patternName = juce::String("Pattern_") + juce::String(char('A' + currentPatternIndex));
            auto tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                .getChildFile(patternName + ".mid");
            
            // Export the pattern
            if (pattern.exportToMidiFile(tempFile))
            {
                // Start drag operation
                juce::StringArray files;
                files.add(tempFile.getFullPathName());
                performExternalDragDropOfFiles(files, true);
            }
        }
    }
}

void StepSequencer::mouseUp(const juce::MouseEvent& e)
{
}

bool StepSequencer::keyPressed(const juce::KeyPress& key)
{
    // Toggle play/stop with spacebar
    if (key.getKeyCode() == juce::KeyPress::spaceKey)
    {
        if (playing)
            stop();
        else
            play();
        return true;
    }
    
    return false;
}

void StepSequencer::setCurrentPattern(int index)
{
    if (index >= 0 && index < NUM_PATTERNS)
    {
        currentPatternIndex = index;
        swingSlider.setValue(getPattern(currentPatternIndex).getSwing(), juce::dontSendNotification);
        updateStepButtons();
        
        if (onPatternChanged)
            onPatternChanged(index);
        
        repaint();
    }
}

StepPattern& StepSequencer::getPattern(int index)
{
    jassert(index >= 0 && index < NUM_PATTERNS);
    return patterns[index];
}

const StepPattern& StepSequencer::getPattern(int index) const
{
    jassert(index >= 0 && index < NUM_PATTERNS);
    return patterns[index];
}

void StepSequencer::setPlaybackPosition(int step)
{
    playbackStep = step;
    
    // Update bar/beat display
    // Assuming 16th notes, 4 steps per beat, 4 beats per bar
    if (step >= 0)
    {
        int bar = (step / 16) + 1;
        int beat = ((step % 16) / 4) + 1;
        int sixteenth = (step % 4) + 1;
        
        barBeatLabel.setText(juce::String(bar) + "." + juce::String(beat) + "." + juce::String(sixteenth),
                            juce::dontSendNotification);
    }
    else
    {
        barBeatLabel.setText("--", juce::dontSendNotification);
    }
    
    // Repaint all step buttons to show playback highlight
    for (auto& btn : stepButtons)
    {
        btn->repaint();
    }
    
    repaint();
}

void StepSequencer::play()
{
    if (!processor) return;
    
    playing = true;
    playbackStep = 0;
    
    // Convert current pattern to MIDI and load into player
    auto& pattern = getPattern(currentPatternIndex);
    auto midiSeq = pattern.toMidiSequence();
    
    auto& player = processor->getMIDIPlayer();
    player.loadSequence(midiSeq, pattern.getTempo());
    player.setTempo(pattern.getTempo());
    player.setLoop(true);
    player.play();
    
    if (onPlayStateChanged)
        onPlayStateChanged(true);
    
    repaint();
}

void StepSequencer::stop()
{
    if (processor)
    {
        auto& player = processor->getMIDIPlayer();
        player.stop();
    }
    
    playing = false;
    playbackStep = -1;
    barBeatLabel.setText("--", juce::dontSendNotification);
    
    if (onPlayStateChanged)
        onPlayStateChanged(false);
    
    repaint();
}

void StepSequencer::timerCallback()
{
    if (!processor || !playing) return;
    
    auto& player = processor->getMIDIPlayer();
    
    // If player stopped unexpectedly, restart it (for looping)
    if (!player.isPlaying() && playing)
    {
        play();  // Restart playback
        return;
    }
    
    // Update playback position based on MIDIPlayer position
    // Position is in beats, convert to steps (16th notes)
    double positionInBeats = player.getPosition();
    int currentStep = static_cast<int>(positionInBeats * 4.0);  // 4 steps per beat
    
    auto& pattern = getPattern(currentPatternIndex);
    currentStep = currentStep % pattern.getStepCount();  // Loop within pattern
    
    if (currentStep != playbackStep)
    {
        setPlaybackPosition(currentStep);
    }
}

void StepSequencer::updateStepButtons()
{
    stepButtons.clear();
    
    auto& pattern = getPattern(currentPatternIndex);
    const int numSteps = pattern.getStepCount();
    
    for (int track = 0; track < StepPattern::NUM_TRACKS; ++track)
    {
        for (int step = 0; step < numSteps; ++step)
        {
            auto btn = std::make_unique<StepButton>();
            btn->track = track;
            btn->step = step;
            btn->owner = this;
            gridContainer.addAndMakeVisible(btn.get());
            stepButtons.push_back(std::move(btn));
        }
    }
    
    updateLayout();
}

void StepSequencer::updateLayout()
{
    auto& pattern = getPattern(currentPatternIndex);
    const int numSteps = pattern.getStepCount();
    
    // Calculate dynamic cell size to fit all steps in available width
    const int availableWidth = gridContainer.getWidth();
    const int availableHeight = gridContainer.getHeight();
    
    if (availableWidth <= 0 || availableHeight <= 0) return;
    
    // Calculate cell size based on number of steps
    const int totalGaps = (numSteps - 1) * CELL_GAP;
    const int dynamicCellWidth = (availableWidth - totalGaps) / numSteps;
    
    const int trackGaps = (StepPattern::NUM_TRACKS - 1) * CELL_GAP;
    const int dynamicCellHeight = (availableHeight - trackGaps) / StepPattern::NUM_TRACKS;
    
    // Use the smaller dimension to keep cells square-ish, limited by max size
    const int cellSize = juce::jmin(dynamicCellWidth, dynamicCellHeight, CELL_SIZE);
    
    // Position all buttons
    for (auto& btn : stepButtons)
    {
        const int x = btn->step * (cellSize + CELL_GAP);
        const int y = btn->track * (cellSize + CELL_GAP);
        btn->setBounds(x, y, cellSize, cellSize);
    }
}

juce::Rectangle<int> StepSequencer::getStepBounds(int track, int step) const
{
    // This is now calculated dynamically in updateLayout
    return juce::Rectangle<int>();
}

// StepButton implementation
void StepSequencer::StepButton::paint(juce::Graphics& g)
{
    if (!owner) return;
    
    auto& pattern = owner->getPattern(owner->getCurrentPattern());
    const auto& s = pattern.getStep(track, step);
    
    auto& tm = ThemeManager::get();
    auto bounds = getLocalBounds().toFloat();
    
    // Background - match reference design
    if (s.isActive())
    {
        // Active step - bright accent color
        g.setColour(tm.accent());
        g.fillRoundedRectangle(bounds.reduced(1), 2.0f);
        
        // Velocity bar overlay (darker shade at bottom)
        if (s.velocity > 0)
        {
            float velHeight = (s.velocity / 127.0f) * bounds.getHeight();
            auto velBounds = bounds.withTop(bounds.getBottom() - velHeight).reduced(1);
            g.setColour(tm.accent().darker(0.3f));
            g.fillRoundedRectangle(velBounds, 2.0f);
        }
    }
    else
    {
        // Inactive step - dark cell like reference
        g.setColour(juce::Colour(0xff3a3a3a));
        g.fillRoundedRectangle(bounds.reduced(1), 2.0f);
    }
    
    // Border - subtle gray like reference
    g.setColour(juce::Colour(0xff555555));
    g.drawRoundedRectangle(bounds.reduced(1), 2.0f, 1.5f);
    
    // Playback highlight - white border
    if (step == owner->getPlaybackPosition())
    {
        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 2.0f, 2.5f);
    }
}

void StepSequencer::StepButton::mouseDown(const juce::MouseEvent& e)
{
    if (!owner) return;
    
    auto& pattern = owner->getPattern(owner->getCurrentPattern());
    auto& s = pattern.getStep(track, step);
    
    // Start painting mode - determine if we're adding or removing
    owner->isPainting = true;
    owner->paintMode = !s.isActive();  // If inactive, we're adding; if active, we're removing
    
    // Toggle this step
    if (owner->paintMode)
        s.velocity = 100;  // Add with default velocity
    else
        s.velocity = 0;  // Remove
    
    repaint();
}

void StepSequencer::StepButton::mouseDrag(const juce::MouseEvent& e)
{
    if (!owner) return;
    
    // If we're painting, find which button we're over and paint it
    if (owner->isPainting)
    {
        auto* componentUnderMouse = owner->gridContainer.getComponentAt(e.getEventRelativeTo(&owner->gridContainer).getPosition());
        if (auto* btn = dynamic_cast<StepButton*>(componentUnderMouse))
        {
            auto& pattern = owner->getPattern(owner->getCurrentPattern());
            auto& s = pattern.getStep(btn->track, btn->step);
            
            if (owner->paintMode && !s.isActive())
            {
                s.velocity = 100;
                btn->repaint();
            }
            else if (!owner->paintMode && s.isActive())
            {
                s.velocity = 0;
                btn->repaint();
            }
        }
    }
    else
    {
        // Original velocity adjustment when not painting
        auto& pattern = owner->getPattern(owner->getCurrentPattern());
        auto& s = pattern.getStep(track, step);
        
        if (s.isActive())
        {
            float dragDelta = -e.getDistanceFromDragStartY();
            float velChange = dragDelta / getHeight();
            int newVel = juce::jlimit(1, 127, (int)(s.velocity + velChange * 127));
            s.velocity = (uint8_t)newVel;
            repaint();
        }
    }
}

void StepSequencer::StepButton::mouseUp(const juce::MouseEvent& e)
{
    if (!owner) return;
    
    // Stop painting mode
    owner->isPainting = false;
}
