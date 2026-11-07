# Stand

Stand 2026-11-07, Version 0.3.0: **Clock** (`saver_clock`).

## Was er kann

- **Zehn Zifferblätter** (siehe README). Eines blendet in 1,4 s ins nächste über, der Name des neuen steht kurz unten.
- **Das eigene Menü** (START 1) hat 18 Zeilen und blättert. Einstellbar sind:
  - Zifferblatt und Zufall (aus, beim Start, immer)
  - Wechsel (aus, mit jedem Track, alle 1, 5, 15 oder 60 Minuten, nach jedem Wetterbericht)
  - die Farben von Uhr, Datum und Wetter, jede für sich
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

## Geprüft (am PC)

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

- **Noch nicht auf dem Automaten.** Er war am 2026-11-07 den ganzen Nachmittag per SSH nicht erreichbar (Ping ging).
- **Die Übernahme der Steuerung (`input.c`) ist ungetestet.** Am Automaten zu prüfen:
  - welche Eingabegeräte es gibt und welche Codes die Tasten senden, zum Beispiel mit `CLOCK_INPUT_LOG=1`
  - ob das Greifen klappt und mit welchen Rechten der Screensaver läuft
  - ob das Menü des Automaten nach dem Wecken wieder erscheint und den Screensaver nicht sofort neu startet
  - Die Zuordnung (START 1 = `KEY_1`, START 2 = `KEY_2`, Button 1 = `KEY_LEFTCTRL`, Button 2 = `KEY_LEFTALT` und die üblichen Joystick-Codes) ist eine Annahme nach den MAME-Standardtasten.
  - Mit `CLOCK_NO_GRAB=1` lässt sich das Greifen abschalten.
- Ob `curl` oder `wget` auf dem Pi vorhanden ist.
- Am echten Monitor noch nicht beurteilt.
