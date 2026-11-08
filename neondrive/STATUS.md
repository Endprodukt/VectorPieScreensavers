# Status

Status as of 2026-11-07, version 0.7.2: **Neon Drive** in 3D (`saver_neondrive`), built with the SDK's 3D interface (`vp_3d.h`) and hidden lines.

## What is there

- **3D world** in metres, everything in one scene: solid bodies hide what lies behind them. An invisible floor hides everything below the horizon.
- **Road** (`src/road.c`): endless, curves from smoothed noise, three lanes in our direction. The pink grid lies only beside the carriageways.
- **Sections** of 520 m each, never the same one twice in a row (`src/props.c`):
  - Beach: palms, street lamps, VECTORPIE sign
  - City: high-rises with fixed lit windows, street lamps, billboards (one of them large with VECTORPIE)
  - Desert: cacti, rocks, VECTORPIE signs
  - Power: pylons with cables, guard rails, sign
  - Neon tunnel: arcs of light over the road
  - Motorway: sign gantries (every second one with VECTORPIE), street lamps, guard rails, sign
- **Everything at the roadside stands fixed to the track**: nothing changes sides, nothing moves along. You drive past everything.
- **Oncoming carriageway** in about 45 % of the sections, with oncoming traffic that is seen from the front.
- **Traffic** (`src/traffic.c`, models in `src/models.c`): saloon, sports car, articulated lorry (with **VECTORPIE** on both sides of the trailer), motorbike with rider, van, pickup, bus.
  - The car evades by changing lanes and brakes when no lane is free.
  - **It never drives through other vehicles** (see below).
- **Wheels** are round (ten-sided cylinders). Three spoke strokes turn with the motion, on our own car and on vehicles up to 80 m away.
- **Flyers** (`src/flyers.c`): flocks of birds in V formation that slowly flap their wings and glide. Now and then a propeller plane that crosses the road ahead and tows a **VECTORPIE** banner (confirmed in the frames).
- **Camera** (`src/car.c`): seven positions behind the car; every 18 to 32 s it glides to the next one. **The camera is never inside a vehicle:**
  - The **swing** to the right around the car (up to about 57°, 14 s) only begins once the car has changed to the right lane and 180 m are free there. The camera then swings out over the hard shoulder. If this does not succeed within 12 s, it chooses another position.
  - Sideways offset positions always stay beside the carriageway (hard shoulder on the right, central reservation on the left).
  - When gliding across the lanes, the camera rises to 6 m, above any lorry.
  - Vehicles behind the car keep a distance of 12 m.
- **Horizon** (`src/scenery.c`): backdrops on a ring 3.5 km away that turn with the curves. The sun is striped and calm. No stars.
- **Nothing pulses, flashes or blinks.** The music only smoothly determines speed and traffic density. The title appears at every track change.

## Fixed on 2026-11-05

- **"Keeps crashing":** The cause was an endless loop on camera change (`car.c`). The next camera position was re-rolled as long as it was equal to the old one, but the random value did not change inside the loop. Now the choice is loop-free. I also made backdrop changes and angle normalisation loop-free.
  - A later exit was **not a crash**: according to the event log, a press of Esc came after 87 s.
- **Camera inside vehicles** (during the swing and when gliding across the lanes): fixed as described above under "Camera". Check: 15 simulated hours, 0 cases, 157 swings, no loss of speed (504 to 533 km in 5 h).
- **Driving through other cars:** The car braked too softly (smoothly) and ploughed through slower vehicles. Now fixed rules apply (`traffic.c`):
  - The car's front never passes the rear of a vehicle in its path (`traffic_limit`).
  - It does not pull sideways into a vehicle beside it.
  - Faster vehicles from behind adjust their speed.
  - Braking is harder.
- **Guard rail posts moving along and jumping palms and billboards:** They now stand fixed to the track.

## Checked (2026-11-05, on the PC)

- PC and Pi builds without warnings in our own code. `build/saver_neondrive-0.7.2.vps` is produced (`sh tools/make_vps.sh neondrive`). Installed on the cabinet via `sh tools/deploy.sh neondrive`.
- 0.7.0 ran on the board without lost lines, but only at 28 to 30 instead of 40 frames per second. The board needed about 30 ms per frame at 1500 to 1900 lines.
- 0.7.1 (2026-11-07), 40 s on the board:
  - Mostly 40 frames per second, sending about 1 ms. In dense sections with 1700 to 1850 lines it is still 35 to 38.
  - The beam's blank path has dropped from about 260k to 100k to 140k.
  - Compute time on the Pi 3 to 6 ms.
- 0.7.2 (2026-11-07): The cross lines of the grid keep their drawing direction as they come closer (previously it flipped with every new line, which twitches on the monitor). On the board 40 frames per second, sending about 1.2 ms.
- Soak test (`saver_soak`): 5 tracks of 5 simulated hours each with a temporary collision check. **0 overlaps, no hang.**
- Frames viewed: all section types, camera swing, round wheels with spokes, plane with banner, VECTORPIE on signs, billboards and sign gantry, flock of birds. Overviews are in `screenshots/`.
- Lines per frame: usually 1100 to 1900. Rare peaks up to about 2300 (city, desert).

## Known problems and limitations

- **In dense sections on the board 35 to 38 instead of 40 frames per second** (see "Checked"). The board gets slow when the beam's whole path per frame (lit and blank) becomes too long, from about 270k. Ideas: fewer lines in the distance, or draw vehicles and objects in beam order.
- Brightness on the monitor not yet judged.
- When programs are started from within the Claude environment, they are occasionally terminated from outside (code 127/143). To view on the PC, therefore start `neondrive/run_pc.bat` by double-clicking.
- The soak test (`saver_soak`) used to open a visible window with "Not responding". That was never the screensaver itself. Now its window stays invisible and answers the window messages.

## Open tasks

- [ ] Test the `.vps` on the cabinet (`VP_STATS=1`)
- [ ] If the board flickers: thin out city and desert further (`props.c`)
- [ ] Ideas: hills, bridges over water, night sections without sun
