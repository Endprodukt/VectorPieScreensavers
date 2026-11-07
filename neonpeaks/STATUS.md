# Status

Stand 2026-11-07, Version 0.1.1: **Neon Peaks** (`saver_neonpeaks`).

## Was da ist (`src/main.c`)

- **Tal:** ein flaches Gitter, 32 m breit. Die Kamera fliegt in 5 m Höhe darüber und driftet langsam etwas seitwärts.
- **Berge:** links und rechts als Drahtgitter, aber massiv: Sie verdecken, was hinter ihnen liegt. Das Gelände hat 24 Reihen zu je 10 m und 21 Punkte quer, außen weiter auseinander. Es ist fest in der Welt, man fliegt also daran vorbei.
  - Neue Reihen wachsen am Horizont aus der Ebene, sie springen nicht auf.
  - Das Gelände wird mit der Entfernung dunkler. Das ist ein eigener Nebel, der Nebel der Ansicht würde auch die Sonne dimmen.
- **Sonne:** ein Kreis aus waagrechten Linien, die langsam nach unten gleiten. In der unteren Hälfte werden die Lücken breiter, das gibt die typischen Streifen. Davor steht eine ferne Bergkette, die man nie erreicht.
- **Musik:**
  - Jede neue Reihe am Horizont bekommt ihre Form aus dem Spektrum, das gerade läuft. Die Bässe formen die Berge nah am Tal, die Höhen die äußeren. Laute Passagen ergeben hohe Berge, die dann heranfliegen.
  - Das Spektrum ist geglättet, damit keine Zacken entstehen.
  - Die Fluggeschwindigkeit liegt bei 20 bis 42 m/s und folgt gleitend der Lautstärke.
  - **Farben:** 5 Neon-Farbsätze (Magenta, Cyan, Pink, Violett, Blau; Grün und Orange sind raus). Sie blenden ständig ineinander über: Jeder wird 18 s gehalten, dann folgt eine Überblendung von 8 s. Ein neuer Track startet die nächste Überblendung sofort. Der Start ist immer der klassische Satz (Magenta-Gitter, Cyan-Gipfel).
  - Bei einem neuen Track erscheint 7 s lang sein Titel.
  - Nichts blinkt oder pulsiert.
- Für den PC: `SAVER_SHOTS=<ordner>` speichert alle 2 s ein Bild.

## Geprüft (2026-11-07)

- PC- und Pi-Build ohne Warnungen. `build/saver_neonpeaks-0.1.1.vps` wird erzeugt.
- Am PC Bilder angesehen, mit Farbwechsel beim Track-Wechsel.
- **Auf dem Automaten installiert** (`sh tools/deploy.sh neonpeaks`) und 25 s auf dem Board getestet:
  - gleichmäßig 40 Bilder pro Sekunde
  - etwa 520 Linien pro Bild, aber nur etwa 80 Sprünge
  - Senden 0,5 ms, keine verlorenen Linien

## Bekannte Probleme und Grenzen

- Am echten Monitor noch nicht beurteilt: Helligkeit, Tempo, wie stark man die Musik in den Bergen erkennt.

## Offene Aufgaben

- [ ] Am Automaten ansehen und mit echter Musik beurteilen
- [ ] Unter SETTINGS → SCREENSAVERS → SELECT auswählen (das geht nur am Automaten)
