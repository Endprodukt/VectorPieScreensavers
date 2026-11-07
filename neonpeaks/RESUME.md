# Weitermachen

**Neon Peaks** ist der vierte Screensaver im Repository „VectorPie Screensavers“ (Ordner `neonpeaks/`). Bauen, Paketieren und Installieren: siehe `../BUILD.md`. Stand: siehe `STATUS.md`.

## Wünsche des Nutzers

- Im Stil von Retrowave-Mountains-Videos (Vorlage: YouTube „4K Retrowave Mountains Screensaver“): Neon-Gitter, Drahtgitterberge, gestreifte Sonne.
- **Hier darf der Screensaver auf die Musik reagieren** (2026-11-07).
- **Farben:** nur Neonfarben, die ineinander überblenden. Am besten gefiel dem Nutzer der erste Satz (Magenta und Cyan); Grün und Orange gefielen weniger (2026-11-07). Trotzdem gilt: nichts blinkt oder pulsiert, alles ändert sich gleitend (siehe `../neondrive/RESUME.md`).

## Aufbau (alles in `src/main.c`)

- `G.h[RING][COLS]`: die Höhen jeder Geländereihe (Reihe n liegt bei z = n·DZ). `make_row` erzeugt sie, sobald sie gebraucht werden, aus Rauschen mal Musik (`G.bands`, geglättet). `height` lässt sie am Horizont aus der Ebene wachsen.
- `ground_draw`:
  - unsichtbare Dreiecke (`vp3d_scene_polygon` mit Maske 0) als Verdeckung. Dreiecke, weil Vierecke am Hang nicht eben sind und sonst ihre eigenen Kanten verdecken würden.
  - Darauf die Gitterlinien als `vp3d_scene_line`, in Schlangenlinien: Jedes Stück beginnt, wo das letzte endete. Das senkte auf dem Board die Sprünge von 470 auf 80 und die Sendezeit von 25 auf 0,5 ms. **Dieser Trick könnte auch Neon Drive (28–30 fps, Senden etwa 30 ms) helfen.**
- `sun_draw` und `range_draw`: Sonne und ferne Kette wandern mit der Kamera mit und werden so nie erreicht. Ein unsichtbarer Boden bei y = 0 verdeckt ihre unteren Teile.
- `PAL`: 5 Farbsätze (Boden, Hang, Gipfel, Sonne oben, Sonne unten, Kette). `pal()` blendet über. Jeder wird `PAL_HOLD` gehalten, die Überblendung dauert `PAL_BLEND`, ein neuer Track startet die nächste.
