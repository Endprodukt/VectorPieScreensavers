# Clock

Die Uhrzeit auf zehn Zifferblättern:

- **ANALOG:** Uhr mit gleitendem Sekundenzeiger und Datumsfenster
- **DIGITAL:** Siebensegment-Anzeige mit Datum
- **FLIP:** Fallblattanzeige, deren Karten umklappen
- **BINARY:** jede Ziffer als Bits
- **WORDS:** Wortuhr in der eingestellten Sprache
- **NIXIE:** Glimmröhren
- **ORBIT:** Stunden, Minuten und Sekunden kreisen wie Planeten
- **TUNNEL:** ein Röhrennetz wie bei den alten Vektor-Röhrenshootern
- **ROCKS:** treibende Felsen und ein kleines Schiff wie bei den alten Vektor-Felsenshootern
- **WEATHER:** das Wetter jetzt und die nächsten Tage

Unter jedem Zifferblatt steht eine Zeile mit Datum und Wetter.

## Das Menü der Uhr

**START 1** öffnet es, während die Uhr läuft. Darin bewegt man sich mit dem Joystick. Button 1 oder START 1 wählt aus, Button 2 oder START 2 geht zurück und schließt das Menü. Nach einer Minute ohne Eingabe schließt es sich selbst.

Einstellbar ist:
- **FACE:** das Zifferblatt
- **CHANGE FACE:** ob das Zifferblatt von selbst wechselt: aus (Standard), mit jedem Track, oder alle 1, 5, 15 oder 60 Minuten
- **COLOR:** die eigenen Farben der Zifferblätter oder eine Farbe für alles (Cyan, Grün, Bernstein, Rot, Magenta, Blau, Weiß, Regenbogen)
- **Was angezeigt wird:** SECONDS, DATE, WEATHER, 24 HOURS (sonst 12) und TRACK TITLE, jeweils an oder aus
- **PLACE:** Ortszeit des Automaten, eine Stadt aus der Liste, oder eine beliebige Stadt per Suche über eine Bildschirmtastatur. Der Ort bestimmt Uhrzeit (mit Sommerzeit) und Wetter.
- **LANGUAGE:** Englisch (Standard), Deutsch, Französisch, Spanisch

Die Einstellungen bleiben in `save/clock.ini` neben dem Programm erhalten.

Außerhalb des Menüs spielt **START 2** den nächsten Track. Jede andere Taste weckt den Automaten wie bei jedem Screensaver.

- Starten am PC: `run_pc.bat` per Doppelklick. Enter oder M öffnet das Menü, die Pfeiltasten bewegen darin (außerhalb wechseln sie das Zifferblatt), Backspace geht zurück, N spielt den nächsten Track.
- Stand und offene Punkte: [STATUS.md](STATUS.md)
- Für die Weiterarbeit: [RESUME.md](RESUME.md)
- Bauen und Installieren: [../BUILD.md](../BUILD.md)
