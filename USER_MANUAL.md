# Drum Tech - User Manual

**Version**: 2.0.0  
**Date**: March 21, 2026  
**Platform**: macOS (AU, VST3, Standalone)

---

## Table of Contents

1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Quick Start](#quick-start)
4. [User Interface Overview](#user-interface-overview)
5. [Kit Tab](#kit-tab)
6. [Grooves Tab](#grooves-tab)
7. [Mixer Tab](#mixer-tab)
8. [Trigger Tab](#trigger-tab)
9. [Routing Tab](#routing-tab)
10. [Preset Management](#preset-management)
11. [Keyboard Shortcuts](#keyboard-shortcuts)
12. [Troubleshooting](#troubleshooting)
13. [Technical Specifications](#technical-specifications)

---

## Introduction

Drum Tech is a professional drum production tool that combines:

- **Advanced Sampler Engine**: 64 voices, velocity layers, round-robin
- **Groove Library**: MIDI pattern management with tap-to-find search
- **Audio Trigger Engine**: Convert audio recordings to MIDI
- **Professional Mixer**: 11 channels with built-in effects and plugin hosting
- **Multi-Bus Routing**: 32 configurable buses with DAW integration

### Key Features

✅ Velocity-sensitive drum pads with ADSR envelopes  
✅ Built-in effects: Reverb, Delay, EQ, Compressor, Transient Shaper  
✅ 3rd party VST3/AU plugin hosting  
✅ MIDI groove library with search and preview  
✅ Audio-to-MIDI conversion with drum classification  
✅ Multi-bus routing with send/return FX  
✅ Comprehensive preset management  
✅ Full keyboard shortcut support  

---

## Installation

### System Requirements

- **macOS**: 10.13 or later
- **CPU**: Intel Core i5 or better (Apple Silicon supported)
- **RAM**: 4 GB minimum, 8 GB recommended
- **Disk Space**: 500 MB for plugin, additional space for samples
- **DAW**: Any AU or VST3 compatible host

### Installation Steps

1. **Download** the installer from the official website
2. **Run** the installer package
3. **Select** installation location:
   - AU: `/Library/Audio/Plug-Ins/Components/`
   - VST3: `/Library/Audio/Plug-Ins/VST3/`
   - Standalone: `/Applications/`
4. **Restart** your DAW
5. **Scan** for new plugins in your DAW

### First Launch

1. Open Drum Tech in your DAW or as standalone
2. Click **"LOAD SAMPLES"** to select your drum sample folder
3. The plugin will scan and load all compatible audio files
4. Start playing!

---

## Quick Start

### 5-Minute Workflow

1. **Load Samples**: Click "LOAD SAMPLES" and select a folder containing drum samples
2. **Play**: Use MIDI keyboard or click drum pads with mouse
3. **Adjust**: Tweak volume, pan, and effects in the Mixer tab
4. **Add Groove**: Go to Grooves tab, search for a pattern, double-click to preview
5. **Save**: Press `Cmd+S` to save your kit as a preset

---

## User Interface Overview

### Main Window

```
┌─────────────────────────────────────────────────────┐
│ DRUM SAMPLER 2.0        [LOAD SAMPLES]      [BUGS] │
├─────────────────────────────────────────────────────┤
│ [KIT] [GROOVES] [MIXER] [TRIGGER] [ROUTING]        │
├─────────────────────────────────────────────────────┤
│                                                     │
│              Active Tab Content                     │
│                                                     │
└─────────────────────────────────────────────────────┘
```

### Navigation

- **Tab Buttons**: Click to switch between different views
- **Keyboard Shortcuts**: `Cmd+1` through `Cmd+5` for quick tab access
- **Tab Key**: Cycle through tabs

---

## Kit Tab

### Drum Pads

**11 Drum Pads**:
- Kick (C1 / MIDI 36)
- Snare (D1 / MIDI 38)
- Hi-Hat (F#1 / MIDI 42)
- Tom 1 (A1 / MIDI 45)
- Tom 2 (C2 / MIDI 48)
- Tom 3 (D2 / MIDI 50)
- Crash (C#2 / MIDI 49)
- Ride (D#2 / MIDI 51)
- Clap (D#1 / MIDI 39)
- Rim (C#1 / MIDI 37)
- Cowbell (G#2 / MIDI 56)

### Pad Controls

Each pad has:
- **Sample Name**: Click to load different sample
- **Volume**: Adjust pad volume
- **Pan**: Stereo positioning
- **Tune**: Pitch adjustment (-12 to +12 semitones)
- **ADSR**: Attack, Decay, Sustain, Release envelope

### Loading Samples

1. Click on a drum pad
2. Click "Load Sample" button
3. Select audio file (WAV, AIFF, MP3)
4. Sample is automatically mapped to the pad

---

## Grooves Tab

### Groove Browser

**Search and Filter**:
- **Search Box**: Type to filter by name
- **Category**: Filter by genre (Rock, Jazz, Funk, etc.)
- **Tempo Range**: Set min/max BPM (60-200)
- **Favorites Only**: Show only starred grooves

### Preview Playback

1. **Single-click**: Select groove
2. **Double-click**: Start preview playback
3. **Preview Button**: Play selected groove
4. **Stop**: Click preview button again or press Space

### Adding to Timeline

1. Select a groove from the list
2. Click "Add to Timeline"
3. Groove appears in timeline view
4. Drag to reposition

### Tap-to-Find

1. Click "Tap" button
2. Tap rhythm on drum pads or keyboard
3. Click "Find Matches"
4. View grooves sorted by similarity

---

## Mixer Tab

### Channel Strip

Each of 11 channels has:
- **Fader**: Volume control (-∞ to +6 dB)
- **Pan**: Stereo position (L-C-R)
- **Mute**: Silence channel
- **Solo**: Hear only this channel
- **Phase**: Invert phase (180°)

### Insert Effects

**Built-in Effects**:
1. **Reverb**: Room, Hall, Plate algorithms
2. **Delay**: Tempo-synced or manual time
3. **EQ**: 3-band parametric (Low shelf, Mid peak, High shelf)
4. **Compressor**: Threshold, ratio, attack, release
5. **Transient Shaper**: Attack/sustain control

**Adding Effects**:
1. Click "+" button in insert slot
2. Select effect type
3. Adjust parameters
4. Click "X" to remove

### 3rd Party Plugins

1. Click "Scan Plugins" to find installed VST3/AU plugins
2. Click "+" in insert slot
3. Select "Plugin..." from menu
4. Choose from available plugins
5. Plugin editor opens automatically

### Send Effects

**4 Send Buses**:
- Send 1-4 available on each channel
- Adjust send level (0-100%)
- Each send bus can host one effect
- Return level controls mix back to master

---

## Trigger Tab

### Audio-to-MIDI Conversion

**Workflow**:
1. **Load Audio**: Click "Load Audio" or drag file
2. **Adjust Threshold**: Set detection sensitivity (0.0-1.0)
3. **Bleed Suppression**: Reduce false triggers (0.0-1.0)
4. **Process**: Click "Process" to detect hits
5. **Review**: Check waveform with trigger markers
6. **Export**: Click "Export MIDI" to save

### Parameters

- **Threshold**: Lower = more sensitive, higher = less sensitive
- **Bleed Suppression**: Higher = more aggressive filtering
- **Sensitivity**: Overall detection sensitivity multiplier
- **Min Time Between Hits**: Prevent double-triggering (ms)

### Drum Classification

Automatically classifies detected hits as:
- Kick (low frequency, high energy)
- Snare (mid frequency, moderate brightness)
- Hi-Hat (high frequency, high zero-crossing rate)
- Tom (mid-low frequency)
- Crash (high frequency, high energy)
- Ride (mid-high frequency)

---

## Routing Tab

### Output Buses

**32 Configurable Buses**:
- Mono or Stereo
- Independent volume control
- Link to DAW outputs
- Custom naming

### Routing Channels

1. Select channel in mixer
2. Go to Routing tab
3. Choose output bus (1-32)
4. Adjust bus volume
5. Enable "Link to DAW" for multi-output

### Send/Return Configuration

1. Select send bus (1-4)
2. Choose output destination
3. Set return level
4. Add effect to send bus

---

## Preset Management

### Saving Presets

1. Configure your kit (samples, mixer, effects, routing)
2. Press `Cmd+S` or click "Save Preset"
3. Enter preset name
4. Choose category
5. Click "Save"

**Preset Location**: `~/Library/Application Support/DrumTech/Presets/`

### Loading Presets

1. Press `Cmd+O` or click "Load Preset"
2. Browse presets by category
3. Click preset to load
4. All settings are restored

### Categories

- Rock
- Jazz
- Electronic
- Hip Hop
- Metal
- Custom (user-created)

---

## Keyboard Shortcuts

### Playback
- `Space` - Play/Stop
- `Cmd+R` - Record

### Navigation
- `Tab` - Next Tab
- `Shift+Tab` - Previous Tab
- `Cmd+1` - Kit Tab
- `Cmd+2` - Grooves Tab
- `Cmd+3` - Mixer Tab
- `Cmd+4` - Trigger Tab
- `Cmd+5` - Routing Tab

### File Operations
- `Cmd+S` - Save Preset
- `Cmd+O` - Load Preset
- `Cmd+E` - Export MIDI
- `Cmd+I` - Import Audio

### Editing
- `Cmd+Z` - Undo
- `Cmd+Shift+Z` - Redo
- `Cmd+C` - Copy
- `Cmd+V` - Paste
- `Delete` - Delete

### View
- `Cmd+D` - Toggle Debug Console
- `Cmd+=` - Zoom In
- `Cmd+-` - Zoom Out

### Mixer
- `Cmd+M` - Mute All
- `Cmd+Shift+S` - Solo All
- `Cmd+Shift+R` - Reset Mixer

### Help
- `F1` - Show Help
- `Cmd+,` - Preferences

---

## Troubleshooting

### No Sound

**Check**:
1. Sample loaded on pad?
2. Channel muted?
3. Master volume up?
4. DAW track armed/monitoring?
5. Audio interface connected?

### High CPU Usage

**Solutions**:
1. Increase buffer size in DAW
2. Reduce polyphony (voice count)
3. Disable unused effects
4. Use freeze/bounce tracks
5. Close other applications

### Crackling/Dropouts

**Fixes**:
1. Increase audio buffer size
2. Disable WiFi/Bluetooth
3. Close background apps
4. Check CPU usage in Activity Monitor
5. Update audio drivers

### Plugin Not Found in DAW

**Steps**:
1. Verify installation location
2. Rescan plugins in DAW
3. Check DAW plugin format (AU vs VST3)
4. Restart DAW
5. Check macOS security settings

### Samples Won't Load

**Verify**:
1. File format supported (WAV, AIFF, MP3)
2. Sample rate compatible (44.1/48/96 kHz)
3. File not corrupted
4. Sufficient disk space
5. File permissions correct

---

## Technical Specifications

### Audio Engine
- **Sample Rate**: 44.1, 48, 88.2, 96 kHz
- **Bit Depth**: 16, 24, 32-bit float
- **Polyphony**: 64 voices
- **Latency**: <10ms (real-time mode)

### Effects
- **Reverb**: Algorithmic, 32-bit processing
- **Delay**: Stereo, tempo-synced, up to 2 seconds
- **EQ**: 3-band parametric, 20Hz-20kHz
- **Compressor**: RMS/Peak detection, 1:1 to 20:1 ratio
- **Transient Shaper**: Attack/Sustain model

### Formats
- **Plugin**: AU, VST3
- **Standalone**: macOS application
- **Preset**: XML-based (.ds2preset)
- **MIDI**: Standard MIDI File (SMF)

### Performance
- **CPU Usage**: 2-5% (typical)
- **RAM Usage**: 100-500 MB (depends on samples)
- **Disk I/O**: Minimal (samples loaded to RAM)

---

## Support

### Resources
- **Website**: www.drumtech.com
- **Email**: support@drumtech.com
- **Forum**: forum.drumtech.com
- **Video Tutorials**: youtube.com/drumtech

### Reporting Bugs
1. Click "BUGS" button in plugin
2. Review error log
3. Click "Export Log"
4. Email log file to support

---

**© 2026 Drum Tech. All rights reserved.**
