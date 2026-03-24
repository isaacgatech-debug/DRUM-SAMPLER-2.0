# Drum Library

A professional drum sampler plugin built with JUCE 8, featuring velocity layers, round-robin sampling, and an intuitive kit view interface.

![Drum Library UI](docs/screenshot.png)

## Features

- **Multi-format support**: AU, VST3, and Standalone
- **Intelligent sample mapping**: Automatic MIDI note assignment from filenames
- **Velocity layers**: Up to 127 velocity zones per instrument
- **Round-robin sampling**: Natural variation with multiple samples per hit
- **Modern UI**: Dark theme with visual drum kit layout
- **Tab-based interface**: KIT / MIXER / FX pages (Mixer and FX coming soon)
- **64 polyphonic voices**: No voice stealing under normal use
- **Sample browser**: Load any folder of WAV/AIF/AIFF files

## System Requirements

- **macOS**: 10.15 (Catalina) or later
- **Architecture**: Apple Silicon (arm64) or Intel (x86_64)
- **DAW**: Logic Pro, Ableton Live, Reaper, or any AU/VST3 host
- **Xcode**: 14.0+ (for building from source)
- **CMake**: 3.22+ (for building from source)

## Installation

### Pre-built Binaries

1. Download the latest release from the [Releases](https://github.com/yourusername/drum-library/releases) page
2. Copy `Drum Library.component` to `~/Library/Audio/Plug-Ins/Components/`
3. Copy `Drum Library.vst3` to `~/Library/Audio/Plug-Ins/VST3/`
4. Rescan plugins in your DAW

### Standalone App

Double-click `Drum Library.app` to run the standalone version.

## Building from Source

### Prerequisites

```bash
# Install Xcode from the Mac App Store
xcode-select --install

# Download JUCE 8.0.4
curl -L "https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.4.tar.gz" -o juce.tar.gz
tar -xzf juce.tar.gz
mv JUCE-8.0.4 /tmp/JUCE-8.0.4

# Download CMake 3.28.1
curl -L "https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-macos-universal.tar.gz" -o cmake.tar.gz
tar -xzf cmake.tar.gz
```

### Build Steps

```bash
# Clone the repository
git clone https://github.com/yourusername/drum-library.git
cd drum-library

# Configure CMake
/tmp/cmake-3.28.1-macos-universal/CMake.app/Contents/bin/cmake \
  -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets (AU, VST3, Standalone)
/tmp/cmake-3.28.1-macos-universal/CMake.app/Contents/bin/cmake \
  --build build --config Release -j4

# Install plugins
cp -R build/DrumLibrary_artefacts/Release/AU/Drum\ Library.component \
  ~/Library/Audio/Plug-Ins/Components/
cp -R build/DrumLibrary_artefacts/Release/VST3/Drum\ Library.vst3 \
  ~/Library/Audio/Plug-Ins/VST3/

# Code sign (required for macOS)
codesign --force --deep --sign - \
  ~/Library/Audio/Plug-Ins/Components/Drum\ Library.component
codesign --force --deep --sign - \
  ~/Library/Audio/Plug-Ins/VST3/Drum\ Library.vst3
```

## Usage

### Loading Samples

1. Click **LOAD SAMPLES** button (top right)
2. Select a folder containing drum samples
3. Samples are automatically mapped to MIDI notes based on filename keywords

### Filename Conventions

The sampler recognizes these patterns:

#### Explicit Note Mapping
```
Kick_n36_v0-63_rr1.wav    → MIDI note 36, velocity 0-63, round-robin group 1
Snare_n38_v64-127_rr2.wav → MIDI note 38, velocity 64-127, round-robin group 2
```

#### Keyword Mapping (GM Standard)
- `kick`, `bd` → Note 36
- `snare`, `sd` → Note 38
- `hihat`, `hh`, `hat` → Note 42
- `tom1`, `racktom` → Note 50
- `tom2` → Note 48
- `tom3`, `floortom` → Note 45
- `tom4` → Note 43
- `crash1`, `crash` → Note 49
- `ride` → Note 51
- `splash` → Note 55
- `crash2` → Note 57

### MIDI Mapping

Standard GM drum map:

| Instrument | MIDI Note |
|------------|-----------|
| Kick       | 36        |
| Snare      | 38        |
| Hi-Hat     | 42        |
| Tom 1      | 50        |
| Tom 2      | 48        |
| Tom 3      | 45        |
| Tom 4      | 43        |
| Crash 1    | 49        |
| Ride       | 51        |
| Splash     | 55        |
| Crash 2    | 57        |

## Project Structure

```
DrumLibraryJUCE/
├── CMakeLists.txt           # Build configuration
├── Source/
│   ├── PluginProcessor.h    # Audio processing and state management
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h       # UI components and layout
│   ├── PluginEditor.cpp
│   ├── SamplerEngine.h      # Sample playback engine
│   └── SamplerEngine.cpp
├── build/                   # Build output (generated)
└── docs/                    # Documentation and assets
```

## Architecture

### Sampler Engine

- **Voice allocation**: 64 concurrent voices with automatic stealing
- **Sample loading**: Multi-threaded via JUCE AudioFormatManager
- **Round-robin**: Per-note counters for natural variation
- **Velocity mapping**: Linear gain scaling (0-127 → 0.0-1.0)

### UI Design

- **Colors**:
  - Background: `#1A1A1A`
  - Header: `#111111`
  - Accent: `#E8A020` (orange)
  - Text: `#CCCCCC`
- **Layout**: 900×580px default, resizable 600×400 to 1400×900
- **Kit View**: 11 drum pad zones with visual feedback

## Known Issues

### Logic Pro AU Compatibility (macOS 15)

The AU plugin validates but may show a blank UI in Logic Pro on macOS 15 due to JUCE 8 + Logic AU wrapper incompatibility. **Workaround**: Use the Standalone app or VST3 version in a different DAW.

**Status**: Tracking JUCE issue, expected fix in JUCE 8.0.5+

### Workarounds

1. **Use Standalone**: Full UI works perfectly
2. **Use VST3**: Better compatibility than AU in some hosts
3. **Alternative DAWs**: Ableton Live, Reaper, Studio One have better VST3 support

## Development

### Adding New Features

See [ARCHITECTURE.md](docs/ARCHITECTURE.md) for detailed implementation notes.

### Code Style

- C++17 standard
- JUCE coding conventions
- Prefer `juce::` namespace prefix
- Use `auto` for complex iterator types
- Keep functions under 50 lines where possible

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Credits

Built with [JUCE 8.0.4](https://juce.com/) by ROLI Ltd.

## Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/drum-library/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/drum-library/discussions)

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history.
