# Drum Tech

**Drum Tech** is a professional drum production plugin with groove library, audio-to-MIDI triggering, advanced mixer with third-party plugin hosting, and multi-bus routing.

## Project Status

**Phase 1: Foundation** - In Progress

This is the initial implementation of the Drum Tech architecture as outlined in the comprehensive development plan.

## Current Features (Phase 1)

- ✅ Project structure with modular architecture
- ✅ Enhanced SamplerEngine with pitch shifting and ADSR
- ✅ 11-channel mixer infrastructure
- ✅ 32-bus routing system with DAW integration
- ✅ 5 built-in effects (Reverb, Delay, EQ, Compressor, Transient Shaper)
- ✅ Multi-tab UI framework (Kit, Grooves, Mixer, Trigger, Routing)
- ⏳ Sample loading and playback
- ⏳ Basic UI components

## Planned Features

### Phase 2: Effects & Plugin Hosting (Months 3-4)
- Full implementation of built-in effects
- 3rd party VST/AU plugin hosting
- Send/return FX routing

### Phase 3: Grooves & MIDI (Months 5-6)
- MIDI groove library management
- Timeline/arrangement view
- Tap-to-find rhythm search

### Phase 4: Trigger Engine (Months 7-8)
- Audio-to-MIDI conversion
- Onset detection and drum classification
- Bleed suppression

### Phase 5: UI/UX Polish (Month 9)
- Interactive drum pads with articulations
- Kick beater animation
- Complete mixer UI

## Building from Source

### Prerequisites

- macOS 10.15 or later
- Xcode 14.0+
- CMake 3.22+
- Git (first configure downloads JUCE **8.0.4** via CMake FetchContent; offline builds can use a local JUCE — see below)

**JUCE location (pick one):**

1. **Default** — no extra setup; CMake fetches [JUCE 8.0.4](https://github.com/juce-framework/JUCE) into the build tree.
2. **Local clone** — `-DJUCE_PATH=/absolute/path/to/JUCE` (must contain JUCE’s root `CMakeLists.txt`).
3. **Vendor in repo** — put JUCE at `ThirdParty/JUCE/` (same layout as the upstream repo).

The built plugin product name is **Drum Tech** (see `PRODUCT_NAME` in `CMakeLists.txt`).

**CMake / Git:** This repo does not ignore `*.cmake` globally, so optional CMake modules under something like `cmake/` stay visible to Git if you add them. Generated CMake junk is expected only under `build/` or, if you ever configure in-tree, at the repository root (those root files are ignored explicitly).

### Build Steps

```bash
# Configure CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release -j4

# Install plugins (optional) — confirm exact names under build/DrumTech_artefacts/Release/
cp -R "build/DrumTech_artefacts/Release/AU/Drum Tech.component" \
  ~/Library/Audio/Plug-Ins/Components/
cp -R "build/DrumTech_artefacts/Release/VST3/Drum Tech.vst3" \
  ~/Library/Audio/Plug-Ins/VST3/
```

## Project Structure

```
DrumTech/
├── CMakeLists.txt
├── ThirdParty/JUCE/   # optional; local JUCE checkout
├── Source/
│   ├── Core/              # Plugin processor and editor
│   ├── Sampler/           # Sample engine and voice management
│   ├── Mixer/             # Mixer channels and bus management
│   ├── Effects/           # Built-in and 3rd party effects
│   ├── Routing/           # Audio bus routing
│   ├── Grooves/           # MIDI groove library
│   ├── Trigger/           # Audio-to-MIDI conversion
│   └── UI/                # User interface components
└── README.md
```

## Architecture

The plugin is built with a modular architecture:

- **SamplerEngine**: 64-voice polyphonic sampler with round-robin and velocity layers
- **MixerChannel**: Per-drum channel strips with insert FX and sends
- **BusManager**: 32 configurable buses with DAW multi-output support
- **EffectProcessor**: Base class for built-in and 3rd party effects
- **UI Components**: Modular tab-based interface

## License

MIT License

## Development Timeline

- **Phase 1**: Foundation (Months 1-2) - Current
- **Phase 2**: Effects & Plugin Hosting (Months 3-4)
- **Phase 3**: Grooves & MIDI (Months 5-6)
- **Phase 4**: Trigger Engine (Months 7-8)
- **Phase 5**: UI/UX Polish (Month 9)
- **Phase 6**: Testing & Release (Month 10+)

## Documentation

See the comprehensive development plan at `.windsurf/plans/drum-sampler-2-comprehensive-plan-271b61.md` for detailed specifications and implementation roadmap.
