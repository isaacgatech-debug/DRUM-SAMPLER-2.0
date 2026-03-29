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

    trackNameLabel.setText(trackDropdown.getButtonText(), juce::dontSendNotification);
    trackNameLabel.setFont(PluginFonts::label(12.0f));
    trackNameLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textPrimary));
    trackNameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(trackNameLabel);

    trackDropdown.onClick = [this]
    {
        syncTrackNameLabel();
    };

    zoomInBtn.onClick = [this]
    {
        scrollX = juce::jmax(0, scrollX - barWidth);
        repaint();
    };
    zoomOutBtn.onClick = [this]
    {
        scrollX += barWidth;
        repaint();
    };
    cutBtn.onClick = [this]
    {
        if (selectedBlock >= 0 && selectedBlock < static_cast<int>(blocks.size()))
        {
            blocks.erase(blocks.begin() + selectedBlock);
            selectedBlock = -1;
            repaint();
        }
    };
    addTrackBtn.onClick = [this]
    {
        const int nextBar = blocks.empty() ? 1 : blocks.back().startBar + blocks.back().durationBars;
        addBlock({ nextBar, 4, 0, "New Pattern" });
    };
    selectBtn.onClick = [this]
    {
        selectedBlock = blockAtPoint({ barToX(1), rollRect.getCentreY() });
        repaint();
    };

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

    auto trackBarBounds = area.removeFromTop(trackBarH);
    auto trackBarArea   = trackBarBounds;
    trackDropdown.setBounds(trackBarArea.removeFromLeft(96).reduced(3));
    blockDropdown.setBounds(trackBarArea.removeFromLeft(84).reduced(3));
    trackBarArea.removeFromLeft(6);
    addTrackBtn.setBounds(trackBarArea.removeFromLeft(34).reduced(3));
    selectBtn.setBounds(trackBarArea.removeFromRight(44).reduced(3));
    cutBtn.setBounds(trackBarArea.removeFromRight(44).reduced(3));
    zoomInBtn.setBounds(trackBarArea.removeFromRight(40).reduced(3));
    zoomOutBtn.setBounds(trackBarArea.removeFromRight(40).reduced(3));

    const int gapL = addTrackBtn.getRight() + 6;
    const int gapR = zoomOutBtn.getX() - 6;
    if (gapR > gapL + 8)
    {
        trackNameLabel.setVisible(true);
        trackNameLabel.setBounds(gapL, trackBarBounds.getY() + 2, gapR - gapL, trackBarBounds.getHeight() - 4);
    }
    else
    {
        trackNameLabel.setVisible(false);
        trackNameLabel.setBounds(0, 0, 0, 0);
    }

    syncTrackNameLabel();

    area.removeFromTop(rulerH);   // ruler is painted
    rollRect = area.removeFromTop(rollH);
}

void GrooveTimeline::syncTrackNameLabel()
{
    trackNameLabel.setText(trackDropdown.getButtonText(), juce::dontSendNotification);
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

    drawRuler(g, rulerArea);
    drawRollArea(g, rollArea);
}

void GrooveTimeline::drawRuler(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(juce::Colour(PluginColors::pluginSurface));
    g.fillRect(area);
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawHorizontalLine(static_cast<int>(area.getBottom()) - 1, area.getX(), area.getRight());

    g.setFont(PluginFonts::mono(11.0f));
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

    auto inner = rect.reduced(6.0f, 4.0f);
    constexpr float kMinWTwoLines = 104.0f;
    constexpr float kNameH          = 15.0f;
    constexpr float kTypeH          = 13.0f;

    if (inner.getWidth() < kMinWTwoLines || inner.getHeight() < kNameH + kTypeH + 6.0f)
    {
        g.setFont(PluginFonts::label(11.0f));
        g.setColour(col.brighter(0.4f));
        g.drawText(block.name, inner, juce::Justification::centredLeft, true);
        return;
    }

    auto nameR = inner.removeFromTop(kNameH);
    auto typeR = inner.removeFromBottom(kTypeH);

    // Mini waveform in remaining middle band only (no overlap with text)
    if (inner.getWidth() > 8.0f && inner.getHeight() > 4.0f)
    {
        juce::Path waveform;
        const float wy = inner.getCentreY();
        const float amp  = juce::jmin(inner.getHeight() * 0.35f, 10.0f);
        int steps = juce::jmax(2, static_cast<int>(inner.getWidth() / 4));
        waveform.startNewSubPath(inner.getX() + 2, wy);
        for (int s = 0; s < steps; ++s)
        {
            float sx = inner.getX() + 2 + s * (inner.getWidth() - 4) / juce::jmax(1, steps - 1);
            float sy = wy + amp * std::sin((float)s * 0.7f + (float)block.startBar);
            waveform.lineTo(sx, sy);
        }
        g.setColour(col.withAlpha(0.35f));
        g.strokePath(waveform, juce::PathStrokeType(1.0f));
    }

    g.setFont(PluginFonts::label(11.0f));
    g.setColour(col.brighter(0.4f));
    g.drawText(block.name, nameR, juce::Justification::centredLeft, true);

    g.setFont(PluginFonts::mono(9.5f));
    g.setColour(col.withAlpha(0.85f));
    g.drawText(PluginColors::grooveTypeName(block.type), typeR, juce::Justification::centredLeft, true);
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

void GrooveTimeline::mouseMove(const juce::MouseEvent& e)
{
    if (!rollRect.contains(e.getPosition()))
    {
        setTooltip({});
        return;
    }

    const int i = blockAtPoint(e.getPosition());
    if (i >= 0 && i < static_cast<int>(blocks.size()))
    {
        const auto& b = blocks[static_cast<size_t>(i)];
        setTooltip(b.name + " — " + PluginColors::grooveTypeName(b.type));
    }
    else
    {
        setTooltip({});
    }
}

void GrooveTimeline::mouseExit(const juce::MouseEvent&)
{
    setTooltip({});
}

juce::MidiMessageSequence GrooveTimeline::buildSequenceForGrid() const
{
    juce::MidiMessageSequence seq;
    for (const auto& b : blocks)
    {
        for (int bar = 0; bar < b.durationBars; ++bar)
        {
            const double barStartBeat = static_cast<double>((b.startBar - 1 + bar) * 4);
            for (int step = 0; step < 16; ++step)
            {
                const double beat = barStartBeat + (static_cast<double>(step) * 0.25);
                int note = 42;
                int vel = 70;

                if (b.type == 1) // fill
                {
                    note = (step % 4 == 0) ? 38 : 45;
                    vel = 78 + (step % 8) * 5;
                }
                else if (b.type == 2) // chorus
                {
                    note = (step % 8 == 0) ? 36 : ((step % 4 == 0) ? 38 : 42);
                    vel = (step % 4 == 0) ? 98 : 74;
                }
                else if (b.type == 3) // pre-chorus
                {
                    note = (step % 4 == 2) ? 38 : 42;
                    vel = 78;
                }
                else
                {
                    note = (step % 8 == 0) ? 36 : ((step % 4 == 0) ? 38 : 42);
                    vel = (step % 4 == 0) ? 86 : 66;
                }

                // Keep first v1 pass simple: 16th-note grid playback.
                const auto on = juce::MidiMessage::noteOn(1, note, static_cast<juce::uint8>(juce::jlimit(1, 127, vel)));
                const auto off = juce::MidiMessage::noteOff(1, note);
                auto onMsg = on;
                auto offMsg = off;
                onMsg.setTimeStamp(beat);
                offMsg.setTimeStamp(beat + 0.12);
                seq.addEvent(onMsg);
                seq.addEvent(offMsg);
            }
        }
    }
    seq.updateMatchedPairs();
    return seq;
}
