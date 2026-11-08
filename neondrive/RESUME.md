# Continuing

**Neon Drive** is a VectorPie screensaver: an endless synthwave drive in 3D to the cabinet's music. It is the first screensaver in the "VectorPie Screensavers" repository (folder `neondrive/`). Building, packaging and installing: see `../BUILD.md`. Current status and open points: see `STATUS.md`.

## User's wishes (permanently valid)

- **Nothing pulses, flashes or blinks.** No reaction to beats, not even from the sun. The music may only smoothly control speed and traffic density.
- No stars next to the sun.
- Everything at the roadside stands fixed to the track: nothing jumps, nothing moves along with the car.
- Backdrops on the horizon change slowly (minutes).
- VECTORPIE should appear: on lorries, sign gantries, road signs, billboards and the plane's banner.

## Structure

| File | Contents |
|---|---|
| `src/saver.h` | shared state `G` (distance, speed, heading, camera, music), road dimensions, prototypes |
| `src/main.c` | music feed, speed, title, the scene per frame, `SAVER_SHOTS`; with `SAVER_SOAK` the soak test |
| `src/road.c` | curves, road table (`road_point`, `road_heading`), sections (`zone_kind`), oncoming carriageway (`road_oncoming`), grid and lane lines |
| `src/models.c` | all meshes made of boxes and single lines: car, 7 vehicle types, roadside objects; the VECTORPIE line font (`vectorpie_strokes`, `word`) |
| `src/props.c` | roadside objects per section type |
| `src/traffic.c` | traffic in both directions, evasion logic, speed limiting behind vehicles |
| `src/flyers.c` | flocks of birds, plane with VECTORPIE banner |
| `src/car.c` | our own car (leaning when changing lanes) and the camera (positions, gliding) |
| `src/scenery.c` | backdrop ring, rising and sinking, sun, horizon line |

## Basics

- In every frame the car is at the origin, and the road there points towards +z. `road_build()` lays out the road from 40 m behind to 420 m ahead of the car. `road_point(d, x, y)` returns the point d metres ahead, x across, y up.
- Everything runs through a single `vp3d_scene`. Solid meshes and `vp3d_scene_polygon` hide things; lines and wireframes hide nothing. With `vp3d_scene_set_contact(0.3)` lines on the ground stay visible. An invisible ground polygon hides everything below y=0.
- Backdrops are flat polygons on a ring with R=3500 m. Their details lie 3 m in front, so that their own polygon does not hide them.
- **Roadside objects are tied to the track.** Positions, sides and variety are derived from the distance from the start of the section, never from a counter over the currently visible objects. Such a counter made the palms jump. Posts and repetitions start at the beginning of the section, not at the car. That is how the guard rail posts had moved along.
- **No dice loops** like `do x = random(); while (x == old);` with a random value that does not change inside the loop. That hung the program (camera change). Instead: `(old + 1 + random * (n - 1)) % n`.
- Line budget: at about 2000 lines per frame it gets tight on the board (experience from Vector Racer). Measure with `VP_STATS=1` or the soak test.
- To view on the PC: double-click `neondrive/run_pc.bat`. Programs started from the Claude environment are sometimes terminated from outside there. Frames: `SAVER_SHOTS=<folder>`, saved at the end of `draw()`.
- Soak test: `build/pc/neondrive/saver_soak.exe <hours> <seed>` simulates about 25 minutes of driving per second without music, outputs the heaviest frames and finds hangs (in which case it does not run to completion).
- **Never through other vehicles:** `traffic_limit()` limits the car's path (front never past the rear of a vehicle in its path). `steer()` does not let the car pull sideways into a vehicle beside it. Vehicles behind the car take on its speed. Anyone changing this checks with the soak test and a temporary overlap check in `traffic_update` (last time 0 in 25 simulated hours) and removes the check again afterwards.
- Wheels: `wheel()` in `models.c` remembers the wheel positions per vehicle type. `model_spokes()` draws three spokes per wheel, rotated by distance times `WHEEL_TURN`. This is deliberately slower than real, otherwise the frame rate would cause the wagon-wheel effect.
- Camera: `SHOTS` in `car.c`. The field `swing` makes a position swing around the centre of the car to the right and back (sin² curve, `SWING_TIME`). Beforehand `traffic_hold_lane(RIGHT_LANE)` brings the car into the right lane, and the swing only starts when `traffic_room` is greater than `SWING_ROOM`.
- **The camera must never be inside a vehicle.** Three things ensure this: sideways positions stay beside the carriageway, when gliding across the lanes the camera rises to `OVER_TRAFFIC`, and vehicles behind the car keep a `BEHIND` distance. This is checked with a temporary point-in-vehicle check in the soak test (last time 0 cases) and with the distance driven (no loss of speed).
- The soak test `saver_soak` hides its window and answers the window messages. Otherwise Windows shows it as "Not responding", which looks like a crash.

## SDK

- The documentation is in `vendor/VectorPie/sdk/docs/`: `05-3d.md` (3D, scenes) and `09-screensavers.md`. The API is in `vp_3d.h`, `vp_saver.h` and `vp_music_feed.h`.
- A screensaver is a normal `vp_run` game with `kind = screensaver` in `game.ini`. The SDK takes care of burn-in protection.
- To take over the SDK again: copy `F:\Release Game\VectorPie` (or newer) to `vendor/VectorPie`, without `.git`.

## Speed on the board: the order of the lines

The SDK draws in exactly the order in which lines are added. Between two lines the beam travels dark. The board gets slow when the whole path per frame (lit and blank, `lit length` and `blank length` in `VP_STATS`) becomes too long, from about 270k.

Since 0.7.1 (2026-11-07) the following therefore applies:
- In `road.c` the lines run back and forth (`along(..., back)`, the cross lines and the dashes). Each line starts where the last one ended.
- In `props.c` there are two passes: first the right side (and everything above the road) from near to far, then the left from far to near (`pass`, `here()`).
- The sun's lines also run back and forth.

New parts should be drawn the same way.

## Drawing direction must stay fixed

A line drawn the other way round lies slightly offset on the monitor. When lines are drawn back and forth, the direction must therefore be tied to the line itself (to its own number), not to its place in the loop. Otherwise the direction flips every time a line moves up, and all lines twitch left and right (user feedback on Neon Peaks, 2026-11-07; fixed since Neon Peaks 0.1.2, Neon Ridge 0.1.2, Neon Drive 0.7.2).
