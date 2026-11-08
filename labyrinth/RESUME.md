# Continuing

**Labyrinth** is the third screensaver in the "VectorPie Screensavers" repository (folder `labyrinth/`). It is our own vector version of the classic Windows maze screensaver. Building, packaging and installing: see `../BUILD.md`. Status: see `STATUS.md`.

## User's wishes

- "The classic Windows maze screensaver", but without taking anything from the original: our own objects (crystals, a pie as the exit, VECTORPIE signs) and our own patterns.
- **Lots of movement** (feedback 2026-11-07, with reference to a video of the original): never stop, take curves while walking. Stopping and turning on the spot from the first draft has therefore been removed.
- **Turning upside down, as in the original, but not constantly** (feedback 2026-11-07, after the turning had first been removed entirely): a crystal slowly turns the world over (`ROLL_S`, 3 s). Afterwards the wanderer walks a stretch upside down (`UPSIDE`, 10 cells) and then turns back by himself. Crystals he walks through while upside down or during the turn change nothing. A new maze always starts on the floor.
- As with all screensavers here: nothing pulses or blinks (see `../neondrive/RESUME.md`).

## Structure (everything in `src/main.c`)

- `M`: the maze.
  - `hw[y][x]` is the wall along z = y·S, `vw[y][x]` the wall along x = x·S.
  - `maze_new` generates it (depth-first search) and immediately computes the whole path according to the right-hand rule: `path[i]` = cell and direction in which it was entered.
- `walk_at(u)` returns position and viewing direction at `u` cells along the path. Every corner is a quadratic Bézier curve from half a cell before to half a cell after; the viewing direction turns smoothly along with it (when turning around, by π to the right). `G.u` is the progress.
- `maze_draw`:
  - Walls up to `SEEN` (9) cells as `vp3d_scene_polygon`. Top and bottom edges are drawn; vertical edges only at wall ends.
  - Patterns (`PATTERN`) as lines in the wall plane, visible from both sides. Contact 0.05 ensures that the wall itself does not hide them.
  - Signs 6 cm in front of the wall, so that they can only be seen from one side.
- Crystal (octahedron) and pie are meshes from `make_crystal` and `make_pie`.
- Line budget: about 50 to 300 lines, on the board 40 fps with plenty of headroom.
