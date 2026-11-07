# Weitermachen

**Labyrinth** ist der dritte Screensaver im Repository „VectorPie Screensavers“ (Ordner `labyrinth/`). Er ist eine eigene Vektor-Version des klassischen Windows-Labyrinth-Screensavers. Bauen, Paketieren und Installieren: siehe `../BUILD.md`. Stand: siehe `STATUS.md`.

## Wünsche des Nutzers

- „Der klassische Windows-Labyrinth-Screensaver“, aber nichts aus dem Original übernehmen: eigene Objekte (Kristalle, ein Kuchen als Ausgang, VECTORPIE-Schilder) und eigene Muster.
- **Viel Bewegung** (Rückmeldung 2026-11-07, mit Verweis auf ein Video des Originals): nie anhalten, Kurven im Laufen nehmen. Das Stehenbleiben und Drehen auf der Stelle aus dem ersten Entwurf ist deshalb entfernt.
- **Über Kopf drehen, wie im Original, aber nicht ständig** (Rückmeldung 2026-11-07, nachdem das Drehen zuerst ganz entfernt war): Ein Kristall dreht die Welt langsam um (`ROLL_S`, 3 s). Danach läuft der Wanderer eine Strecke über Kopf (`UPSIDE`, 10 Zellen) und dreht dann von selbst zurück. Kristalle, durch die er über Kopf oder während des Drehens läuft, ändern nichts. Ein neues Labyrinth beginnt immer auf dem Boden.
- Wie bei allen Screensavern hier: nichts pulsiert oder blinkt (siehe `../neondrive/RESUME.md`).

## Aufbau (alles in `src/main.c`)

- `M`: das Labyrinth.
  - `hw[y][x]` ist die Wand entlang z = y·S, `vw[y][x]` die Wand entlang x = x·S.
  - `maze_new` erzeugt es (Tiefensuche) und berechnet gleich den ganzen Weg nach der Rechte-Hand-Regel: `path[i]` = Zelle und Richtung, in der sie betreten wurde.
- `walk_at(u)` liefert Ort und Blickrichtung bei `u` Zellen entlang des Wegs. Jede Ecke ist eine quadratische Bézierkurve von einer halben Zelle davor bis eine halbe Zelle danach; die Blickrichtung dreht dabei weich mit (beim Wenden um π nach rechts). `G.u` ist der Fortschritt.
- `maze_draw`:
  - Wände bis `SEEN` (9) Zellen als `vp3d_scene_polygon`. Gezeichnet werden Ober- und Unterkante, senkrechte Kanten nur an Wandenden.
  - Muster (`PATTERN`) als Linien in der Wandebene, sichtbar von beiden Seiten. Contact 0,05 sorgt dafür, dass die eigene Wand sie nicht verdeckt.
  - Schilder 6 cm vor der Wand, damit sie nur auf einer Seite zu sehen sind.
- Kristall (Oktaeder) und Kuchen sind Meshes aus `make_crystal` und `make_pie`.
- Linienbudget: etwa 50 bis 300 Linien, auf dem Board 40 fps mit viel Luft.
