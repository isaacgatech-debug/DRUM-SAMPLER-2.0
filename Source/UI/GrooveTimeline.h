#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include "PluginColors.h"

/**
 * Persistent MIDI roll timeline — always visible below all tabs.
 *
 * Layout (top to bottom):
 *   28px  Track bar   [Track v][Block v] Track 1 [+]  [sel][cut][+][-]
 *   16px  Bar ruler   1  2  3  4  5 ...
 *   90px  Roll area   Coloured draggable groove blocks
 */
class GrooveTimeline : public juce::Component
{
public:
    struct GrooveBlock
    {
        int          startBar     = 1;
        int          durationBars = 2;
        int          type         = 0;   // 0=Verse 1=Fill 2=Chorus 3=PreChorus 4=Bridge
        juce::String name         = "Pattern";
    };

    GrooveTimeline();

    void paint    (juce::Graphics& g) override;
    void resized  () override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp  (const juce::MouseEvent& e) override;

    void addBlock(const GrooveBlock& block);
    void clearBlocks();
    const std::vector<GrooveBlock>& getBlocks() const { return blocks; }

    static constexpr int barWidth     = 64;  // px per bar
    static constexpr int trackBarH    = 28;
    static constexpr int rulerH       = 16;
    static constexpr int rollH        = 90;

private:
    void drawTrackBar (juce::Graphics& g, juce::Rectangle<float> area);
    void drawRuler    (juce::Graphics& g, juce::Rectangle<float> area);
    void drawRollArea (juce::Graphics& g, juce::Rectangle<float> area);
    void drawBlock    (juce::Graphics& g, const GrooveBlock& block,
                       juce::Rectangle<float> blockRect, bool selected);

    int  blockAtPoint(juce::Point<int> pt) const;
    int  xToBar      (int x) const;
    int  barToX      (int bar) const;

    std::vector<GrooveBlock> blocks;
    int selectedBlock  = -1;
    int draggingBlock  = -1;
    int dragOffsetBars = 0;
    int dragCurrentBar = 1;

    // Scroll offset in px
    int scrollX = 0;

    juce::TextButton trackDropdown {"Track 1"};
    juce::TextButton blockDropdown {"Block"};
    juce::TextButton addTrackBtn   {"+"};
    juce::TextButton selectBtn     {"SEL"};
    juce::TextButton cutBtn        {"CUT"};
    juce::TextButton zoomInBtn     {"+"};
    juce::TextButton zoomOutBtn    {"-"};

    juce::Rectangle<int> rollRect;   // roll area bounds for hit-testing

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveTimeline)
};
