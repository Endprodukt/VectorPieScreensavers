# Status

Stand 2026-11-07, Version 0.1.1: **Neon Ridge** (`saver_neonridge`).

## Was da ist (`src/main.c`)

- **Kamera:** steht still in 8 m Höhe und blickt leicht nach oben, sodass der Horizont unter der Bildmitte liegt (wie im Video).
- **Bergmassiv:**
  - Aufbau: ein hoher Gipfel in der Mitte, niedrigere daneben und dazwischen ein Grat.
  - Gezeichnet als 30 Reihen von Höhenlinien quer, 700 bis 1150 m entfernt. Die vorderen Reihen verdecken die hinteren.
  - Höhere Linien sind heller und blauer.
  - Die Details sind bei jedem Start etwas anders.
- **Himmel:** 140 Sterne, die sanft funkeln: Jeder wird in seinem eigenen Takt (alle 1,7 bis 5 s) gleitend heller und dunkler, aus zwei überlagerten Wellen, damit es unregelmäßig wirkt (seit 0.1.1, Wunsch des Nutzers). Dazu ein Leuchten über dem Horizont aus 9 Linien, unten cyan, nach oben blau und schwächer.
- **Ebene:** waagrechte Linien, die langsam herankommen, mit perspektivischen Abständen. Ihr ferner Rand ist eine lange Linie, links pink und rechts cyan.
- **Lichtspur:** ein heller Kern mit pinkem Schein links und rechts. Sie kommt unten links herein, schwingt nach rechts bis kurz unter den Horizont und läuft am Horizont entlang zurück nach links. Sie schwingt langsam etwas mit.
- **Musik, gleitend:**
  - Die Ebene kommt bei lauter Musik schneller heran: 0,35 bis 1,35 Linien pro Sekunde.
  - Die Lichtspur schwingt schneller.
  - Das Horizontleuchten wird heller.
  - Bei einem neuen Track erscheint 7 s lang sein Titel. Nichts blinkt oder pulsiert.
- Für den PC: `SAVER_SHOTS=<ordner>` speichert alle 2 s ein Bild.

## Geprüft (2026-11-07)

- PC- und Pi-Build ohne Warnungen. `build/saver_neonridge-0.1.1.vps` wird erzeugt. 0.1.1 auf dem Board: 40 Bilder pro Sekunde, etwa 515 Linien, Senden 0,6 ms, keine verlorenen Linien.
- Am PC Bilder angesehen und mit einem Standbild des Videos verglichen.
- **Auf dem Automaten installiert** (`sh tools/deploy.sh neonridge`) und 15 s auf dem Board getestet:
  - gleichmäßig 40 Bilder pro Sekunde
  - etwa 500 Linien pro Bild
  - Senden 0,6 ms, keine verlorenen Linien

## Bekannte Probleme und Grenzen

- Das weiche Leuchten und die Spiegelungen des Videos gibt es auf einem Vektormonitor nicht. Sie sind hier als Linien angedeutet.
- Über dem Hauptgipfel steht manchmal ein einzelner Strich einer hinteren Linienreihe heraus.
- Am echten Monitor noch nicht beurteilt: Helligkeit von Sternen und Leuchten, und ob das Funkeln der Sterne schnell genug ist.

## Offene Aufgaben

- [ ] Am Automaten ansehen
- [ ] Unter SETTINGS → SCREENSAVERS → SELECT auswählen (das geht nur am Automaten)
