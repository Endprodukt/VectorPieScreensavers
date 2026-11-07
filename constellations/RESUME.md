# Weitermachen

**Constellations** ist der sechste Screensaver im Repository „VectorPie Screensavers“ (Ordner `constellations/`). Bauen, Paketieren und Installieren: siehe `../BUILD.md`. Stand: siehe `STATUS.md`.

## Wünsche des Nutzers

- **Echte Sternbilder** (2026-11-07).
- **Sterne müssen funkeln**, sanft und gleitend, jeder in seinem eigenen Takt (2026-11-07).
- Nichts darf pulsieren oder blinken. Musik ändert Dinge nur gleitend (siehe `../neondrive/RESUME.md`).

## Aufbau

- `src/sky.h`: die Daten.
  - Je Sternbild seine Sterne mit Rektaszension (Stunden), Deklination (Grad), Helligkeit, Farbe und gegebenenfalls Namen.
  - `path`: die Figur als Folge von Sternnummern, −1 beginnt eine neue Linie.
  - `shared`: derselbe Stern in zwei Figuren (Elnath, Alpheratz). Er wird nur einmal gezeichnet.
  - `SKY_CON` ist zugleich die Reihenfolge des Rundflugs.
  - `SKY_LONE`: helle Einzelsterne.
- `src/main.c`:
  - `sky_make`: Richtungen aller Sterne, Mitte und Größe jeder Figur, dazu die schwachen Sterne (Milchstraße über den galaktischen Nordpol und das galaktische Zentrum).
  - Projektion: stereografisch um `G.mid`, Norden oben, Osten links. `G.half` ist die halbe Bildbreite (`con_half`).
  - `tour_update`: Gleiten (`glide`), Einzeichnen (`fig`), Stehen (`hold`), dann `tour_next`.
  - `stars_draw` sortiert die sichtbaren Sterne in Bändern hin und her, damit der Strahl kurze Wege hat.
  - Die Zeichenrichtung jedes Sterns und jeder Figurenlinie ist fest: Sterne von links nach rechts und von unten nach oben, Linien in der Reihenfolge des Pfads (siehe `../neonridge/RESUME.md`, „Zeichenrichtung muss fest bleiben“).
- Zum Testen: `SAVER_TEMPO=<Faktor>` macht den Rundflug schneller, `SAVER_SHOTS=<Ordner>` speichert alle 2 s ein Bild.
