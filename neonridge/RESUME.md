# Continuing

**Neon Ridge** is the fifth screensaver in the "VectorPie Screensavers" repository (folder `neonridge/`). Building, packaging and installing: see `../BUILD.md`. Status: see `STATUS.md`.

## User's wishes

- **Just like in the video** "4K Retrowave Mountains Screensaver" (YouTube `M_SRn64CiyM`), 2026-11-07, after Neon Peaks had been interpreted too freely.
- The video shows:
  - a stationary camera and a starry sky
  - a horizon glowing cyan
  - a dark mountain massif with wavy contour lines and a high central peak
  - a dark plain with horizontal lines
  - a pink light trail that swings from bottom left to the right and runs back along the horizon
  - no sun
- It may react to the music, but nothing may pulse or blink (see `../neondrive/RESUME.md`).

## Structure (everything in `src/main.c`)

- `range_make`: heights `MH` from peaks (`peak`) plus noise. `range_draw` draws invisible triangles for occlusion and on top of them the rows in a serpentine line.
- `sky_draw`: the glow as lines 4000 m away (behind the mountains) and the stars at 5000 m. `stars_make` sorts the stars in bands back and forth so that the beam has short paths. `STAR[i][3]` and `[4]` are rate and phase of the twinkling.
- **Stars must twinkle** (user's wish, 2026-11-07), gently and smoothly, each at its own rate.
- `plain_draw`: the lines with spacing `P_RATIO`; `G.flow` is the phase in lines.
- `trail_at(s)`: the path of the trail, in screen widths `u` (−1 left, 1 right) and distance `z`. `trail_draw` draws three lines: glow, core back, glow.
- Mind the order of the lines: see `../neondrive/RESUME.md` ("Speed on the board").

## Drawing direction must stay fixed

A line drawn the other way round lies slightly offset on the monitor. When lines are drawn back and forth, the direction must therefore be tied to the line itself (to its own number), not to its place in the loop. Otherwise the direction flips every time a line moves up, and all lines twitch left and right (user feedback on Neon Peaks, 2026-11-07; fixed since Neon Peaks 0.1.2, Neon Ridge 0.1.2, Neon Drive 0.7.2).
