# Changelog

All notable changes to Drum Library will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-03-21

### Added
- Initial release of Drum Library sampler plugin
- JUCE 8.0.4 based architecture
- AU, VST3, and Standalone builds for macOS (Apple Silicon)
- Intelligent sample loading with automatic MIDI note mapping
- Filename parsing for explicit note/velocity/round-robin assignment
- Keyword-based mapping (kick→36, snare→38, etc.) following GM standard
- 64 polyphonic voices with automatic voice management
- Round-robin sampling support for natural variation
- Velocity layer support (0-127 range)
- Dark theme UI with orange accent color
- Kit view with 11 drum pad zones (visual layout)
- Tab navigation (KIT/MIXER/FX) - Mixer and FX pages are placeholders
- LOAD SAMPLES button with folder browser
- Status display showing loaded sample count and folder path
- Resizable window (600×400 to 1400×900)
- State persistence (remembers last loaded sample folder)
- CMake build system
- Comprehensive documentation (README, ARCHITECTURE, CHANGELOG)

### Known Issues
- Logic Pro on macOS 15: AU plugin validates but shows blank UI due to JUCE 8 + Logic AU wrapper incompatibility
  - **Workaround**: Use Standalone app or VST3 in alternative DAW
  - **Status**: Tracking JUCE framework issue, expected fix in JUCE 8.0.5+
- No sample streaming - all samples loaded into RAM
- Mixer and FX pages not yet implemented (UI placeholders only)
- No MIDI flash animation on kit view (planned for v1.1)

### Technical Details
- Built with JUCE 8.0.4
- C++17 standard
- Supports WAV, AIFF, AIF formats via JUCE AudioFormatManager
- macOS 10.15+ required
- Code signed with ad-hoc signature

## Development History

### 2026-03-21 - Session 1: Initial Development

**Phase 1: Project Setup**
- Created CMake project structure
- Downloaded and configured JUCE 8.0.4
- Set up plugin metadata (company: DrumLib, code: Dlbp, bundle ID: com.drumlib.plugin)
- Configured build targets for AU, VST3, and Standalone

**Phase 2: Sampler Engine**
- Implemented `DrumSample` struct for sample data storage
- Created `DrumVoice` class for single-sample playback
- Built `SamplerEngine` with 64-voice polyphony
- Added filename parsing algorithm:
  - Explicit pattern: `_n<note>_v<low>-<high>_rr<group>`
  - Keyword fallback: 11 drum instrument keywords
- Implemented round-robin selection with per-note counters
- Added velocity-based gain scaling

**Phase 3: Plugin Processor**
- Implemented JUCE AudioProcessor interface
- Routed MIDI and audio to SamplerEngine
- Added state save/restore for sample folder path
- Configured stereo output bus

**Phase 4: UI Development**
- Created dark theme design (bg: #1A1A1A, accent: #E8A020)
- Built `KitView` component with 11 drum pad ellipses
- Positioned pads to resemble physical drum kit layout
- Added header with "DRUM LIBRARY" title
- Implemented tab buttons (KIT/MIXER/FX)
- Created LOAD SAMPLES button with async FileChooser
- Added status label showing sample count and folder name
- Made window resizable with constraints

**Phase 5: Build & Compilation**
- Fixed JUCE 8 compatibility issues:
  - Replaced `JuceHeader.h` with module-specific includes
  - Updated `Font` constructor to `FontOptions`
  - Migrated `FileChooser::browseForDirectory()` to async `launchAsync()`
  - Removed broken Timer initialization in KitView
- Added `JUCE_COREGRAPHICS_DRAW_ASYNC=0` for macOS 15 rendering
- Successfully compiled all three targets

**Phase 6: Installation & Testing**
- Installed AU and VST3 to system plugin folders
- Code signed with `codesign --force --deep --sign -`
- Tested Standalone app - **full UI working correctly**
- Attempted Logic Pro AU loading - encountered blank UI issue
- Identified JUCE 8 + Logic AU wrapper incompatibility on macOS 15
- Confirmed Standalone as primary working deployment

**Phase 7: Documentation**
- Created comprehensive README.md with:
  - Feature list and system requirements
  - Installation instructions
  - Build from source guide
  - Usage documentation with filename conventions
  - MIDI mapping table
  - Known issues and workarounds
- Created ARCHITECTURE.md with:
  - Component responsibilities
  - Data flow diagrams
  - Build system documentation
  - Performance considerations
  - Extension points for future development
- Created CHANGELOG.md documenting development history
- Prepared project for clean repository deployment

## [Unreleased]

### Planned for v1.1
- Fix Logic Pro AU compatibility (pending JUCE update)
- Add MIDI flash animation on kit view drum pads
- Improve sample browser with audio preview
- Add drag-and-drop sample loading
- Optimize sample loading with progress indicator

### Planned for v2.0
- Implement Mixer page with per-instrument controls:
  - Volume faders
  - Pan knobs
  - Mute/Solo buttons
  - Level meters
- Implement FX page with built-in effects:
  - Reverb (plate/room/hall)
  - Delay (stereo/ping-pong)
  - EQ (3-band parametric)
- Add preset system for saving/loading configurations
- Implement sample streaming for large kits
- Add multi-mic support (close/overhead/room)

### Planned for v3.0
- Multi-output routing (separate audio outs per instrument)
- Built-in groove player with MIDI loop library
- Expansion pack system
- Cloud sample library integration
- Advanced round-robin modes (random, sequential, velocity-based)
- Sample editor with trim/normalize/reverse
