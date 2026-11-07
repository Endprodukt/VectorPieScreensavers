# Status

Stand 2026-11-07, Version 0.7.1: **Neon Drive** in 3D (`saver_neondrive`), gebaut mit der 3D-Schnittstelle des SDK (`vp_3d.h`) und verdeckten Linien.

## Was da ist

- **3D-Welt** in Metern, alles in einer Szene: Feste Körper verdecken, was dahinter liegt. Ein unsichtbarer Boden verdeckt alles unterhalb des Horizonts.
- **Straße** (`src/road.c`): endlos, Kurven aus geglättetem Rauschen, drei Spuren in unsere Richtung. Das pinke Grid liegt nur neben den Fahrbahnen.
- **Abschnitte** von je 520 m, nie zweimal derselbe hintereinander (`src/props.c`):
  - Strand: Palmen, Laternen, VECTORPIE-Schild
  - Stadt: Hochhäuser mit festen beleuchteten Fenstern, Laternen, Werbetafeln (eine davon groß mit VECTORPIE)
  - Wüste: Kakteen, Felsen, VECTORPIE-Schilder
  - Strom: Masten mit Leitungen, Leitplanken, Schild
  - Neon-Tunnel: Lichtbögen über der Straße
  - Autobahn: Schilderbrücken (jede zweite mit VECTORPIE), Laternen, Leitplanken, Schild
- **Alles am Rand steht fest an der Strecke**: Nichts wechselt die Seite, nichts fährt mit. Man fährt an allem vorbei.
- **Gegenfahrbahn** in etwa 45 % der Abschnitte, mit Gegenverkehr, der von vorne zu sehen ist.
- **Verkehr** (`src/traffic.c`, Modelle in `src/models.c`): Limousine, Sportwagen, Sattelzug (mit **VECTORPIE** auf beiden Seiten des Aufliegers), Motorrad mit Fahrer, Transporter, Pickup, Bus.
  - Das Auto weicht per Spurwechsel aus und bremst, wenn keine Spur frei ist.
  - **Es fährt nie durch andere Fahrzeuge** (siehe unten).
- **Räder** sind rund (zehneckige Zylinder). Drei Speichenstriche drehen sich mit der Fahrt, beim eigenen Auto und bei Fahrzeugen bis 80 m Entfernung.
- **Flieger** (`src/flyers.c`): Vogelschwärme in V-Formation, die langsam mit den Flügeln schlagen und gleiten. Ab und zu ein Propellerflugzeug, das die Straße vorne kreuzt und ein **VECTORPIE**-Banner zieht (im Bild bestätigt).
- **Kamera** (`src/car.c`): sieben Einstellungen hinter dem Auto, alle 18 bis 32 s gleitet sie zur nächsten. **Die Kamera ist nie in einem Fahrzeug:**
  - Der **Schwenk** nach rechts um das Auto (bis etwa 57°, 14 s) beginnt erst, wenn das Auto auf die rechte Spur gewechselt ist und dort 180 m frei sind. Die Kamera schwingt dann über den Seitenstreifen. Gelingt das nicht innerhalb von 12 s, wählt sie eine andere Einstellung.
  - Seitlich versetzte Einstellungen bleiben immer neben der Fahrbahn (Seitenstreifen rechts, Mittelstreifen links).
  - Beim Gleiten quer über die Spuren hebt sich die Kamera auf 6 m, über jeden LKW hinweg.
  - Fahrzeuge hinter dem Auto halten 12 m Abstand.
- **Horizont** (`src/scenery.c`): Kulissen auf einem Ring in 3,5 km Entfernung, die sich mit den Kurven drehen. Die Sonne ist gestreift und ruhig. Keine Sterne.
- **Nichts pulsiert, blitzt oder blinkt.** Die Musik bestimmt nur gleitend Tempo und Verkehrsdichte. Bei jedem Trackwechsel erscheint der Titel.

## Behoben am 2026-11-05

- **„Stürzt ständig ab“:** Ursache war eine Endlosschleife beim Kamerawechsel (`car.c`). Die nächste Kameraposition wurde neu gewürfelt, solange sie gleich der alten war, aber der Würfelwert änderte sich in der Schleife nicht. Jetzt ist die Wahl schleifenfrei. Kulissenwechsel und Winkel-Normalisieren habe ich ebenfalls schleifenfrei gemacht.
  - Ein späteres Beenden war **kein Absturz**: Laut Ereignis-Mitschnitt kam nach 87 s ein Druck auf Esc.
- **Kamera in Fahrzeugen** (beim Schwenk und beim Gleiten über die Spuren): behoben wie oben unter „Kamera“ beschrieben. Prüfung: 15 simulierte Stunden, 0 Fälle, 157 Schwenks, kein Tempoverlust (504 bis 533 km in 5 h).
- **Durchfahren anderer Autos:** Das Auto bremste zu weich (gleitend) und rauschte durch langsamere Fahrzeuge. Jetzt gelten feste Regeln (`traffic.c`):
  - Die Front des Autos kommt nie über das Heck eines Fahrzeugs in seinem Weg (`traffic_limit`).
  - Es zieht nicht seitlich in ein Fahrzeug neben sich.
  - Schnellere Fahrzeuge von hinten passen ihr Tempo an.
  - Gebremst wird kräftiger.
- **Mitfahrende Leitplanken-Pfosten und springende Palmen und Werbetafeln:** Sie stehen jetzt fest an der Strecke.

## Geprüft (2026-11-05, am PC)

- PC- und Pi-Build ohne Warnungen im eigenen Code. `build/saver_neondrive-0.7.1.vps` wird erzeugt (`sh tools/make_vps.sh neondrive`). Auf dem Automaten installiert per `sh tools/deploy.sh neondrive`.
- 0.7.0 lief auf dem Board ohne verlorene Linien, aber nur mit 28 bis 30 statt 40 Bildern pro Sekunde. Das Board brauchte etwa 30 ms pro Bild bei 1500 bis 1900 Linien.
- 0.7.1 (2026-11-07), 40 s auf dem Board:
  - Meist 40 Bilder pro Sekunde, Senden etwa 1 ms. In dichten Abschnitten mit 1700 bis 1850 Linien sind es noch 35 bis 38.
  - Der Leerweg des Strahls ist von etwa 260k auf 100k bis 140k gesunken.
  - Rechenzeit am Pi 3 bis 6 ms.
- Dauertest (`saver_soak`): 5 Strecken je 5 simulierte Stunden mit einer vorübergehenden Kollisionsprüfung. **0 Überschneidungen, kein Hänger.**
- Bilder angesehen: alle Abschnittsarten, Kamera-Schwenk, runde Räder mit Speichen, Flugzeug mit Banner, VECTORPIE auf Schildern, Tafeln und Schilderbrücke, Vogelschwarm. Übersichten liegen in `screenshots/`.
- Linien pro Bild: üblich 1100 bis 1900. Seltene Spitzen bis etwa 2300 (Stadt, Wüste).

## Bekannte Probleme und Grenzen

- **In dichten Abschnitten auf dem Board 35 bis 38 statt 40 Bilder pro Sekunde** (siehe „Geprüft“). Langsam wird das Board, wenn der ganze Weg des Strahls pro Bild (hell und dunkel) zu lang wird, ab etwa 270k. Ideen: weniger Linien in der Ferne oder Fahrzeuge und Gegenstände in Strahlreihenfolge zeichnen.
- Helligkeit am Monitor noch nicht beurteilt.
- Werden Programme aus der Claude-Umgebung heraus gestartet, werden sie gelegentlich von außen beendet (Code 127/143). Zum Ansehen am PC deshalb `neondrive/run_pc.bat` per Doppelklick starten.
- Der Dauertest (`saver_soak`) öffnete früher ein sichtbares Fenster mit „Keine Rückmeldung“. Das war nie der Screensaver selbst. Jetzt bleibt sein Fenster unsichtbar und beantwortet die Fensternachrichten.

## Offene Aufgaben

- [ ] `.vps` auf dem Automaten testen (`VP_STATS=1`)
- [ ] Falls das Board flackert: Stadt und Wüste weiter ausdünnen (`props.c`)
- [ ] Ideen: Hügel, Brücken über Wasser, Nachtabschnitte ohne Sonne
