# Drum Pack Naming Conventions (v1)

Drum Tech v1 uses **filename heuristics** for import.  
Each sample file name should encode instrument and mic context using underscore-separated tokens.

## Recommended Pattern

`<instrument>_<articulation>_<mic>_<velOrLayer>[_rrX].wav`

Example:

- `snare_rimshot_snare_top_vel096_rr2.wav`
- `hihat_tip_closed_oh_l_vel064.wav`
- `kick_center_kick_in_vel127.wav`

## Instrument Tokens (current parser baseline)

- `kick`, `bd`
- `snare`, `sd`
- `hihat`, `hh`, `hat`
- `tom1`, `tom2`, `tom3`, `tom4`, `racktom`, `floortom`
- `crash`, `crash1`, `crash2`
- `ride`
- `splash`

## Mic Stem Tokens (current parser)

- Kick In: `kick_in`, `kick in`, `kickin`
- Kick Out: `kick_out`, `kick out`, `kickout`
- Snare Top: `snare_top`, `snare top`, `snaretop`
- Snare Bottom: `snare_bottom`, `snare bot`, `snarebottom`
- Tom 1: `tom_1`, `tom1`, `racktom`
- Tom 2: `tom_2`, `tom2`
- Tom 3: `tom_3`, `tom3`, `floortom`
- Overhead L: `oh_l`, `overheadl`, `overhead_l`, `ovh_l`, `ohl`
- Overhead R: `oh_r`, `overheadr`, `overhead_r`, `ovh_r`, `ohr`
- Room L: `room_l`, `rooml`, `room l`, `rm_l`
- Room R: `room_r`, `roomr`, `room r`, `rm_r`
- Room 2 R: `room_2_r`, `room2r`, `room 2 r`
- Room 2 L: `room_2_l`, `room2l`, `room 2 l`
- Scotch: `scotch`
- Hi-Hat: `hihat`, `hi_hat`, `hh`, `_hat`

## Velocity Layer Tokens (team convention)

Current parser maps by MIDI velocity and RR logic.  
For consistent production, encode one of:

- `vel001` ... `vel127`
- `pp`, `p`, `mp`, `mf`, `f`, `ff` (optional team-side alias, convert to `velXXX` on export)

## Articulation Tokens (team convention for v1 content)

Use explicit tokens for recorded variants:

- `tip`, `side`
- `closed`, `partial_open`, `open`
- `bell`, `rim`, `rimshot`, `crossstick`
- `flam`, `roll`
- `muted`
- `brush_swirl`

## Rules

- Lowercase only, underscores preferred.
- No spaces in final exported filenames.
- Keep one source of truth naming on disk before import.
- Unknown tokens may still load, but mapping quality drops and warnings are expected.
