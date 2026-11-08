# Status

Status as of 2026-11-07, version 0.1.1: **Labyrinth** (`saver_labyrinth`).

## What is there (`src/main.c`)

- **Mazes:** each one random, 9 to 16 cells per side. Every cell can be reached by exactly one path. Cells are 2.4 m in size, the walls 2.2 m high, the eye is at half height.
- **The walk:** The wanderer keeps his right hand on the wall and so always reaches the exit.
  - He never stops. He takes curves in an arc and turns as he goes; he walks into dead ends, turns to the right and walks back.
  - Speed: 2 cells per second, smoothly up to 3.2 with loud music. In curves he goes a little slower, and much slower when turning around.
- **Walls:** One pattern per maze: bricks, panels, tiles or stripes with a diamond. The pattern is only drawn up to about 5 cells away; beyond that it would be lost in the fog anyway. In addition, a grid on floor and ceiling, and fog that darkens with distance.
- **VECTORPIE signs:** on about every eleventh wall, on one side.
- **Crystals:** 3 per maze, on the path. They rotate and disappear when the wanderer arrives. If the wanderer walks through one while on the floor, the world slowly turns upside down over 3 s. He then walks on upside down for 10 cells and afterwards slowly turns back (since 0.1.1, the user's wish).
  - A crystal only stands on the first visit to a cell and has disappeared when the wanderer is half a cell away. So the camera never ends up inside it.
  - This used to be possible; its edges then ran across the screen as slanted strokes (feedback 2026-11-07).
- **Exit:** a rotating pie with a missing slice. Before it, the maze fades out and the next one fades in with different colours. There are 5 colour sets.
- **Music:** The speed follows the volume smoothly. On a new track its title appears for 7 s. Nothing pulses or blinks.
- For the PC:
  - `SAVER_SHOTS=<folder>` saves a frame every 2 s.
  - `SAVER_TEMPO=<factor>` runs correspondingly faster.

## Checked (2026-11-07)

- PC and Pi builds without warnings. `build/saver_labyrinth-0.1.1.vps` is produced. 0.1.1 on the board: 40 frames per second, no lost lines.
- Frames viewed on the PC: patterns, signs, crystals, the flip, the exit and several mazes in a row (100 s at 8× speed).
  - 38 to 41 fps.
  - At most about 290 lines per frame.
- **Installed on the cabinet** (`sh tools/deploy.sh labyrinth`) and tested on the board for 25 s:
  - a steady 40 frames per second
  - 50 to 126 lines per frame
  - no lost lines

## Known problems and limitations

- At junctions where a cross wall meets a continuous wall, the vertical corner line is sometimes drawn by both walls, i.e. twice. This is unsightly but harmless.
- How it looks on the real monitor has not yet been judged: brightness of the patterns, speed and curves.

## Open tasks

- [ ] Look at it on the cabinet: speed, curves, brightness
- [ ] Select it under SETTINGS → SCREENSAVERS → SELECT (this can only be done on the cabinet)
- [ ] Possible extensions: a creature that walks through the corridors; mazes with rooms
