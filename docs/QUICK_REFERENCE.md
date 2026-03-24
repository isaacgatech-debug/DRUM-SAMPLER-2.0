# Quick Reference

## Build Commands

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build all
cmake --build build --config Release -j4

# Build specific target
cmake --build build --target DrumLibrary_Standalone

# Clean build
rm -rf build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

## Install Commands

```bash
# Install AU
cp -R "build/DrumLibrary_artefacts/Release/AU/Drum Library.component" \
  ~/Library/Audio/Plug-Ins/Components/

# Install VST3
cp -R "build/DrumLibrary_artefacts/Release/VST3/Drum Library.vst3" \
  ~/Library/Audio/Plug-Ins/VST3/

# Code sign
codesign --force --deep --sign - \
  ~/Library/Audio/Plug-Ins/Components/Drum\ Library.component

# Clear Logic cache
rm -f ~/Library/Caches/AudioUnitCache/com.apple.audiounits.cache
```

## File Locations

```
Source code:          Source/
Build output:         build/DrumLibrary_artefacts/Release/
AU install:           ~/Library/Audio/Plug-Ins/Components/
VST3 install:         ~/Library/Audio/Plug-Ins/VST3/
Documentation:        docs/
```

## Filename Conventions

```
Explicit:   Kick_n36_v0-63_rr1.wav
Keywords:   kick.wav → Note 36
            snare.wav → Note 38
            hihat.wav → Note 42
```

## MIDI Note Map

```
Kick    = 36    Tom1    = 50
Snare   = 38    Tom2    = 48
HiHat   = 42    Tom3    = 45
Crash1  = 49    Tom4    = 43
Ride    = 51    Crash2  = 57
Splash  = 55
```

## Project Structure

```
DrumLibraryJUCE/
├── CMakeLists.txt              # Build config
├── Source/
│   ├── PluginProcessor.h/cpp   # Audio processing
│   ├── PluginEditor.h/cpp      # UI
│   └── SamplerEngine.h/cpp     # Sample playback
├── docs/                       # Documentation
├── build/                      # Build output
└── README.md                   # Main docs
```

## Key Classes

```cpp
SamplerEngine    - Sample loading & playback
DrumSample       - Single sample data
DrumVoice        - Single voice playback
PluginProcessor  - Audio processor
PluginEditor     - UI editor
KitView          - Drum kit visual
```

## Common Tasks

### Add New UI Component

1. Declare in `PluginEditor.h`
2. Initialize in constructor
3. Position in `resized()`
4. Paint in `paint()` or component's own paint

### Add Audio Processing

1. Add to `SamplerEngine` or `PluginProcessor`
2. Initialize in `prepareToPlay()`
3. Process in `processBlock()`
4. Add UI controls

### Debug Build

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --config Debug
```

## Troubleshooting

```bash
# Check plugin exists
ls ~/Library/Audio/Plug-Ins/Components/Drum\ Library.component

# Check signature
codesign -dv ~/Library/Audio/Plug-Ins/Components/Drum\ Library.component

# Clear all caches
rm -f ~/Library/Caches/AudioUnitCache/com.apple.audiounits.cache
rm -rf ~/Library/Caches/com.apple.logic.pro*

# Rescan in Logic
Logic Pro → Settings → Plug-in Manager → Reset & Rescan All
```

## Git Workflow

```bash
# Create branch
git checkout -b feature/your-feature

# Commit
git add .
git commit -m "feat: Add your feature"

# Push
git push origin feature/your-feature

# Create PR on GitHub
```

## Documentation Files

```
README.md           - Overview, features, installation
ARCHITECTURE.md     - Code structure, design decisions
BUILD.md            - Detailed build instructions
CHANGELOG.md        - Version history
CONTRIBUTING.md     - Contribution guidelines
LICENSE             - MIT license
QUICK_REFERENCE.md  - This file
```

## Color Palette

```cpp
Background:  0xFF1A1A1A  // Dark gray
Header:      0xFF111111  // Darker gray
Accent:      0xFFE8A020  // Orange
Text:        0xFFCCCCCC  // Light gray
Cymbal:      0xFFD4A800  // Gold
Drum:        0xFFE8A020  // Bronze
```

## UI Layout

```
Window size:  900×580 (default)
Resizable:    600×400 to 1400×900
Header:       56px
Tab bar:      36px
Content:      488px
```

## Known Issues

- Logic Pro AU shows blank UI on macOS 15 (JUCE 8 issue)
- Workaround: Use Standalone app
- Status: Awaiting JUCE 8.0.5+ fix

## Support

- Issues: https://github.com/yourusername/drum-library/issues
- Docs: https://github.com/yourusername/drum-library/tree/main/docs
- JUCE: https://forum.juce.com/
