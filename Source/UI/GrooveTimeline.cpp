#include "GrooveTimeline.h"

GrooveTimeline::GrooveTimeline()
{
    auto styleBtn = [](juce::TextButton& btn)
    {
        btn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textPrimary));
    };

    styleBtn(trackDropdown);
    styleBtn(blockDropdown);
    styleBtn(addTrackBtn);
    styleBtn(selectBtn);
    styleBtn(cutBtn);
    styleBtn(zoomInBtn);
    styleBtn(zoomOutBtn);

    addAndMakeVisible(trackDropdown);
    addAndMakeVisible(blockDropdown);
    addAndMakeVisible(addTrackBtn);
    addAndMakeVisible(selectBtn);
    addAndMakeVisible(cutBtn);
    addAndMakeVisible(zoomInBtn);
    addAndMakeVisible(zoomOutBtn);

    // Seed with example blocks
    blocks.push_back({ 1, 4, 0, "Hats Tight Tip" });
    blocks.push_back({ 5, 2, 1, "Fill" });
    blocks.push_back({ 7, 4, 3, "Pre-Chorus" });
    blocks.push_back({ 11, 2, 1, "Fill" });
    blocks.push_back({ 13, 8, 2, "Chorus" });

    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void GrooveTimeline::resized()
{
    auto area = getLocalBounds();

    auto trackBarArea = area.removeFromTop(trackBarH);
    trackDropdown.setBounds(trackBarArea.removeFromLeft(80).reduced(2));
    blockDropdown.setBounds(trackBarArea.removeFromLeft(70).reduced(2));
    trackBarArea.removeFromLeft(4);
    addTrackBtn.setBounds(trackBarArea.removeFromLeft(24).reduced(2));
    selectBtn.setBounds(trackBarArea.removeFromRight(36).reduced(2));
    cutBtn.setBounds(trackBarArea.removeFromRight(36).reduced(2));
    zoomInBtn.setBounds(trackBarArea.removeFromRight(24).reduced(2));
    zoomOutBtn.setBounds(trackBarArea.removeFromRight(24).reduced(2));

    area.removeFromTop(rulerH);   // ruler is painted
    rollRect = area.removeFromTop(rollH);
}

void GrooveTimeline::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    auto trackBarArea = b.removeFromTop(trackBarH);
    auto rulerArea    = b.removeFromTop(rulerH);
    auto rollArea     = b.removeFromTop(rollH);

    // Track bar bg
    g.setColour(juce::Colour(PluginColors::pluginPanel));
    g.fillRect(trackBarArea);
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawHorizontalLine(static_cast<int>(trackBarArea.getBottom()) - 1,
                         trackBarArea.getX(), trackBarArea.getRight());

    // Track name label
    g.setFont(PluginFonts::label(10.0f));
    g.setColour(juce::Colour(PluginColors::textPrimary));
    g.drawText("Track 1", static_cast<int>(trackBarArea.getX() + 160), static_cast<int>(trackBarArea.getY()),
               80, static_cast<int>(trackBarArea.getHeight()), juce::Justification::centredLeft, false);

    drawRuler(g, rulerArea);
    drawRollArea(g, rollArea);
}

void GrooveTimeline::drawRuler(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(juce::Colour(PluginColors::pluginSurface));
    g.fillRect(area);
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawHorizontalLine(static_cast<int>(area.getBottom()) - 1, area.getX(), area.getRight());

    g.setFont(PluginFonts::mono(9.0f));
    g.setColour(juce::Colour(PluginColors::textMuted));

    int numBars = static_cast<int>(area.getWidth()) / barWidth + 2;
    for (int bar = 1; bar <= numBars; ++bar)
    {
        int x = barToX(bar);
        if (x > area.getRight()) break;

        g.setColour(juce::Colour(PluginColors::pluginBorder));
        g.drawVerticalLine(x, area.getY(), area.getBottom());

        g.setColour(juce::Colour(PluginColors::textMuted));
        g.drawText(juce::String(bar), x + 2, static_cast<int>(area.getY()),
                   30, static_cast<int>(area.getHeight()), juce::Justification::centredLeft, false);
    }
}

void GrooveTimeline::drawRollArea(juce::Graphics& g, juce::Rectangle<float> area)
{
    // Background
    g.setColour(juce::Colour(PluginColors::pluginBg));
    g.fillRect(area);

    // Bar gridlines
    g.setColour(juce::Colour(PluginColors::pluginBorder).withAlpha(0.4f));
    int numBars = static_cast<int>(area.getWidth()) / barWidth + 2;
    for (int bar = 1; bar <= numBars; ++bar)
    {
        int x = barToX(bar);
        if (x > area.getRight()) break;
        g.drawVerticalLine(x, area.getY(), area.getBottom());
    }

    // Groove blocks
    for (int i = 0; i < static_cast<int>(blocks.size()); ++i)
    {
        const auto& block = (i == draggingBlock)
            ? GrooveBlock { dragCurrentBar, blocks[i].durationBars, blocks[i].type, blocks[i].name }
            : blocks[i];

        float bx = static_cast<float>(barToX(block.startBar));
        float bw = static_cast<float>(block.durationBars * barWidth - 2);
        juce::Rectangle<float> blockRect(bx, area.getY() + 4.0f, bw, area.getHeight() - 8.0f);
        drawBlock(g, block, blockRect, i == selectedBlock);
    }

    // Bottom border
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawHorizontalLine(static_cast<int>(area.getBottom()) - 1, area.getX(), area.getRight());
}

void GrooveTimeline::drawBlock(juce::Graphics& g, const GrooveBlock& block,
                                juce::Rectangle<float> rect, bool selected)
{
    juce::Colour col = PluginColors::grooveColor(block.type);

    // Block fill
    g.setColour(col.withAlpha(0.25f));
    g.fillRoundedRectangle(rect, 3.0f);

    // Border — brighter if selected
    g.setColour(selected ? col : col.withAlpha(0.55f));
    g.drawRoundedRectangle(rect, 3.0f, selected ? 1.5f : 0.75f);

    // Mini waveform decoration (random squiggle for visual)
    {
        juce::Path waveform;
        float wy = rect.getCentreY();
        float amp = rect.getHeight() * 0.18f;
        int steps = static_cast<int>(rect.getWidth() / 3);
        if (steps > 1)
        {
            waveform.startNewSubPath(rect.getX() + 2, wy);
            for (int s = 0; s < steps; ++s)
            {
                float sx = rect.getX() + 2 + s * (rect.getWidth() - 4) / steps;
                float sy = wy + amp * std::sin(s * 0.7f + block.startBar);
                waveform.lineTo(sx, sy);
            }
            g.setColour(col.withAlpha(0.35f));
            g.strokePath(waveform, juce::PathStrokeType(1.0f));
        }
    }

    // Pattern name
    g.setFont(PluginFonts::label(9.0f));
    g.setColour(col.brighter(0.4f));
    g.drawText(block.name,
               rect.reduced(4.0f, 2.0f).removeFromTop(12.0f),
               juce::Justification::centredLeft, true);

    // Type label
    g.setFont(PluginFonts::mono(8.0f));
    g.setColour(col.withAlpha(0.75f));
    g.drawText(PluginColors::grooveTypeName(block.type),
               rect.reduced(4.0f, 2.0f).removeFromBottom(10.0f),
               juce::Justification::centredLeft, true);
}

int GrooveTimeline::barToX(int bar) const
{
    return (bar - 1) * barWidth - scrollX;
}

int GrooveTimeline::xToBar(int x) const
{
    return (x + scrollX) / barWidth + 1;
}

int GrooveTimeline::blockAtPoint(juce::Point<int> pt) const
{
    float ry = static_cast<float>(rollRect.getY());
    float rh = static_cast<float>(rollRect.getHeight());

    for (int i = 0; i < static_cast<int>(blocks.size()); ++i)
    {
        float bx = static_cast<float>(barToX(blocks[i].startBar));
        float bw = static_cast<float>(blocks[i].durationBars * barWidth);
        juce::Rectangle<float> r(bx, ry + 4, bw - 2, rh - 8);
        if (r.contains(pt.toFloat()))
            return i;
    }
    return -1;
}

void GrooveTimeline::mouseDown(const juce::MouseEvent& e)
{
    if (!rollRect.contains(e.getPosition())) return;

    int hit = blockAtPoint(e.getPosition());
    selectedBlock = hit;
    draggingBlock = hit;

    if (hit >= 0)
        dragOffsetBars = xToBar(e.x) - blocks[hit].startBar;

    repaint();
}

void GrooveTimeline::mouseDrag(const juce::MouseEvent& e)
{
    if (draggingBlock < 0 || !rollRect.contains(e.getPosition())) return;

    int newBar = juce::jmax(1, xToBar(e.x) - dragOffsetBars);
    dragCurrentBar = newBar;
    repaint();
}

void GrooveTimeline::mouseUp(const juce::MouseEvent& /*e*/)
{
    if (draggingBlock >= 0)
    {
        // Snap: dragCurrentBar is already bar-aligned
        blocks[draggingBlock].startBar = dragCurrentBar;
        draggingBlock = -1;
        repaint();
    }
}

void GrooveTimeline::addBlock(const GrooveBlock& block)
{
    blocks.push_back(block);
    repaint();
}

void GrooveTimeline::clearBlocks()
{
    blocks.clear();
    selectedBlock = draggingBlock = -1;
    repaint();
}
