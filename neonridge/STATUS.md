# Status

Status as of 2026-11-07, version 0.1.2: **Neon Ridge** (`saver_neonridge`).

## What is there (`src/main.c`)

- **Camera:** stands still at a height of 8 m and looks slightly upwards, so that the horizon lies below the centre of the frame (as in the video).
- **Mountain massif:**
  - Layout: a high peak in the middle, lower ones beside it and a ridge in between.
  - Drawn as 30 rows of contour lines running across, 700 to 1150 m away. The front rows hide the rear ones.
  - Higher lines are brighter and bluer.
  - The details are a little different at every start.
- **Sky:** 140 stars that twinkle gently: each one becomes smoothly brighter and darker at its own rate (every 1.7 to 5 s), from two superimposed waves so that it looks irregular (since 0.1.1, the user's wish). In addition, a glow above the horizon made of 9 lines, cyan at the bottom, bluer and fainter towards the top.
- **Plain:** horizontal lines that slowly come closer, with perspective spacing. Their far edge is a long line, pink on the left and cyan on the right.
- **Light trail:** a bright core with a pink glow left and right. It comes in at the bottom left, swings to the right up to just below the horizon and runs back to the left along the horizon. It sways slowly a little.
- **Music, smooth:**
  - The plain comes closer faster with loud music: 0.35 to 1.35 lines per second.
  - The light trail sways faster.
  - The horizon glow gets brighter.
  - On a new track its title appears for 7 s. Nothing blinks or pulses.
- For the PC: `SAVER_SHOTS=<folder>` saves a frame every 2 s.

## Checked (2026-11-07)

- PC and Pi builds without warnings. `build/saver_neonridge-0.1.2.vps` is produced. 0.1.2 (2026-11-07): The lines of the plain keep their drawing direction as they come closer (previously it flipped with every new line, which twitches on the monitor). On the board: 40 frames per second, about 515 lines, sending 0.6 ms, no lost lines.
- Frames viewed on the PC and compared with a still from the video.
- **Installed on the cabinet** (`sh tools/deploy.sh neonridge`) and tested on the board for 15 s:
  - a steady 40 frames per second
  - about 500 lines per frame
  - sending 0.6 ms, no lost lines

## Known problems and limitations

- The soft glow and the reflections of the video do not exist on a vector monitor. Here they are suggested with lines.
- Above the main peak a single stroke of a rear row of lines sometimes sticks out.
- Not yet judged on the real monitor: brightness of stars and glow, and whether the twinkling of the stars is fast enough.

## Open tasks

- [ ] Look at it on the cabinet
- [ ] Select it under SETTINGS → SCREENSAVERS → SELECT (this can only be done on the cabinet)
