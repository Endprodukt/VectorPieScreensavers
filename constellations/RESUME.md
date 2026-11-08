# Continuing

**Constellations** is the sixth screensaver in the "VectorPie Screensavers" repository (folder `constellations/`). Building, packaging and installing: see `../BUILD.md`. Status: see `STATUS.md`.

## User's wishes

- **Real constellations** (2026-11-07).
- **Stars must twinkle**, gently and smoothly, each at its own rate (2026-11-07).
- Nothing may pulse or blink. Music only changes things smoothly (see `../neondrive/RESUME.md`).

## Structure

- `src/sky.h`: the data.
  - For each constellation its stars with right ascension (hours), declination (degrees), magnitude, colour and, where applicable, name.
  - `path`: the figure as a sequence of star numbers; −1 starts a new line.
  - `shared`: the same star in two figures (Elnath, Alpheratz). It is drawn only once.
  - `SKY_CON` is also the order of the tour.
  - `SKY_LONE`: bright single stars.
- `src/main.c`:
  - `sky_make`: directions of all stars, centre and size of each figure, plus the faint stars (Milky Way via the galactic north pole and the galactic centre).
  - Projection: stereographic around `G.mid`, north at the top, east on the left. `G.half` is half the image width (`con_half`).
  - `tour_update`: gliding (`glide`), drawing in (`fig`), holding (`hold`), then `tour_next`.
  - `stars_draw` sorts the visible stars in bands back and forth so that the beam has short paths.
  - The drawing direction of every star and every figure line is fixed: stars from left to right and from bottom to top, lines in the order of the path (see `../neonridge/RESUME.md`, "Drawing direction must stay fixed").
- For testing: `SAVER_TEMPO=<factor>` makes the tour faster, `SAVER_SHOTS=<folder>` saves a frame every 2 s.

## Round on the tube

The board has a square coordinate field, but the monitor stretches it to 4:3 (3:4 with a vertical monitor). Anything drawn round in 2D therefore appears a third too wide. The 3D screensavers compensate for this via `view.aspect`. In 2D the rule is: compress horizontally towards the centre by 0.75 (`ASPECT_X`; with `VP_VERTICAL=1` the height instead). The user noticed this on the clock (2026-11-07).
