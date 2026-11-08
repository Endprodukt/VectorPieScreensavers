# Continuing

**Digital Rain** is the second screensaver in the "VectorPie Screensavers" repository (folder `digitalrain/`): green code rain with game names. Building, packaging and installing: see `../BUILD.md`. Status: see `STATUS.md`.

## User's wishes

- Matrix style, as everyone knows it. The gag: names from the game list and VECTORPIE.
- The names **run with the rain** (fall as drops, not across the screen). They may well come more often, also several at the same time (feedback 2026-11-07). Gliding and snapping into place from 0.1.0 have therefore been removed.
- As with all screensavers here: nothing pulses or blinks (see `../neondrive/RESUME.md`).

## Structure (everything in `src/main.c`)

- `GLYPHS`: the characters as strokes in a 0..1 cell. Bit 7 in the cell value means mirrored.
- `C[COLS]`: per column the drops (`head` in rows from the top, `speed`, `len`) and the characters of each row.
- `ND[NAME_DROPS]`: the falling names (column, letters, tail, `head`, `speed`). `start_name` looks for a free column with no name next to it, `names_update` moves them at the speed of the rain and starts a new one every 3 to 8 s. In a column with a name no new raindrops are created, and the rain cells below the name stay free (`name_cell`).
- Game list: `read_gamelist` (format `Manufacturer|Name|...`) and `read_games` (`/persistent/games/*/game.ini`). `add_name` converts to capitals, removes `(...)` and duplicates, and leaves out names over `NAME_FIT` (18) characters.
- Line budget: about 550 lines per frame, 40 fps on the board. So there is room for more columns or longer tails.
- A copy of the game list for the PC is fetched from the cabinet:

  ```bash
  scp -i ~/.ssh/id_ed25519_vectorpie pi@192.168.0.122:/usr/local/share/advance/gamelist.ini <file>
  ```

  Then set `SAVER_GAMELIST=<file>`.
