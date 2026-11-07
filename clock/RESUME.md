# Weitermachen

**Clock** ist der siebte Screensaver im Repository „VectorPie Screensavers“ (Ordner `clock/`). Bauen, Paketieren und Installieren: siehe `../BUILD.md`. Stand: siehe `STATUS.md`.

## Wünsche des Nutzers (2026-11-07)

- Eine Uhr mit den verschiedensten Zifferblättern: digital, analog, Klappzahlen, binär und was sonst einfällt.
- Je ein Zifferblatt im Stil von Tempest und von Asteroids. Umgesetzt als TUNNEL und ROCKS, mit eigenen Formen und ohne Namen oder Grafiken der Spiele.
- Datum, Wetter „und solche Geschichten“, weil der Automat online ist.
- **Ein Menü im Screensaver**, um zum Beispiel die Zifferblätter umzuschalten, ohne ihn zu beenden. Das ist noch offen, siehe unten.
- Nichts darf blinken oder pulsieren. Musik ändert Dinge nur gleitend.

## Das Menü-Problem

Laut SDK (`vendor/VectorPie/sdk/docs/09-screensavers.md`) liest das Menü des Automaten die Steuerung selbst:
- Bei jeder Taste beendet es den Screensaver (SIGTERM).
- Nur NEXT TRACK lässt ihn laufen und spielt den nächsten Track.
- Der Screensaver bekommt keine eigenen Eingaben.

Deshalb schaltet jetzt jeder neue Track das Zifferblatt weiter. Wege zu einem echten Menü, beim Nutzer angefragt:
1. **Der Screensaver übernimmt die Steuerung selbst:** Er öffnet `/dev/input/event*` mit `EVIOCGRAB`, dann sieht das Menü des Automaten nichts mehr. Er macht dann selbst, was das Menü sonst tut: Eine Menütaste öffnet sein Menü, jede andere Taste beendet ihn, NEXT TRACK ruft `vp_saver_skip()` auf. Vorher am Automaten zu prüfen: welche Eingabegeräte es gibt und mit welchen Rechten der Screensaver läuft.
2. **Eine eigene Einstellungs-App** in der Spieleliste, die `clock.ini` schreibt.
3. **Eine Erweiterung des Automaten-Menüs** durch VectorPie: Tasten an Screensaver durchreichen.

## Aufbau

- `src/main.c`: die Zifferblätter (`face_*`), Überblenden (`G.from`, `G.face`, `G.blend`), `info_line`.
  - Eigene Ziffern (`GLYPH`, `glyph_part`), weil sie sich stauchen und schneiden lassen müssen: die Klappkarten und die Temperatur.
  - Die Klappkarten (`flip_card`): Erst klappt die obere Hälfte der alten Ziffer nach unten, dann die untere Hälfte der neuen. Was hinter einer Klappe liegt, wird nur außerhalb von ihr gezeichnet.
- `src/weather.c`: Abruf in einem Thread, Ort aus `clock.ini`, ein einfacher JSON-Leser für die Antworten von Open-Meteo.
- `package/clock.ini`: der Ort. `tools/make_vps.sh` und `tools/deploy.sh` nehmen seit dieser Uhr alle Dateien aus `package/` mit.
- Zum Testen: `SAVER_FACE=<0..9>` (Start-Zifferblatt), `SAVER_CYCLE=<s>` (automatisch umschalten), `SAVER_SHOTS=<Ordner>`.
