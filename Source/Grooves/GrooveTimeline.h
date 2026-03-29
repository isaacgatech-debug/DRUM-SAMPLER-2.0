#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../UI/PluginColors.h"
#include "GrooveLibrary.h"
#include "MIDIPlayer.h"
#include <vector>

struct TimelineClip
{
    const GrooveMetadata* groove = nullptr;
    double startPosition = 0.0;
    double length = 4.0;
    juce::Colour colour;
    
    juce::Rectangle<int> bounds;
};

class GrooveTimeline : public juce::Component,
                       public juce::FileDragAndDropTarget
{
public:
    GrooveTimeline();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    
    void addClip(const GrooveMetadata* groove, double position);
    void removeClip(int index);
    void clearTimeline();
    
    void setPlaybackPosition(double position) { playbackPosition = position; repaint(); }
    void setZoom(float zoomLevel);
    void setSnapToGrid(bool snap) { snapToGrid = snap; }
    
    std::vector<TimelineClip>& getClips() { return clips; }
    
private:
    double pixelsToBeats(int pixels) const;
    int beatsToPixels(double beats) const;
    double snapPosition(double position) const;
    void updateClipBounds();
    
    std::vector<TimelineClip> clips;
    
    double playbackPosition = 0.0;
    float zoom = 1.0f;
    bool snapToGrid = true;
    double gridSize = 1.0;
    
    int selectedClipIndex = -1;
    bool draggingClip = false;
    juce::Point<int> dragStartPos;
    double clipDragStartPosition = 0.0;
    
    juce::Colour bgColour{0xFF2A2A2A};
    juce::Colour gridColour{0xFF3A3A3A};
    juce::Colour playheadColour{juce::Colour(PluginColors::accent)};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveTimeline)
};
