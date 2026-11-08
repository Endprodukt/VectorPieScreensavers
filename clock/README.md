# Clock

Die Uhrzeit auf fünfzehn Zifferblättern:

- **ANALOG:** Uhr mit Rand, Zahlen und Datumsfenster
- **MINIMAL:** ohne Rand und ohne Zahlen, nur zwölf Striche, schmale Zeiger
- **NUMERALS:** nur die zwölf Zahlen, Balkenzeiger
- **ROMAN:** römische Zahlen, Breguet-Zeiger (mit Ring vor der Spitze)
- **SQUARE:** quadratisches Zifferblatt, Pfeilzeiger
- **FULL SCREEN:** ohne Rahmen, die Striche laufen an den Bildrändern entlang, lange spitze Zeiger reichen bis dorthin
- **DIGITAL:** Siebensegment-Anzeige mit Datum
- **FLIP:** Fallblattanzeige, deren Karten umklappen
- **BINARY:** jede Ziffer als Bits
- **WORDS:** Wortuhr in der eingestellten Sprache
- **NIXIE:** Glimmröhren
- **ORBIT:** Stunden, Minuten und Sekunden kreisen wie Planeten
- **TUNNEL:** ein Röhrennetz wie bei den alten Vektor-Röhrenshootern
- **ROCKS:** treibende Felsen und ein kleines Schiff wie bei den alten Vektor-Felsenshootern
- **WEATHER:** das Wetter jetzt und die nächsten Tage

Jedes Zifferblatt hat eigene, aufeinander abgestimmte Farben, etwa bei DIGITAL Stunden cyan, Minuten magenta, Sekunden gelb. Unter jedem Zifferblatt steht eine Zeile mit Datum und Wetter.

## Das Menü der Uhr

**F2 oder Tab** öffnet es, während die Uhr läuft, und schließt es wieder. Darin bewegt man sich mit dem Joystick. Button 1 oder START 1 wählt aus, Button 2 oder START 2 geht zurück und schließt das Menü. Nach einer Minute ohne Eingabe schließt es sich selbst.

Einstellbar ist:
- **FACE:** das Zifferblatt
- **RANDOM FACE:**
  - aus (Standard)
  - beim Start ein zufälliges Zifferblatt
  - immer, also auch bei jedem Wechsel
- **CHANGE FACE:** ob das Zifferblatt von selbst wechselt:
  - aus (Standard)
  - mit jedem Track
  - alle 1, 5, 15 oder 60 Minuten
  - nach jedem Wetterbericht
- **SWITCH FACE WITH:** womit man zum nächsten Zifferblatt schaltet:
  - SPINNER (Standard)
  - LEFT / RIGHT: Joystick links und rechts
  - CUSTOM: eine eigene Taste. Mit Button 1 auf der Zeile wird sie gelernt: „PRESS A BUTTON“, dann die gewünschte Taste drücken (F2 oder Tab bricht ab). Die Zeile zeigt sie dann, etwa „CUSTOM: BUTTON 3“.
  - NONE
- **CLOCK COLOR, DATE COLOR, WEATHER COLOR:** Farben für Uhr, Datum und Wetter, jede für sich. Zur Wahl stehen:
  - ORIGINAL: die eigenen, bunten Farben des Zifferblatts
  - eine Farbe für alles: Cyan, Grün, Bernstein, Rot, Magenta, Blau, Weiß oder Regenbogen
  - RANDOM SINGLE: mit jedem Zifferblatt eine neue Farbe aus den sieben einfachen, nie zweimal dieselbe hintereinander
  - RANDOM MULTI: mit jedem Zifferblatt neue bunte Farben. Alle eigenen Farben werden gemeinsam ein zufälliges Stück um den Farbkreis gedreht, so passen sie weiter zueinander.
- **SECONDS:** aus, SMOOTH (der Zeiger gleitet, Standard) oder TICK (er springt jede Sekunde)
- **Was sonst angezeigt wird:** DATE, WEATHER LINE (die Wetterzeile unter der Uhr), 24 HOURS (sonst 12) und TRACK TITLE, jeweils an oder aus
- **WEATHER REPORT:** der Wetterbericht als Vollbild, aus oder alle 1, 2, 3, 5, 10, 15, 30 oder 60 Minuten. Er blendet über die Uhr und danach wieder zurück.
- **REPORT LENGTH:** wie lange er bleibt, 10, 20, 30 oder 60 s
- **WEATHER STYLE:** wie das Wetter gezeigt wird, im Wetterbericht und auf dem Zifferblatt WEATHER:
  - OVERVIEW: jetzt und die nächsten 4 Tage
  - SIMPLE: groß Symbol, Temperatur und Wetter
  - TODAY: dazu gefühlte Temperatur, Min/Max, Regen, Wind mit Richtung, Luftfeuchte, Sonnenauf- und -untergang
  - DAYS: 7 Tage mit Min, Max und Regenwahrscheinlichkeit
  - HOURS: die nächsten 24 Stunden als Temperaturkurve, mit Symbolen und Regenbalken
  - ROTATING: jedes Mal die nächste Ansicht
- **WEATHER COLORS:** bunt (Temperaturen von Blau bis Rot, Sonne gelb, Regen blau) oder einfarbig in der Wetterfarbe
- **PLACE:** Ortszeit des Automaten, eine Stadt aus der Liste, oder eine beliebige Stadt per Suche über eine Bildschirmtastatur. Der Ort bestimmt Uhrzeit (mit Sommerzeit) und Wetter.
- **LANGUAGE:** Englisch (Standard), Deutsch, Französisch, Spanisch

Die Einstellungen bleiben in `save/clock.ini` neben dem Programm erhalten.

Der **Spinner** (oder was unter SWITCH FACE WITH eingestellt ist) schaltet durch die Zifferblätter. Nach dem letzten (und vor dem ersten) kommt der Wetterbericht. Er bleibt so lange wie eingestellt (REPORT LENGTH), auch wenn der Wetterbericht im Takt aus ist. Im Menü bewegt der Spinner durch die Zeilen.

Außerhalb des Menüs spielt **START 2** den nächsten Track. Jede andere Taste, auch START 1, weckt den Automaten wie bei jedem Screensaver.

- Starten am PC: `run_pc.bat` per Doppelklick. Tab, F2, Enter oder M öffnet das Menü, die Pfeiltasten bewegen darin (außerhalb schalten links und rechts wie der Spinner), Backspace geht zurück, N spielt den nächsten Track.
- Stand und offene Punkte: [STATUS.md](STATUS.md)
- Für die Weiterarbeit: [RESUME.md](RESUME.md)
- Bauen und Installieren: [../BUILD.md](../BUILD.md)
