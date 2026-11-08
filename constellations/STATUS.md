# Status

Status as of 2026-11-08, version 0.1.2: **Constellations** (`saver_constellations`).

## What it shows

- **26 real constellations** in a round tour, each close to the previous one: Great Bear, Little Bear, Dragon, Hercules, Northern Crown, Herdsman, Virgin, Scorpion, Archer, Eagle, Dolphin, Lyre, Swan, Cepheus, Cassiopeia, Andromeda, Pegasus, Ram, Perseus, Charioteer, Bull, Orion, Great Dog, Little Dog, Twins, Lion. The start is a different one each time.
- **Sequence per constellation:**
  1. The view glides there along a great circle, 5 to 11 s.
  2. The figure draws itself in line by line over 4 s.
  3. It stays for 11 s while the view moves slightly closer.
  4. As the view glides on, it fades out over 2.5 s.
- **Labels:** The English name is at the bottom, with the Latin one below it (only one line where both are the same, e.g. ORION). The bright stars of the figure are named in their English spelling (Betelgeuse, Rigel, Vega, Pleiades …). Since 0.1.2; before that the names were German.
- **The other figures** stand faintly in the background, as in a star atlas.
- **Stars:**
  - The stars of the figures and 22 bright stars beside them have their real positions (J2000), magnitudes and colours: Betelgeuse and Antares reddish, Rigel and Vega bluish.
  - Bright stars are small crosses, faint ones dots.
  - In addition there are 1500 faint stars. Two thirds of them lie along the Milky Way, at its real location and denser towards the centre. Their individual positions are invented.
- **All stars twinkle gently:** Each one becomes smoothly brighter and darker at its own rate (1.7 to 5 s).
- **Shooting stars:** About one every 30 s, fading smoothly in and out.
- **Music, everything smooth:** With loud music the tour goes a little faster, the stars twinkle more lively and shooting stars come more often. On a new track its title is shown at the top for 7 s.

## Checked

- 0.1.2: All names in English: the 26 constellations (e.g. SWAN / CYGNUS, GREAT BEAR / URSA MAJOR) and ten star names that were spelled the German way (VEGA, ALTAIR, BETELGEUSE, POLARIS, PLEIADES …). Checked on the PC with screenshots.

- 0.1.1: The constellations have their real proportions on the 4:3 monitor. Previously they were a third too wide.

- PC and Pi builds without warnings. `build/saver_constellations-0.1.0.vps` is produced.
- Frames viewed on the PC, at four times the speed over two rounds: the shapes are right, for example Orion, the Square of Pegasus, the W of Cassiopeia, the cross of the Swan and the teapot of the Archer. The orientation is right too, north at the top and east on the left as in the sky.
- On the PC 40 frames per second with 150 to 300 lines.

## Open

- Installed on 2026-11-08 (0.1.2): 40 frames per second, about 200 lines, no dropped lines. The English names are not yet judged on the real monitor.

- Installed on 2026-11-08 (0.1.1): 40 frames per second, 70 to 230 lines, no dropped lines. Not yet judged on the real monitor.
- Not yet judged on the real monitor: whether the faint stars and the faint figures are visible enough.
- A few figure lines are simplified (Great Bear, Dragon, Virgin), the way many star charts draw them.
