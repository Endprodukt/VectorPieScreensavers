# Status

Status as of 2026-11-07, version 0.1.2: **Neon Peaks** (`saver_neonpeaks`).

## What is there (`src/main.c`)

- **Valley:** a flat grid, 32 m wide. The camera flies over it at a height of 5 m and slowly drifts a little sideways.
- **Mountains:** left and right as wireframes, but solid: they hide what lies behind them. The terrain has 24 rows of 10 m each and 21 points across, further apart on the outside. It is fixed in the world, so you fly past it.
  - New rows grow out of the plain at the horizon; they do not pop up.
  - The terrain gets darker with distance. This is a custom fog; the view's fog would also dim the sun.
- **Sun:** a circle of horizontal lines that slowly glide downwards. In the lower half the gaps get wider, which gives the typical stripes. In front of it stands a distant mountain range that is never reached.
- **Music:**
  - Every new row at the horizon gets its shape from the spectrum currently playing. The bass shapes the mountains close to the valley, the treble the outer ones. Loud passages produce high mountains, which then fly closer.
  - The spectrum is smoothed so that no spikes appear.
  - The flying speed is 20 to 42 m/s and follows the volume smoothly.
  - **Colours:** 5 neon colour sets (magenta, cyan, pink, violet, blue; green and orange are out). They constantly cross-fade into each other: each is held for 18 s, followed by a cross-fade of 8 s. A new track starts the next cross-fade immediately. The start is always the classic set (magenta grid, cyan peaks).
  - On a new track its title appears for 7 s.
  - Nothing blinks or pulses.
- For the PC: `SAVER_SHOTS=<folder>` saves a frame every 2 s.

## Checked (2026-11-07)

- PC and Pi builds without warnings. `build/saver_neonpeaks-0.1.2.vps` is produced.
- 0.1.2 (2026-11-07): The rows twitched left and right because their drawing direction flipped with every row moving up. Now it is tied to the row's number. On the board 40 frames per second, sending 0.6 ms. Not yet confirmed on the monitor.
- Frames viewed on the PC, with colour change on track change.
- **Installed on the cabinet** (`sh tools/deploy.sh neonpeaks`) and tested on the board for 25 s:
  - a steady 40 frames per second
  - about 520 lines per frame, but only about 80 jumps
  - sending 0.5 ms, no lost lines

## Known problems and limitations

- Not yet judged on the real monitor: brightness, speed, how clearly the music can be seen in the mountains.

## Open tasks

- [ ] Look at it on the cabinet and judge it with real music
- [ ] Select it under SETTINGS → SCREENSAVERS → SELECT (this can only be done on the cabinet)
