# Stand

Stand 2026-11-07, Version 0.1.0: **Clock** (`saver_clock`).

## Was er zeigt

- **Zehn Zifferblätter:** ANALOG, DIGITAL, KLAPPZAHLEN, BINAER, WORTUHR, NIXIE, ORBIT, TUNNEL, ROCKS und WETTER (siehe README). Eines blendet in 1,4 s ins nächste über. Der Name des neuen steht 3,5 s lang klein unten.
- **Datum und Wetter:** Unter jedem Zifferblatt außer WETTER steht eine Zeile wie „SA 07.11. BERLIN 3 GRAD BEDECKT“.
- **WETTER:** Ort und Uhrzeit, ein Symbol (Sonne oder Mond, Wolken, Regen, Schnee, Nebel, Gewitter), die Temperatur groß, Wind, Luftfeuchte und die Vorhersage für heute und die drei nächsten Tage.
- **Woher das Wetter kommt:**
  - `src/weather.c` holt es in einem eigenen Thread mit `curl` (oder `wget`) von Open-Meteo, kostenlos und ohne Schlüssel. Das geschieht alle 15 Minuten, nach einem Fehler jede Minute.
  - Der Ort steht in `clock.ini` neben dem Programm: `place = Berlin` oder `lat`, `lon` und `name`.
  - Ohne Ort oder ohne Netz zeigt die Uhr kein Wetter.
- **Umschalten:** Jeder neue Track bringt das nächste Zifferblatt, also auch die Taste NEXT TRACK. Am PC gehen auch die Pfeiltasten.
- **Musik, alles gleitend:** Bei lauter Musik leuchten Ringe und Marken etwas heller. Bei einem neuen Track steht 7 s lang sein Titel oben. Nichts blinkt, auch die Doppelpunkte stehen still.

## Geprüft

- PC-Build ohne Warnungen.
- Am PC alle Zifferblätter angesehen: Die Zeit stimmt auf allen, zum Beispiel BINAER, WORTUHR („ES IST ZEHN VOR NEUN“) und ORBIT gegen 20:51. Das Wetter für Berlin wurde geholt und angezeigt.
- Am PC 40 Bilder pro Sekunde, bis etwa 900 Linien.

## Offen

- **Noch nicht auf dem Automaten:** Am 2026-11-07 war er per SSH nicht erreichbar (Ping ging). Dort ist noch zu prüfen:
  - ob `curl` oder `wget` da ist
  - ob die Zeitzone des Pi stimmt
  - der Pi-Build mit `pthread`
  - das Tempo auf dem Board
- **Der Ort ist ein Platzhalter (Berlin)**, bis der Nutzer seinen nennt.
- **Ein eigenes Menü im Screensaver** ist noch nicht möglich: Das Menü des Automaten beendet ihn bei jeder Taste außer NEXT TRACK (siehe RESUME).
- Am echten Monitor noch nicht beurteilt.
