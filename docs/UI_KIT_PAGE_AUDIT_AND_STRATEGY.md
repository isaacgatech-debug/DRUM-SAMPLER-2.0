# Drum Tech — KIT page UI audit & fix strategy

This document captures **observed UI problems** on the main **KIT** screen (and the persistent chrome that surrounds it), **why they happen in the current code**, and a **prioritized strategy** to make the front page legible, trustworthy, and usable.

It is intended for designers and engineers; it does not replace task-level tickets but groups work into coherent phases.

---

## 1. What the KIT page should do

The first screen after launch should answer, at a glance:

| User need | Implication for UI |
|-----------|-------------------|
| **Which kit am I using?** | Clear kit name in header; optional kit art or summary on the main area. |
| **Which drum did I select?** | Obvious selection state on pads/cymbals + **readable** name and controls for *that* instrument. |
| **Can I shape level / mute / solo quickly?** | Large enough hit targets; labels that read as “level”, not cryptic abbreviations alone. |
| **Where do I go next?** | Tabs readable; timeline/transport understandable without decoding symbols. |

Secondary: the **backdrop** is decorative; it should **scale gracefully** on wide windows (no huge empty pillars unless intentional).

---

## 2. Issue catalog (screenshot-aligned)

Severity: **P0** = breaks meaning or trust; **P1** = hurts usability; **P2** = polish.

### 2.1 Mojibake / “â ¼” / “â…” text (P0 — meaning)

**Symptoms**

- Section headers show **`â ¼ Voice and Layer`**, **`â ¼ MIDI Mapping`** instead of a clear expand/collapse affordance.
- Transport buttons show **`â…`**-style garbage instead of recognizable loop / stop / play / record icons.

**Root cause (code)**

- `InstrumentSettingsPanel.cpp` draws Unicode arrows in section headers: `"▶ "` / `"▼ "` concatenated with labels.
- `TransportBar.h` assigns Unicode symbols to `TextButton` labels: `⟳`, `■`, `▶`, `●`.

If the **active typeface** (from the look-and-feel / system font) **does not contain those code points**, or text is mishandled through a non–UTF-8 path, the OS substitutes replacement glyphs or mis-decoded bytes → **mojibake**.

**Fix direction**

- Prefer **ASCII or short words** on transport: `LOOP`, `STOP`, `PLAY`, `REC` (or icons drawn with `Path` / `Drawable` in `paint`, not as font glyphs).
- Replace `▶`/`▼` in painted headers with **“+ / –”**, **“Show / Hide”**, or **chevrons drawn as vector paths** so they never depend on font coverage.
- Ensure **all source files are UTF-8** and the project treats them as such (compiler / IDE); still, **do not rely on rare symbols** for critical chrome.

---

### 2.2 Timeline track row: “+ ck 1” / clipped “Track” label (P0 — layout bug)

**Symptoms**

- Center of the timeline toolbar looks like **`+ ck 1`** or otherwise truncated; “Track 1” reads broken.

**Root cause (code)**

- `GrooveTimeline::paint` draws the track name with a **hard-coded X offset** (`trackBarArea.getX() + 192`) and fixed width, **independent of actual button bounds** from `resized()`.
- Child buttons (Track dropdown, Block dropdown, `+`, etc.) are laid out from the left; the painted label **overlaps** the `+` button and is **clipped**, producing fragments like “ck 1”.

**Fix direction**

- **Remove** the magic `192` / `120` paint hack.
- Either:
  - **Dedicated `Label`** component placed in `resized()` in the **remaining** track-bar band (between last left control and first right control), or
  - Compute `juce::Rectangle<int>` from **actual** child bounds (e.g. `getRight()` of `addTrackBtn` + margin) and draw only in that rectangle.

---

### 2.3 Groove block labels overlapping or “garbled” (P1)

**Symptoms**

- Colored blocks show **overlapping or corrupted** text (e.g. pattern name + type running together).

**Root cause (code)**

- `GrooveTimeline::drawBlock` stacks **pattern name** (`removeFromTop`) and **type** (`removeFromBottom`) in the same rect. On **narrow** blocks or with **long names**, rectangles **overlap** or `drawText(..., true)` ellipsizes in ways that look like multiple strings superimposed.
- Any **font that lacks glyphs** for unusual characters in future data will worsen readability.

**Fix direction**

- Enforce a **minimum block width** for two lines; below that, show **single line** (name only) + tooltip with full metadata.
- Use **consistent vertical layout** (fixed line heights, `GlyphArrangement` or two explicit rects with no overlap).
- Optional: **truncate name** with ellipsis and full text on hover.

---

### 2.4 Right panel: tiny sliders, S/M, and toggles (P1)

**Symptoms**

- Level / soft-hit sliders feel **hairline-thin**; thumb is easy to miss.
- **S** / **M** are small; toggles are **painted** faux switches that don’t match native size expectations.

**Root cause (code)**

- JUCE **linear sliders** default to a **thin** track unless the look-and-feel increases **track thickness**, thumb radius, or uses a custom slider component.
- `InstrumentSettingsPanel` uses **custom-painted** toggle rows (`drawToggleRow`) with **fixed pixel geometry**; scaling the window does not necessarily scale hit targets proportionally unless layout is redone.
- Header comment still references **260px** panel; implementation was widened but **density** of controls is still high.

**Fix direction**

- Introduce **`UIDesign` tokens**: minimum **thumb** size (e.g. ≥ 18–22 px), **track height** (e.g. ≥ 6–8 px), **row height** per control class.
- Consider **`Slider::SliderLayout` / custom LookAndFeel** methods for linear sliders, or small wrapper components (“`FatHorizontalSlider`”).
- Replace painted toggles with **`ToggleButton`** + LNF for **accessible** hit targets, *or* keep custom paint but **sync** `mouseDown` rects to **painted pill** bounds and **increase** pill size.

---

### 2.5 Transport bar: icons too small or unreadable (P1)

**Symptoms**

- Controls are hard to identify; compounded by mojibake when Unicode fails.

**Root cause (code)**

- Buttons are sized in `TransportBar::resized` but use **single-character** labels; effective **clickable** area may still feel small on HiDPI.
- `AnalogKnob` for **VOL** uses `Style::Small` in `TransportBar.h` — knob stays visually modest even when bar height increased.

**Fix direction**

- **Text or vector** transport (see §2.1); bump **minimum button width/height** (e.g. 44×44 pt guideline for primary actions).
- Use **`AnalogKnob::Style::Standard`** (or larger) for master volume when bar height allows.

---

### 2.6 Kit view: wasted horizontal space (P2 — composition)

**Symptoms**

- On **wide** windows, large **black columns** left/right of the backdrop; kit feels like a small island.

**Root cause (code)**

- `DrumKitView::paint` draws the backdrop with **centred + onlyReduceInSize** placement inside the kit area (full width minus settings panel). **Aspect ratio** of art is preserved → letterboxing.

**Fix direction**

- **Product decision**: either accept letterboxing (OK for “poster” art) or **switch strategy**:
  - **Crop** (fill width, clip top/bottom) with `RectanglePlacement::fillDestination` or custom fit.
  - **Extend** background with a **gradient / panel** that matches the art edges.
  - **Resize kit hit regions** to use more horizontal space for control (bigger targets).

---

### 2.7 Visual hierarchy: one giant button among tiny controls (P2)

**Symptoms**

- “MIDI In/E-Drums Settings” dominates while neighbors are tiny.

**Root cause**

- Intentional emphasis without **middle-tier** controls (no “secondary” button style sizing).

**Fix direction**

- Define **button tiers**: primary / secondary / tertiary with **height and font** steps.
- Optionally move rare actions behind a **gear menu** or secondary screen to balance the column.

---

### 2.8 Stale documentation in headers (P3)

**Examples**

- `TransportBar.h` comment still says **44px** strip; editor now uses a taller transport.
- `InstrumentSettingsPanel.h` still says **260px** panel.

**Fix direction**

- Align comments with `PluginEditor` / component constants or refer to a single **`UIDesign`** namespace to avoid drift.

---

## 3. Fix strategy (phased)

### Phase A — Trust & legibility (highest ROI)

1. **Replace Unicode-dependent transport and section labels** with ASCII words or vector-drawn icons (§2.1).
2. **Fix `GrooveTimeline` track title layout** — eliminate magic X offset; use layout-driven bounds (§2.2).
3. **Audit groove block text layout** for overlap; add min width + tooltip (§2.3).

*Exit criteria:* No mojibake on stock macOS/Windows; track row shows full “Track 1” (or chosen name) without clipping; blocks readable at default zoom.

### Phase B — Hit targets & density

1. **Slider LNF or custom sliders** — thicker track + larger thumb on instrument panel (§2.4).
2. **Transport** — larger buttons, master knob size, optional tooltips (§2.5).
3. **Toggle rows** — enlarge pills or migrate to real `ToggleButton`s (§2.4).

*Exit criteria:* Sliders and transport comfortable on trackpad; WCAG-ish target sizes for primary actions.

### Phase C — KIT page composition

1. **Backdrop fit policy** (letterbox vs crop vs hybrid) (§2.6).
2. **Button tiering** and optional re-homing of MIDI settings (§2.7).
3. **Optional**: `UIDesign` central file for spacing, fonts, min sizes (§2.8).

---

## 4. Suggested ownership

| Area | Primary files (current) |
|------|-------------------------|
| Transport + encoding | `TransportBar.h`, `TransportBar.cpp`, `DrumTechLookAndFeel.h` |
| Timeline track row + blocks | `GrooveTimeline.h`, `GrooveTimeline.cpp` |
| Instrument sidebar | `InstrumentSettingsPanel.h`, `InstrumentSettingsPanel.cpp` |
| Kit layout / backdrop | `DrumKitView.h`, `DrumKitView.cpp` |
| Global chrome sizing | `PluginEditor.h`, `PluginEditor.cpp` |

---

## 5. References in repo (quick grep anchors)

- Unicode transport: `TransportBar.h` — `loopButton {"⟳"}`, `stopButton {"■"}`, etc.
- Unicode section headers: `InstrumentSettingsPanel.cpp` — `"▶ "`, `"▼ "`.
- Hard-coded timeline label X: `GrooveTimeline.cpp` — `trackBarArea.getX() + 192`.
- Block text stacking: `GrooveTimeline.cpp` — `drawBlock` name + type layout.

---

*Last updated: 2026-03-28 — aligned with user screenshot review and current source layout.*

---

## 6. Implementation status (2026-03-28)

The following were applied in code:

| Audit item | Change |
|------------|--------|
| Unicode / mojibake (transport, sections, trigger) | `TransportBar`: `LOOP` / `STOP` / `PLAY` / `REC`. `InstrumentSettingsPanel`: `[+]` / `[-]` section prefixes. `TriggerUI`: `Play`, `^ EQ` / `v EQ`. |
| Timeline track label overlap | `trackNameLabel` (`Label`) in the gap between `+` and zoom cluster; removed fixed `+192` paint. Zoom buttons labeled `In` / `Out`. |
| Groove block text overlap | `drawBlock` uses sequential `removeFromTop` / `removeFromBottom` on one `inner` rect; waveform only in middle band; narrow blocks = single-line name. `GrooveTimeline` inherits `SettableTooltipClient` for `name — type` tooltips on hover. |
| Thin sliders | `DrumTechLookAndFeel.cpp`: larger thumb radius + thicker linear track (not capped at 6px). `UIDesign` namespace in `PluginColors.h`. Instrument panel slider rows use `instrumentSliderHeight` (36px). |
| Transport size / VOL knob | Wider text buttons; `AnalogKnob::Style::Standard` + wider knob bounds. |
| Toggles / S-M | Taller toggle pills; `S`/`M` buttons 36px wide; row height 32px. |
| MIDI button hierarchy | Shorter copy `MIDI & E-drum setup`, panel-coloured button, muted text, max height 30px. |
| Kit backdrop letterboxing | `DrumKitView`: `fillDestination` so art fills the kit area (crops instead of pillarboxing). |
