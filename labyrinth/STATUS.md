# Status

Stand 2026-11-07, Version 0.1.0: **Labyrinth** (`saver_labyrinth`).

## Was da ist (`src/main.c`)

- **Labyrinthe:** jedes zufällig, 9 bis 16 Zellen pro Seite. Jede Zelle ist auf genau einem Weg erreichbar. Zellen sind 2,4 m groß, die Wände 2,2 m hoch, das Auge ist in halber Höhe.
- **Der Gang:** Der Wanderer hält die rechte Hand an der Wand und kommt so immer zum Ausgang.
  - Er bleibt nie stehen. Kurven nimmt er im Bogen und dreht sich dabei; in Sackgassen läuft er hinein, wendet nach rechts und läuft zurück.
  - Tempo: 2 Zellen pro Sekunde, bei lauter Musik gleitend bis 3,2. In Kurven geht er etwas langsamer, beim Wenden deutlich.
- **Wände:** Je Labyrinth ein Muster: Ziegel, Kassetten, Kacheln oder Streifen mit Raute. Das Muster wird nur bis etwa 5 Zellen Entfernung gezeichnet, dahinter wäre es im Nebel ohnehin verloren. Dazu ein Gitter auf Boden und Decke und Nebel, der mit der Entfernung abdunkelt.
- **VECTORPIE-Schilder:** auf etwa jeder elften Wand, auf einer Seite.
- **Kristalle:** 3 pro Labyrinth, auf dem Weg. Wer hindurchläuft, dreht die Welt in 3 s langsam auf den Kopf; der nächste Kristall dreht sie zurück.
  - Ein Kristall steht nur auf dem ersten Besuch einer Zelle und ist eine halbe Zelle vor dem Wanderer verschwunden. So kommt die Kamera nie in ihn hinein.
  - Früher konnte das passieren; seine Kanten zogen dann als schräge Striche durchs Bild (Rückmeldung 2026-11-07).
- **Ausgang:** ein drehender Kuchen mit fehlendem Stück. Davor blendet das Labyrinth aus und das nächste in anderen Farben wieder ein. Es gibt 5 Farbsätze.
- **Musik:** Das Tempo folgt gleitend der Lautstärke. Bei einem neuen Track erscheint 7 s lang sein Titel. Nichts pulsiert oder blinkt.
- Für den PC:
  - `SAVER_SHOTS=<ordner>` speichert alle 2 s ein Bild.
  - `SAVER_TEMPO=<faktor>` läuft entsprechend schneller.

## Geprüft (2026-11-07)

- PC- und Pi-Build ohne Warnungen. `build/saver_labyrinth-0.1.0.vps` wird erzeugt.
- Am PC Bilder angesehen: Muster, Schilder, Kristalle, das Umdrehen, der Ausgang und mehrere Labyrinthe nacheinander (100 s mit 8-fachem Tempo).
  - 38 bis 41 fps.
  - Höchstens etwa 290 Linien pro Bild.
- **Auf dem Automaten installiert** (`sh tools/deploy.sh labyrinth`) und 25 s auf dem Board getestet:
  - gleichmäßig 40 Bilder pro Sekunde
  - 50 bis 126 Linien pro Bild
  - keine verlorenen Linien

## Bekannte Probleme und Grenzen

- An Kreuzungen, wo eine Querwand auf eine durchgehende Wand trifft, wird die senkrechte Ecklinie manchmal von beiden Wänden gezeichnet, also doppelt. Das ist unschön, aber harmlos.
- Wie es am echten Monitor aussieht, ist noch nicht beurteilt: Helligkeit der Muster, Tempo und Kurven.

## Offene Aufgaben

- [ ] Am Automaten ansehen: Tempo, Kurven, Helligkeit
- [ ] Unter SETTINGS → SCREENSAVERS → SELECT auswählen (das geht nur am Automaten)
- [ ] Mögliche Erweiterungen: ein Wesen, das durch die Gänge läuft; Labyrinthe mit Räumen
