# Kombai Handoff — Drum Tech Plugin UI Redesign
**Plugin:** Drum Tech by GridLock
**Codebase:** JUCE C++ (CMake build, macOS/Windows VST3/AU)
**Goal:** Complete UI/UX overhaul — from cartoon orange accent to next-gen sleek audio plugin aesthetic

---

## Context

This is a native JUCE C++ drum sampler plugin. A previous Kombai session (in the wrong Next.js
repo) produced all the UI design thinking and requirements below. The actual implementation must
happen here in C++/JUCE.

The engineer shared a **Superior Drummer 3** screenshot as the visual reference:
- Drum kit photo fills ~75% of the Drums tab (contained, not stretched)
- Right settings panel ~260px with DAW-style controls
- Tab bar at top with active underline
- MIDI roll always visible at the bottom with colored draggable groove blocks
- Transport bar at very bottom
- Deep dark palette, precision 1px borders, professional monospace typography

---

## Current Plugin State

Tabs (PluginEditor.h):
- tabKit     -> DrumKitView     (drum kit background image)
- tabGrooves -> GrooveBrowser   (groove library)
- tabMixer   -> MixerView       (12 channel strips)
- tabTrigger -> TriggerUI       (audio trigger engine)

Current colors — ALL to be replaced:
  bg       0xFF1A1A1A  -> 0xFF0D0D0E
  header   0xFF111111  -> 0xFF141416
  accent   0xFFE8A020  (ORANGE, engineer says "awful") -> 0xFF00C8FF (CYAN)
  textCol  0xFFCCCCCC  -> 0xFFDDE1E7

Existing features:
- TriggerUI: loadAudioFile(), drawWaveform(), drawTriggerMarkers(), thresholdSlider, drag-and-drop
- MixerView: 12 ChannelStrips, VUMeter.h, AnalogKnob.h — needs visual redesign
- DrumKitView: loads drum-backdrop.png — image currently STRETCHES, needs contain fix
- GrooveBrowser: needs persistent MIDI roll timeline added at the bottom

Resources:
  Resources/Backdrops/drum-backdrop.png
  Resources/LOGO/Gridlock Logo - White.png
  Resources/LOGO/gridlock-logo.png

---

## Design Tokens (juce::Colour hex values)

// Backgrounds
0xFF0D0D0E  plugin-bg           (deepest)
0xFF141416  plugin-panel        (top bar, side panels)
0xFF1C1C1F  plugin-surface      (channel strips, cards)
0xFF242428  plugin-surface-hi   (hover)

// Borders
0xFF2C2C32  plugin-border
0xFF3C3C44  plugin-border-hi

// Accent — REPLACES ALL ORANGE
0xFF00C8FF  plugin-accent       (cyan, primary interactive)
0xFF0088CC  plugin-accent-dim

// Text
0xFFDDE1E7  plugin-text
0xFF6B7280  plugin-text-muted
0xFF3A3A44  plugin-text-dim

// VU Meters (segmented, bottom to top)
0xFF22C55E  meter-green   (0-70%)
0xFFEAB308  meter-yellow  (70-85%)
0xFFEF4444  meter-red     (85-100%)

// MIDI Groove block colors (by type)
0xFF0EA5E9  groove-verse
0xFFF97316  groove-fill
0xFFA855F7  groove-chorus
0xFF22C55E  groove-pre-chorus
0xFFEC4899  groove-bridge

---

## Requirements

### 1. DrumKitView — Image Fix + Right Panel
- Fix image scaling: use RectanglePlacement::centred | onlyReduceInSize (object-contain behavior)
- Kit image takes full available area MINUS a 260px right settings panel
- Right panel (new InstrumentSettingsPanel component) contains:
  - Level slider with [0.0] readout, [S][M] buttons
  - Reverse toggle + 200ms readout
  - MIDI Monitor toggle
  - Collapsible "Voice and Layer" section: Layer Limits (Hard/Gradient/Soft each = 4), Voice Limit = 8
  - Soft Hit Level slider
  - Smoothing toggle (On/Off)
  - Hi-Hat CC Edit toggle
  - Level Envelope toggle
  - Velocity Gate toggle
  - Collapsible "MIDI Mapping" section: Open=36, Hit=35 rows with edit icon
  - "MIDI In/E-Drums Settings" button at bottom
- Interactive hotspot overlays on drum pieces (cyan glow ring on hover/select)

### 2. Colors (see Design Tokens above)
- Retire the orange (0xFFE8A020) everywhere — replace with cyan (0xFF00C8FF)
- All component backgrounds, borders, text to use the new tokens
- Monospace font for all numeric readouts:
  juce::Font(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::plain)

### 3. MIDI Roll (persistent strip below all tabs)
New components: GrooveTimeline.h/.cpp + TransportBar.h/.cpp

Layout (below main tab content, always visible):
  [Track dropdown][Block dropdown] Track 1  [+]   [select][cut][zoom+/-]   <- 28px track bar
  [bar ruler: 1  2  3  4  5  6  7  8 ... 30]                               <- 16px ruler
  [colored groove blocks — draggable to reposition]                         <- 90px roll area
  [loop][stop][play][record][tap]  4/4  120bpm  001:01:000  [master vol]   <- 44px transport

Groove blocks:
- Positioned at (startBar - 1) * 64px, width = durationBars * 64px
- Color by type (see tokens above)
- Shows pattern name + type label (e.g., "Hats Tight Tip" / "Verse")
- Draggable via mouseDown/mouseDrag/mouseUp, snap to nearest bar on mouseUp
- Mini waveform decoration inside the block

### 4. TriggerUI — Major Expansion (per-mic channels + multiband freq editor)
Replace the current single-file trigger UI with a grid of TriggerChannel cards (2-3 per row).

Each TriggerChannel card:
  - Header: mic name, Import Audio button, Clear button
  - Waveform area (72px): shows waveform when file loaded, drop zone when empty
    Waveform color: 0xFF00C8FF (cyan), progress: 0xFF0088CC
  - Playback transport (play/pause, filename display)
  - Volume Threshold slider (0-100%, with readout)
  - FrequencyBandEditor (new component, see below)

Mic channels: Kick In, Kick Out, Snare Top, Snare Bot, Hi-Hat, Tom 1, Tom 2, Tom 3

FrequencyBandEditor (new component Source/UI/FrequencyBandEditor.h/.cpp):
- 20Hz to 20kHz on LOG scale (x-axis)
- 72px tall visual display: dark bg, 1px gridlines at 20/50/100/200/500/1k/2k/5k/10k/20kHz
- Colored band rectangles (from freqLow% to freqHigh% on log scale)
- Each band has a horizontal threshold line inside it
- Click band to select it; selected band has bright border
- Below the display: freq axis labels (20, 50, 100, 200, 500, 1k, 2k, 5k, 10k, 20k)
- Band list rows: [toggle] [color swatch] "50 – 150 Hz" "60%" [delete btn]
- Selected band edit controls: Freq Low slider, Freq High slider, Threshold slider
- Add Band button (generates new band with next color in palette)
- PURPOSE: Like multiband compressor — each band monitors its freq range independently.
  Enables ghost note detection — a faint hit at 200Hz can trigger while ignoring bleed
  from cymbals at 8kHz.

Log-scale math:
  freqToPercent(f) = log10(f/20) / log10(20000/20) * 100
  percentToFreq(p) = 20 * pow(1000, p/100)

### 5. MixerView — Analog Console Redesign
Engineer: "cramped and horrible" -> "modern analog console, can be large, prefer gorgeous"

Minimum window width: 1400px. Use setResizeLimits(1200, 800, 4000, 2000) in PluginEditor.

Each channel strip (~88px wide, full mixer height):
  Top color bar (3px, channel-specific color with glow shadow)
  Short name + full name header
  Mini 4-band EQ curve display (polyline SVG-style, 56x24px)
  FX1 / FX2 send knobs (use existing AnalogKnob, 20px diameter)
  [S] Solo button: yellow (0xFFEAB308) when active
  [M] Mute button: red (0xFFEF4444) when active
  VU meter (18 segments, animated, flex-column-reverse so bottom fills first)
  Vertical fader (main level control)
  dB readout below fader (monospace font)
  Pan slider (horizontal, -100 to +100, center = C, right = R50 etc.)

Master channel: 100px wide, pinned right, separated by 2px border
Left section labels sidebar (80px): INPUT, EQ, SENDS, S/M, VU, FADER, PAN
Horizontal scroll for input channels (viewport component)

Channel colors (by channel index):
  0: 0xFFEF4444 (Kick In), 1: 0xFFF97316 (Kick Out), 2: 0xFFEAB308 (Snare Top)
  3: 0xFF84CC16 (Snare Bot), 4: 0xFF22C55E (Hi-Hat), 5: 0xFF06B6D4 (Tom 1)
  6: 0xFF3B82F6 (Tom 2), 7: 0xFF6366F1 (Tom 3), 8: 0xFF8B5CF6 (OVH L)
  9: 0xFFA855F7 (OVH R), 10: 0xFFEC4899 (Room L), 11: 0xFFEC4899 (Room R)
  Master: 0xFF00C8FF (cyan)

### 6. Branding
- Plugin title in header: "GRIDLOCK" (white, Bebas/bold) + divider + "DRUM TECH" (cyan)
- Use existing gridlock-logo.png as a 20x20px icon in top-left
- Remove the "BUGS" button from the main header (move to Ctrl+Shift+D shortcut or hidden)
- Remove "LOAD SAMPLES" button — replace with kit selector dropdown in instrument bar
- Tab labels: KIT / MIXER / TRIGGER / GROOVES (uppercase, 11px bold, tracking 0.1em)
- Active tab: cyan underline + slight cyan background tint

---

## Layout Blueprint (ASCII)

+----------------------------------------------+--------------------+
| [G] GRIDLOCK | DRUM TECH  [KIT][MIXER][TRIGGER][GROOVES]  [Kit v] | <- TopNav 44px
+----------------------------------------------+--------------------+
| [+Instrument v][Kick v][16x24" DW v][More v]  [All Articulations] | <- InstrumentBar 36px
+----------------------------------------------+--------------------+
|                                              | Level      0.0 S M |
|                                              | ----------slider-- |
|    DRUM KIT IMAGE (object-contain)           | Reverse 200ms [t]  |
|                                              | MIDI Monitor  [t]  |
|    Clickable hotspot overlays                | v Voice and Layer  |
|    (cyan ring on hover)                      |   Hard 4 Grad 4    |
|                                              |   Soft 4  VoiceLm8 |
|                                              | Soft Hit Level 0.0 |
|                                              | Smoothing  Off [t] |
|                                              | Hi-Hat CC Edit [t] |
|                                              | Level Env     [t]  |
|                                              | Velocity Gate [t]  |
|                                              | v MIDI Mapping     |
|                                              | Open  36  =        |
|                                              | Hit   35  =        |
+----------------------------------------------+--------------------+
| [Track v][Block v] Track 1 [+]         [sel][cut][+][-]           | <- Track bar 28px
| 1   2   3   4   5   6   7   8   9   10  11  12  13  14 ...        | <- Ruler 16px
| [Verse/Hats Tight Tip] [Fill][Pre Chorus] [Fill][Chorus]          | <- MIDI roll 90px
| [loop][stop][play][rec][tap] 4/4  120  001:01:000  [vol]          | <- Transport 44px
+------------------------------------------------------------------------+

---

## Files to Modify / Create

MODIFY:
  Source/Core/PluginEditor.h/.cpp   -> Branding, window size, tab styling
  Source/UI/DrumKitView.h/.cpp      -> Image contain fix, right settings panel
  Source/UI/MixerView.h/.cpp        -> Spacious analog console redesign
  Source/UI/ChannelStrip.h/.cpp     -> Cyan accent, new colors, segment VU
  Source/UI/VUMeter.h/.cpp          -> Green/yellow/red segments
  Source/Trigger/TriggerUI.h/.cpp   -> Per-channel grid, FrequencyBandEditor

CREATE:
  Source/UI/FrequencyBandEditor.h/.cpp   -> Log-scale multiband freq threshold
  Source/UI/GrooveTimeline.h/.cpp        -> Persistent MIDI roll with draggable blocks
  Source/UI/TransportBar.h/.cpp          -> Bottom transport strip
  Source/UI/InstrumentSettingsPanel.h/.cpp -> Right panel in DrumKitView

DO NOT:
  - Use orange (0xFFE8A020) anywhere
  - Stretch the drum kit image
  - Keep BUGS button in main header
  - Make channel strips narrower than current

---

*Kombai session handoff — 2026-03-24*
