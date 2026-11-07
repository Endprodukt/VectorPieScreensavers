# Weitermachen

**Neon Ridge** ist der fünfte Screensaver im Repository „VectorPie Screensavers“ (Ordner `neonridge/`). Bauen, Paketieren und Installieren: siehe `../BUILD.md`. Stand: siehe `STATUS.md`.

## Wünsche des Nutzers

- **So wie im Video** „4K Retrowave Mountains Screensaver“ (YouTube `M_SRn64CiyM`), 2026-11-07, nachdem Neon Peaks zu frei interpretiert war.
- Das Video zeigt:
  - eine stehende Kamera und einen Sternenhimmel
  - einen cyan leuchtenden Horizont
  - ein dunkles Bergmassiv mit wellenförmigen Höhenlinien und hohem Mittelgipfel
  - eine dunkle Ebene mit waagrechten Linien
  - eine pinke Lichtspur, die von unten links nach rechts schwingt und am Horizont zurückläuft
  - keine Sonne
- Er darf auf die Musik reagieren, aber nichts darf pulsieren oder blinken (siehe `../neondrive/RESUME.md`).

## Aufbau (alles in `src/main.c`)

- `range_make`: Höhen `MH` aus Gipfeln (`peak`) plus Rauschen. `range_draw` zeichnet unsichtbare Dreiecke zur Verdeckung und darauf die Reihen in Schlangenlinie.
- `sky_draw`: das Leuchten als Linien in 4000 m Entfernung (hinter den Bergen) und die Sterne in 5000 m. `stars_make` sortiert die Sterne in Bändern hin und her, damit der Strahl kurze Wege hat. `STAR[i][3]` und `[4]` sind Takt und Phase des Funkelns.
- **Sterne müssen funkeln** (Wunsch des Nutzers, 2026-11-07), sanft und gleitend, jeder in seinem eigenen Takt.
- `plain_draw`: die Linien mit Abstand `P_RATIO`; `G.flow` ist die Phase in Linien.
- `trail_at(s)`: der Weg der Spur, in Bildschirmbreiten `u` (−1 links, 1 rechts) und Entfernung `z`. `trail_draw` zeichnet drei Linien: Schein, Kern zurück, Schein.
- Reihenfolge der Linien beachten: siehe `../neondrive/RESUME.md` („Tempo auf dem Board“).

## Zeichenrichtung muss fest bleiben

Eine Linie, die andersherum gezeichnet wird, liegt auf dem Monitor ein wenig versetzt. Wenn Linien hin und her gezeichnet werden, muss die Richtung deshalb an der Linie selbst hängen (an ihrer eigenen Nummer), nicht an ihrem Platz in der Schleife. Sonst kippt die Richtung jedes Mal, wenn eine Linie nachrückt, und alle Linien zucken links und rechts (Rückmeldung des Nutzers zu Neon Peaks, 2026-11-07; seit Neon Peaks 0.1.2, Neon Ridge 0.1.2, Neon Drive 0.7.2 behoben).
