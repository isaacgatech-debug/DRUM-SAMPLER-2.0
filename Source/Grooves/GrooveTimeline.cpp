#include "GrooveTimeline.h"

GrooveTimeline::GrooveTimeline()
{
}

void GrooveTimeline::paint(juce::Graphics& g)
{
    g.fillAll(bgColour);
    
    auto bounds = getLocalBounds();
    int width = bounds.getWidth();
    int height = bounds.getHeight();
    
    g.setColour(gridColour);
    for (int beat = 0; beat < 64; ++beat)
    {
        int x = beatsToPixels(beat);
        if (x > width) break;
        
        float lineWidth = (beat % 4 == 0) ? 2.0f : 1.0f;
        g.drawVerticalLine(x, 0.0f, static_cast<float>(height));
    }
    
    for (const auto& clip : clips)
    {
        g.setColour(clip.colour.withAlpha(0.7f));
        g.fillRect(clip.bounds);
        
        g.setColour(clip.colour.brighter(0.3f));
        g.drawRect(clip.bounds, 2);
        
        if (clip.groove != nullptr)
        {
            g.setColour(juce::Colours::white);
            g.drawText(clip.groove->name, clip.bounds.reduced(4), 
                      juce::Justification::centredLeft, true);
        }
    }
    
    int playheadX = beatsToPixels(playbackPosition);
    g.setColour(playheadColour);
    g.drawVerticalLine(playheadX, 0.0f, static_cast<float>(height));
    
    g.fillRect(playheadX - 5, 0, 10, 10);
}

void GrooveTimeline::resized()
{
    updateClipBounds();
}

void GrooveTimeline::mouseDown(const juce::MouseEvent& e)
{
    selectedClipIndex = -1;
    
    for (int i = 0; i < static_cast<int>(clips.size()); ++i)
    {
        if (clips[i].bounds.contains(e.getPosition()))
        {
            selectedClipIndex = i;
            draggingClip = true;
            dragStartPos = e.getPosition();
            clipDragStartPosition = clips[i].startPosition;
            break;
        }
    }
    
    repaint();
}

void GrooveTimeline::mouseDrag(const juce::MouseEvent& e)
{
    if (draggingClip && selectedClipIndex >= 0)
    {
        int deltaX = e.getPosition().x - dragStartPos.x;
        double deltaBeats = pixelsToBeats(deltaX);
        
        double newPosition = clipDragStartPosition + deltaBeats;
        if (snapToGrid)
            newPosition = snapPosition(newPosition);
        
        newPosition = juce::jmax(0.0, newPosition);
        
        clips[selectedClipIndex].startPosition = newPosition;
        updateClipBounds();
        repaint();
    }
}

void GrooveTimeline::mouseUp(const juce::MouseEvent&)
{
    draggingClip = false;
}

bool GrooveTimeline::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& file : files)
    {
        if (file.endsWithIgnoreCase(".mid") || file.endsWithIgnoreCase(".midi"))
            return true;
    }
    return false;
}

void GrooveTimeline::filesDropped(const juce::StringArray& files, int x, int)
{
    double position = pixelsToBeats(x);
    if (snapToGrid)
        position = snapPosition(position);
    
    for (const auto& file : files)
    {
        if (file.endsWithIgnoreCase(".mid") || file.endsWithIgnoreCase(".midi"))
        {
        }
    }
}

void GrooveTimeline::addClip(const GrooveMetadata* groove, double position)
{
    TimelineClip clip;
    clip.groove = groove;
    clip.startPosition = position;
    clip.length = groove ? groove->lengthInBeats : 4.0;
    
    juce::Random random;
    clip.colour = juce::Colour::fromHSV(random.nextFloat(), 0.6f, 0.8f, 1.0f);
    
    clips.push_back(clip);
    updateClipBounds();
    repaint();
}

void GrooveTimeline::removeClip(int index)
{
    if (index >= 0 && index < static_cast<int>(clips.size()))
    {
        clips.erase(clips.begin() + index);
        updateClipBounds();
        repaint();
    }
}

void GrooveTimeline::clearTimeline()
{
    clips.clear();
    repaint();
}

void GrooveTimeline::setZoom(float zoomLevel)
{
    zoom = juce::jlimit(0.1f, 10.0f, zoomLevel);
    updateClipBounds();
    repaint();
}

double GrooveTimeline::pixelsToBeats(int pixels) const
{
    return pixels / (20.0 * zoom);
}

int GrooveTimeline::beatsToPixels(double beats) const
{
    return static_cast<int>(beats * 20.0 * zoom);
}

double GrooveTimeline::snapPosition(double position) const
{
    return std::round(position / gridSize) * gridSize;
}

void GrooveTimeline::updateClipBounds()
{
    int trackHeight = 60;
    int trackY = 20;
    
    for (auto& clip : clips)
    {
        int x = beatsToPixels(clip.startPosition);
        int width = beatsToPixels(clip.length);
        
        clip.bounds = juce::Rectangle<int>(x, trackY, width, trackHeight);
    }
}
