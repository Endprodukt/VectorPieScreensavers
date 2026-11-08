# Continuing

**Neon Peaks** is the fourth screensaver in the "VectorPie Screensavers" repository (folder `neonpeaks/`). Building, packaging and installing: see `../BUILD.md`. Status: see `STATUS.md`.

## User's wishes

- In the style of retrowave mountains videos (model: YouTube "4K Retrowave Mountains Screensaver"): neon grid, wireframe mountains, striped sun.
- **Here the screensaver may react to the music** (2026-11-07).
- **Colours:** only neon colours that cross-fade into each other. The user liked the first set best (magenta and cyan); green and orange were liked less (2026-11-07). Even so: nothing blinks or pulses, everything changes smoothly (see `../neondrive/RESUME.md`).

## Structure (everything in `src/main.c`)

- `G.h[RING][COLS]`: the heights of each terrain row (row n lies at z = n·DZ). `make_row` generates them as soon as they are needed, from noise times music (`G.bands`, smoothed). `height` makes them grow out of the plain at the horizon.
- `ground_draw`:
  - invisible triangles (`vp3d_scene_polygon` with mask 0) for occlusion. Triangles, because quads on a slope are not flat and would otherwise hide their own edges.
  - On top, the grid lines as `vp3d_scene_line`, in serpentine lines: each piece starts where the last one ended. On the board this reduced the jumps from 470 to 80 and the send time from 25 to 0.5 ms. **This trick could also help Neon Drive (28–30 fps, sending about 30 ms).**
- `sun_draw` and `range_draw`: sun and distant range move along with the camera and so are never reached. An invisible floor at y = 0 hides their lower parts.
- `PAL`: 5 colour sets (ground, slope, peak, sun top, sun bottom, range). `pal()` cross-fades. Each is held for `PAL_HOLD`, the cross-fade takes `PAL_BLEND`, and a new track starts the next one.

## Drawing direction must stay fixed

A line drawn the other way round lies slightly offset on the monitor. When lines are drawn back and forth, the direction must therefore be tied to the line itself (to its own number), not to its place in the loop. Otherwise the direction flips every time a line moves up, and all lines twitch left and right (user feedback on Neon Peaks, 2026-11-07; fixed since Neon Peaks 0.1.2, Neon Ridge 0.1.2, Neon Drive 0.7.2).
