# Status

Status as of 2026-11-07, version 0.2.0: **Digital Rain** (`saver_digitalrain`), a Matrix-style code rain.

## What is there (`src/main.c`)

- **Rain:** 22 columns with 20 rows. Up to 2 drops fall per column, each with a bright tip and a tail that gets darker towards the back (6 to 17 characters long).
- **Characters:** 16 custom line characters in the style of half-width katakana and digits, about 30 % of them mirrored. Occasionally a character changes quietly, without flashing.
- **Names fall with the rain:** a new one every 3 to 8 s, up to 4 at the same time in different columns (never directly next to each other), the first after 4 s.
  - A name is a drop: its letters run from top to bottom, the last one is the bright tip, and above it follows a tail of 4 to 10 fading characters.
  - It falls row by row like the rain, at the speed of the slower drops (5 to 8 rows per second, smoothly faster with loud music), so that it can be read.
  - About 30 % are **VECTORPIE**; the rest are game names in capitals, without additions in brackets. Names with more than 18 characters are left out because they would not fit on screen.
  - The same name never falls twice at the same time.
- **Game list**, read on the cabinet at start:
  - `/usr/local/share/advance/gamelist.ini` and `gamelist.additions` (lines `Manufacturer|Name|...`, excluding lines commented out with `#`)
  - the `name =` of all `/persistent/games/*/game.ini`

  Duplicate names are removed. New games thus appear automatically.
- **Music:** The rain falls smoothly faster with loud music. Nothing pulses or blinks.
- For the PC: `SAVER_GAMELIST=<file>` reads a copy of the game list, `SAVER_SHOTS=<folder>` saves a frame every 2 s. Without a game list only VECTORPIE appears.

## Checked (2026-11-07)

- PC and Pi builds without warnings in our own code. `build/saver_digitalrain-0.2.0.vps` is produced.
- 0.2.0 on the PC with a copy of the cabinet's game list (43 names up to 18 characters): several names seen in the rain at the same time (VECTORPIE, GEOMETRY WARS …). About 400 to 640 lines per frame, 40 fps.
- 0.2.0 installed on the cabinet (`sh tools/deploy.sh digitalrain`). I tested the board with 0.1.0: 65 names, a steady 40 frames per second, about 440 to 630 lines, no lost lines. 0.2.0 has a similar line count.

## Known problems and limitations

- Brightness and legibility of the names on the real monitor have not yet been judged.
- The Asteroids font draws unknown characters in names as spaces.

## Open tasks

- [ ] Look at it on the cabinet: legibility of the names, speed of the rain, frequency of the names
- [ ] Select it under SETTINGS → SCREENSAVERS → SELECT (this can only be done on the cabinet)
