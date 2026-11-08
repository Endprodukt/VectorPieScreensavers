# Stand

Stand 2026-11-08, Version 0.4.0: **Clock** (`saver_clock`).

## Was er kann

- **Zehn Zifferblätter** (siehe README). Eines blendet in 1,4 s ins nächste über, der Name des neuen steht kurz unten.
- **Das eigene Menü** (F2 oder Tab, ebenso zu) hat 18 Zeilen und blättert. Einstellbar sind:
  - Zifferblatt und Zufall (aus, beim Start, immer)
  - Wechsel (aus, mit jedem Track, alle 1, 5, 15 oder 60 Minuten, nach jedem Wetterbericht)
  - die Farben von Uhr, Datum und Wetter, jede für sich, auch RANDOM: mit jedem Zifferblatt eine neue der sieben einfachen Farben. Beim Überblenden behält das alte Zifferblatt seine Farbe.
  - Sekunden, Datum, Wetterzeile, 24 Stunden, Tracktitel
  - Wetterbericht (alle 5 bis 60 Minuten) und seine Dauer (10 bis 60 s)
  - Wetter-Ansicht und Wetterfarben (bunt oder einfarbig)
  - Ort und Sprache
  - Eine Änderung wirkt sofort.
  - Beim Schließen wird sie in `save/clock.ini` gespeichert.
  - Ohne Eingabe schließt sich das Menü nach 60 s.
- **Ort:**
  - Ortszeit des Automaten, eine von 59 Städten oder eine Suche über eine Bildschirmtastatur (Open-Meteo-Geocoding, bis zu 6 Treffer).
  - Uhrzeit und Wetter folgen dem Ort. Den Abstand zu UTC samt Sommerzeit liefert Open-Meteo mit dem Wetter.
  - Ohne Ort gilt die Ortszeit des Automaten, und es gibt kein Wetter.
- **Sprachen:** Englisch (Standard), Deutsch, Französisch und Spanisch. Das gilt für Wochentage, Monate, Wetterwörter, Menü, die Namen der Zifferblätter und die Wortuhr. Die Wortuhr hat für jede Sprache ein eigenes Raster und eigene Grammatik, etwa „ES IST FUENF VOR HALB DREI“, „IL EST DEUX HEURES MOINS LE QUART“ oder „SON LAS DOS Y MEDIA“.
- **Wetterbericht:** Er blendet als Vollbild über die Uhr und wieder zurück. Ist „nach jedem Wetterbericht“ eingestellt, kommt danach das nächste Zifferblatt. Bei eingeschaltetem Wetterbericht wählt der Zufall das Zifferblatt WEATHER nicht, sonst bliebe die Uhr darauf stehen.
- **Wetter-Ansichten:** OVERVIEW, SIMPLE, TODAY, DAYS (7 Tage), HOURS (24 Stunden als Kurve) oder ROTATING. Jede gibt es bunt oder einfarbig. Die Daten kommen von Open-Meteo: jetzt, stündlich für 24 Stunden, täglich für 7 Tage, mit Sonnenauf- und -untergang.
- **Datum und Wetter** stehen in einer Zeile unter jedem Zifferblatt außer WEATHER, jedes in seiner Farbe. Sie sind je nach Menü an oder aus, das Gradzeichen ist gezeichnet.
- **Musik, alles gleitend:** Bei lauter Musik leuchten Ringe und Marken etwas heller. Nichts blinkt.

## Geprüft am Automaten (2026-11-08)

- 0.3.0 und 0.3.1 installiert, 40 Bilder pro Sekunde, etwa 500 Linien, keine verworfenen Linien.
- Die Übernahme der Steuerung klappt. Der Screensaver läuft als `pi`, der zur Gruppe `input` gehört.
  - Gegriffen werden die Tastatur des Bedienfelds (Arduino Micro Keyboard), der Spinner (Ultimarc SpinTrak), die Arduino-Maus und das 2.4G-Funkgerät mit Tastatur, Maus und Consumer Control.
  - Seit 0.3.1 bleiben der Einschaltknopf (`pwr_button`), die System-Tasten des Funkgeräts, das USB-Audiogerät und HDMI unberührt. In 0.3.0 hatte die Uhr auch den Einschaltknopf gegriffen.
  - Das SDK sieht keinen Joystick, die Steuerung kommt als Tastatur. Die Tasten folgen den Standardtasten des SDK (START 1 = `KEY_1`, Button 1 = `KEY_LEFTCTRL`, Button 2 = `KEY_LEFTALT`), die Zuordnung der Uhr passt dazu.
  - Im Test drückte jemand Button 3 (`KEY_SPACE`): Die Uhr endete wie verlangt. Den Spinner zu drehen weckt nicht.
- `curl` und `wget` sind vorhanden. Wetter und Ortssuche von Open-Meteo antworten.

## Geprüft (am PC)

- 0.4.0: Menü mit F2/Tab per `SAVER_SCRIPT` (`MENU`) geöffnet und geschlossen, Einstellung dabei gespeichert. RANDOM für alle drei Farben: Jedes Zifferblatt kam in einer anderen Farbe, Datum und Wetter wechselten mit. Am Automaten 40 Bilder pro Sekunde, keine verworfenen Linien.

- 0.3.0:
  - Alle Wetter-Ansichten mit echten Daten für Berlin angesehen, bunt und einfarbig.
  - Den Wetterbericht im schnellen Takt (`SAVER_REPORT_EVERY`) über 60 s geprüft: zufällige Zifferblätter, nach jedem Bericht ein neues, die Ansichten im Wechsel.
  - Getrennte Farben für Uhr, Datum und Wetter.
  - Das blätternde Menü.
  - Dabei behoben: Mit „nach Wetterbericht“ wechselte das Zifferblatt ständig, und der Zufall konnte auf WEATHER stehen bleiben.

- 0.2.1: Kreise sind auf dem 4:3-Monitor rund. Die Linien werden quer um 0,75 zusammengezogen, Text behält seine Form. Alle Zifferblätter wurden danach angesehen.

- PC- und Pi-Build ohne Warnungen.
- Das Menü wurde per `SAVER_SCRIPT` durchgespielt:
  - Zifferblatt, Farbe und Sprache umgestellt
  - Köln aus der Liste gewählt: Wetter und Ortszeit auf Französisch, „SAM 7 NOV COLOGNE 7°C COUVERT“
  - „OSLO“ über die Bildschirmtastatur gesucht: 6 Treffer, Oslo in Norwegen gewählt
  - die englische Wortuhr („IT IS TEN PAST NINE“) in Bernstein geprüft
  - Speichern und Laden der Einstellungen
- Version 0.1.0 lief am PC mit 40 Bildern pro Sekunde und bis etwa 900 Linien.

## Offen

- **Am Automaten noch von Hand zu prüfen:**
  - ob F2 oder Tab das Menü öffnet und Joystick und Buttons darin wirken
  - ob das Menü des Automaten nach dem Wecken wieder erscheint und den Screensaver nicht sofort neu startet
  - Mit `CLOCK_INPUT_LOG=1` (und `VP_SAVER=1` beim Start von Hand) zeigt die Uhr jede Taste. Mit `CLOCK_NO_GRAB=1` lässt sich das Greifen abschalten.
- **Der Automat steht auf der Zeitzone America/New_York.** Mit „Ortszeit des Automaten“ geht die Uhr in Deutschland sechs Stunden nach. Abhilfe: im Menü unter PLACE eine Stadt wählen, oder die Zeitzone des Pi umstellen.
- Am echten Monitor noch nicht beurteilt.
