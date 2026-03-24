# Architecture Documentation

## Overview

Drum Library is a JUCE-based drum sampler plugin with three main components:

1. **SamplerEngine** - Sample loading, voice management, and playback
2. **PluginProcessor** - Audio processing and state management
3. **PluginEditor** - User interface and visual feedback

## Component Details

### SamplerEngine

**File**: `Source/SamplerEngine.h`, `Source/SamplerEngine.cpp`

#### Responsibilities
- Load audio samples from disk
- Parse filenames to extract MIDI note, velocity range, and round-robin group
- Manage 64 polyphonic voices
- Handle note-on events with velocity and round-robin selection
- Mix voices into the output buffer

#### Key Classes

**DrumSample**
```cpp
struct DrumSample {
    juce::AudioBuffer<float> buffer;  // Sample audio data
    int sampleRate;                   // Original sample rate
    int midiNote;                     // MIDI note number (0-127)
    int rrGroup;                      // Round-robin group (0-based)
    int velLow;                       // Velocity range low (0-127)
    int velHigh;                      // Velocity range high (0-127)
};
```

**DrumVoice**
- Plays a single sample instance
- Tracks playback position
- Applies velocity-based gain
- Auto-stops when sample completes

**SamplerEngine**
- Manages `std::vector<DrumSample>` for all loaded samples
- Maintains 64 `DrumVoice` instances
- Tracks round-robin counters per MIDI note (128 counters)

#### Sample Loading Algorithm

```
1. Scan folder for .wav, .aif, .aiff files
2. For each file:
   a. Parse filename for _n<note>_v<low>-<high>_rr<group> pattern
   b. If no explicit note, match keywords (kick→36, snare→38, etc.)
   c. Load audio data via AudioFormatManager
   d. Store in samples vector
3. Samples remain in memory until new folder is loaded
```

#### Note-On Algorithm

```
1. Receive MIDI note and velocity
2. Filter samples matching:
   - sample.midiNote == note
   - velocity >= sample.velLow
   - velocity <= sample.velHigh
3. Select sample using round-robin:
   - candidates[rrCounter % candidates.size()]
   - Increment rrCounter for this note
4. Find free voice (or steal oldest)
5. Trigger voice with selected sample and velocity gain
```

#### Voice Rendering

```
For each voice in processBlock():
  1. Check if voice is active
  2. Calculate samples remaining in source buffer
  3. Copy/mix into output buffer with gain
  4. Advance playback position
  5. Mark inactive when sample completes
```

### PluginProcessor

**File**: `Source/PluginProcessor.h`, `Source/PluginProcessor.cpp`

#### Responsibilities
- Implement JUCE AudioProcessor interface
- Route MIDI and audio to SamplerEngine
- Manage plugin state (sample folder path)
- Create editor instance

#### Key Methods

**prepareToPlay**
```cpp
void prepareToPlay(double sampleRate, int samplesPerBlock) {
    engine.prepareToPlay(sampleRate, samplesPerBlock);
}
```

**processBlock**
```cpp
void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi) {
    buffer.clear();  // Clear output
    engine.processBlock(buffer, midi);  // Render samples
}
```

**State Management**
```cpp
void getStateInformation(MemoryBlock& dest) {
    // Save: lastLoadedFolder path
}

void setStateInformation(const void* data, int size) {
    // Restore: reload samples from saved path
}
```

### PluginEditor

**File**: `Source/PluginEditor.h`, `Source/PluginEditor.cpp`

#### Responsibilities
- Display UI with dark theme
- Render kit view with 11 drum pad zones
- Handle tab navigation (KIT/MIXER/FX)
- Provide sample loading interface
- Show status messages

#### UI Components

**Layout** (900×580px default)
```
┌─────────────────────────────────────────────┐
│ Header (56px)                               │
│ ┌─────────────┐              ┌────────────┐ │
│ │ DRUM LIBRARY│              │LOAD SAMPLES│ │
│ └─────────────┘              └────────────┘ │
├─────────────────────────────────────────────┤
│ Tab Bar (36px)                              │
│ [KIT] [MIXER] [FX]                          │
├─────────────────────────────────────────────┤
│                                             │
│ Content Area (488px)                        │
│                                             │
│ Kit View:                                   │
│   ○ Crash1    ○ Splash    ○ Crash2         │
│      ○ Tom1  ○ Tom2  ○ Tom3  ○ Tom4        │
│         ○ HH                                │
│            ○ Snare                          │
│               ○ Kick                        │
│                                             │
└─────────────────────────────────────────────┘
```

**KitView Class**
- Paints drum kit layout with ellipses
- 11 zones with positions, sizes, and labels
- Cymbal zones use gold color, drums use bronze
- Flash effect on MIDI note (not yet implemented)

**Color Palette**
```cpp
juce::Colour bg      { 0xFF1A1A1A };  // Dark gray
juce::Colour header  { 0xFF111111 };  // Darker gray
juce::Colour accent  { 0xFFE8A020 };  // Orange
juce::Colour textCol { 0xFFCCCCCC };  // Light gray
```

**Drum Pad Zones**
```cpp
const Zone zones[] = {
    { "Kick",   36, 0.38f, 0.76f, 0.16f, 0.12f, false },
    { "Snare",  38, 0.24f, 0.62f, 0.13f, 0.10f, false },
    { "HH",     42, 0.13f, 0.46f, 0.10f, 0.07f, true  },
    // ... 8 more zones
};
```
Positions are normalized (0.0-1.0) relative to component size.

#### File Chooser (JUCE 8 Async API)
```cpp
auto chooser = std::make_shared<FileChooser>(...);
chooser->launchAsync(flags, [this, chooser](const FileChooser& fc) {
    auto folder = fc.getResult();
    processor.loadSamplesFromFolder(folder);
    updateStatus();
});
```

## Data Flow

### Sample Loading
```
User clicks LOAD SAMPLES
  → FileChooser dialog opens
    → User selects folder
      → PluginProcessor.loadSamplesFromFolder()
        → SamplerEngine.loadSamplesFromFolder()
          → Scan files, parse names, load audio
            → Store in samples vector
              → PluginEditor.updateStatus()
```

### MIDI Playback
```
DAW sends MIDI note-on
  → PluginProcessor.processBlock(buffer, midi)
    → SamplerEngine.processBlock(buffer, midi)
      → For each MIDI message:
        → noteOn(note, velocity)
          → Find matching samples
            → Select via round-robin
              → Trigger voice
                → Voice.process() mixes into buffer
```

### Audio Output
```
SamplerEngine.processBlock()
  → For each active voice:
    → voice.process(buffer, startSample, numSamples)
      → Read from sample.buffer
        → Apply gain
          → Mix into output buffer
```

## Build System

### CMakeLists.txt Structure

```cmake
# 1. Project setup
cmake_minimum_required(VERSION 3.22)
project(DrumLibrary VERSION 1.0.0)

# 2. Add JUCE
add_subdirectory(/tmp/JUCE-8.0.4 jucelib)

# 3. Define plugin
juce_add_plugin(DrumLibrary
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Drum Library"
    # ... plugin metadata
)

# 4. Add source files
target_sources(DrumLibrary PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SamplerEngine.cpp
)

# 5. Compile definitions
target_compile_definitions(DrumLibrary PUBLIC
    JUCE_COREGRAPHICS_DRAW_ASYNC=0  # Fix Logic AU rendering
    # ... other flags
)

# 6. Link JUCE modules
target_link_libraries(DrumLibrary PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_gui_basics
    # ... other modules
)
```

### Build Targets

- **DrumLibrary** - Shared code library
- **DrumLibrary_AU** - Audio Unit plugin
- **DrumLibrary_VST3** - VST3 plugin
- **DrumLibrary_Standalone** - Standalone application

### Output Locations

```
build/DrumLibrary_artefacts/Release/
├── AU/
│   └── Drum Library.component/
├── VST3/
│   └── Drum Library.vst3/
└── Standalone/
    └── Drum Library.app/
```

## Performance Considerations

### Memory Usage
- Each sample stored in RAM (no streaming)
- Typical kit: ~50-200 samples × 1-5 seconds × 44.1kHz × 2 channels
- Estimated: 50-500 MB per kit

### CPU Usage
- Voice rendering: O(activeVoices) per buffer
- Sample lookup: O(samplesMatchingNote) per note-on
- Typical: <5% CPU for 64 voices at 512 sample buffer

### Optimization Opportunities
1. **Sample streaming** - Load only active regions into RAM
2. **Voice pooling** - Reuse voice objects instead of searching
3. **SIMD mixing** - Vectorize voice rendering loop
4. **Lazy loading** - Load samples on first trigger, not all at once

## Known Issues & Limitations

### Logic Pro AU Blank UI (macOS 15)
**Symptom**: Plugin validates but shows blank window in Logic Pro  
**Cause**: JUCE 8.0.4 AU wrapper incompatibility with Logic's NSView hosting  
**Workaround**: Use Standalone or VST3 version  
**Fix**: Awaiting JUCE 8.0.5+ update  

### No Sample Streaming
**Impact**: Large kits consume significant RAM  
**Mitigation**: Users should load only needed samples  
**Future**: Implement JUCE's AudioFormatReaderSource streaming  

### No Mixer/FX Pages
**Status**: UI placeholders exist, functionality not implemented  
**Roadmap**: v2.0 will add per-channel volume/pan and built-in reverb/delay  

## Extension Points

### Adding New UI Pages

1. Create new Component class (e.g., `MixerPage`)
2. Add to `PluginEditor.h` as member
3. Add tab button in constructor
4. Implement `resized()` to position it
5. Add visibility toggle in tab click handler

### Adding Effects

1. Add `juce::dsp` processor to `PluginProcessor`
2. Call `prepare()` in `prepareToPlay()`
3. Process in `processBlock()` after sampler
4. Add UI controls in `PluginEditor`
5. Link controls to processor parameters

### Supporting New Sample Formats

1. JUCE's `AudioFormatManager` already supports WAV/AIFF/FLAC/OGG
2. For custom formats, implement `AudioFormat` subclass
3. Register with `formatManager.registerFormat()`

## Testing Strategy

### Unit Tests (Future)
- `SamplerEngine::loadSamplesFromFolder()` with mock files
- `SamplerEngine::noteOn()` velocity/RR selection
- `DrumVoice::process()` gain application

### Integration Tests
- Load real sample folder, verify count
- Trigger notes, verify audio output
- State save/restore cycle

### Manual Testing Checklist
- [ ] Load samples from folder
- [ ] Trigger all 11 drum pads via MIDI
- [ ] Verify velocity response (soft/loud)
- [ ] Verify round-robin variation
- [ ] Save/reload project in DAW
- [ ] Resize window (600×400 to 1400×900)
- [ ] Switch between KIT/MIXER/FX tabs

## Future Roadmap

### v1.1
- [ ] Fix Logic Pro AU compatibility
- [ ] Add MIDI flash animation on kit view
- [ ] Improve sample browser with preview

### v2.0
- [ ] Implement Mixer page (volume/pan/mute per instrument)
- [ ] Implement FX page (reverb, delay, EQ)
- [ ] Add preset system
- [ ] Sample streaming for large kits

### v3.0
- [ ] Multi-output routing (separate outs per instrument)
- [ ] Built-in groove player with MIDI loops
- [ ] Expansion pack support
- [ ] Cloud sample library integration
