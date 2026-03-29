# Drum Tech V1 Scope Contract

## Product Statement

Drum Tech v1 is a **MIDI-first drum instrument**: fast to load packs, fast to play from MIDI keyboard/e-kit pad, and simple to navigate.

## In Scope (Must Ship)

- Simplified top-level UX focused on Home/Kit play workflow.
- Trigger workflow removed from normal user navigation.
- Folder-based drum pack import.
- Filename-driven sample mapping (note + mic stem + velocity groups).
- Per-drum pitch and per-mic trim control from sidebar.
- 15 mic stem mixer with functional core controls.
- 8-band parametric EQ per channel (frequency, gain, Q, type, bypass).
- Persistent state/preset support for imported packs and mic trims.

## Out of Scope (V1 Deferred)

- Manifest-driven pack schema.
- Full DAW-grade piano-roll editing.
- Trigger-to-MIDI replacement product workflow.
- Large redesign of DSP bus architecture beyond current stable path.

## Acceptance Criteria

1. Standalone launches and plays samples via MIDI keyboard without Trigger tab.
2. Importing a valid folder loads samples and routes stems without crashes.
3. Sidebar drum selection updates pitch/mic trim controls correctly.
4. Mixer faders and EQ audibly affect channel output.
5. Saved plugin state restores imported folder and mic trim values.
6. Build passes Release for AU/VST3/Standalone targets.

## Canonical v1 Code Areas

- Editor/navigation: `Source/Core/PluginEditor.h`, `Source/Core/PluginEditor.cpp`
- Sampler/import/routing: `Source/Sampler/SamplerEngine.h`, `Source/Sampler/SamplerEngine.cpp`
- Sidebar controls: `Source/UI/InstrumentSettingsPanel.h`, `Source/UI/InstrumentSettingsPanel.cpp`
- Mixer/EQ: `Source/UI/MixerView.cpp`, `Source/UI/ChannelStrip.cpp`, `Source/Effects/ParametricEQEffect.cpp`
- State/presets: `Source/Core/PluginProcessor.cpp`, `Source/Core/StateManager.cpp`, `Source/Core/PresetManager.cpp`
