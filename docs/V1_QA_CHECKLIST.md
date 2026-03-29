# Drum Tech V1 QA Checklist

## Build & Launch

- [ ] Release build succeeds for Standalone/AU/VST3.
- [ ] Standalone launches without crash.
- [ ] Home backdrop loads (`Resources/Backdrops/Drum Tech.png`).

## Navigation

- [ ] Top tabs show `HOME`, `MIXER`, `SETTINGS`.
- [ ] Trigger tab is not visible in normal UX.
- [ ] Kit lock blocks import/switch operations.

## Import & Mapping

- [ ] Import folder from Settings works.
- [ ] Unknown filenames log warnings, no crash.
- [ ] Correct stem routing for Kick/Snare/Toms/OH/Room/Scotch/Hi-Hat.

## Performance & Playback

- [ ] MIDI keyboard triggers expected drums.
- [ ] Velocity layer fallback works when exact layer missing.
- [ ] Articulation hint affects sample selection.
- [ ] Drummer profile and playing style switches affect playback.

## Sidebar Controls

- [ ] Pitch updates are audible.
- [ ] Mic trim sliders update selected drum routing.
- [ ] Velocity curve selector updates note response behavior.

## Mixer & EQ

- [ ] Console faders move and affect level.
- [ ] Mute/Solo/Pan update channel behavior.
- [ ] 8-band EQ window opens and updates channel DSP.

## Persistence

- [ ] Reload project restores imported folder.
- [ ] Mic trims restore from state.
- [ ] Preset save/load keeps v1-critical controls.
