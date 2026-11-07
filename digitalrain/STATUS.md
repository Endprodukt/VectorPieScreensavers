# Status

Stand 2026-11-07, Version 0.2.0: **Digital Rain** (`saver_digitalrain`), ein Matrix-artiger Code-Regen.

## Was da ist (`src/main.c`)

- **Regen:** 22 Spalten mit 20 Zeilen. Pro Spalte fallen bis zu 2 Tropfen, jeder mit heller Spitze und einem Schweif, der nach hinten dunkler wird (6 bis 17 Zeichen lang).
- **Zeichen:** 16 eigene Linienzeichen im Stil von Halbbreiten-Katakana und Ziffern, etwa 30 % davon gespiegelt. Gelegentlich wechselt ein Zeichen still, ohne Aufblitzen.
- **Namen fallen mit dem Regen:** alle 3 bis 8 s ein neuer, bis zu 4 gleichzeitig in verschiedenen Spalten (nie direkt nebeneinander), der erste nach 4 s.
  - Ein Name ist ein Tropfen: Seine Buchstaben stehen von oben nach unten, der letzte ist die helle Spitze, darüber folgt ein Schweif aus 4 bis 10 auslaufenden Zeichen.
  - Er fällt Zeile für Zeile wie der Regen, im Tempo der langsameren Tropfen (5 bis 8 Zeilen pro Sekunde, gleitend schneller bei lauter Musik), damit man ihn lesen kann.
  - Etwa 30 % sind **VECTORPIE**, der Rest sind Spielnamen in Großbuchstaben, ohne Zusätze in Klammern. Namen mit mehr als 18 Zeichen werden ausgelassen, weil sie nicht ins Bild passen würden.
  - Derselbe Name fällt nie zweimal gleichzeitig.
- **Spieleliste**, beim Start auf dem Automaten gelesen:
  - `/usr/local/share/advance/gamelist.ini` und `gamelist.additions` (Zeilen `Hersteller|Name|...`, auskommentierte mit `#` ausgenommen)
  - die `name =` aller `/persistent/games/*/game.ini`

  Doppelte Namen werden entfernt. Neue Spiele erscheinen damit automatisch.
- **Musik:** Der Regen fällt bei lauter Musik gleitend schneller. Nichts pulsiert oder blinkt.
- Für den PC: `SAVER_GAMELIST=<datei>` liest eine Kopie der Spieleliste, `SAVER_SHOTS=<ordner>` speichert alle 2 s ein Bild. Ohne Spieleliste erscheint nur VECTORPIE.

## Geprüft (2026-11-07)

- PC- und Pi-Build ohne Warnungen im eigenen Code. `build/saver_digitalrain-0.2.0.vps` wird erzeugt.
- 0.2.0 am PC mit einer Kopie der Spieleliste des Automaten (43 Namen bis 18 Zeichen): mehrere Namen gleichzeitig im Regen gesehen (VECTORPIE, GEOMETRY WARS …). Etwa 400 bis 640 Linien pro Bild, 40 fps.
- 0.2.0 auf dem Automaten installiert (`sh tools/deploy.sh digitalrain`). Das Board habe ich mit 0.1.0 getestet: 65 Namen, gleichmäßig 40 Bilder pro Sekunde, etwa 440 bis 630 Linien, keine verlorenen Linien. 0.2.0 hat eine ähnliche Linienzahl.

## Bekannte Probleme und Grenzen

- Helligkeit und Lesbarkeit der Namen am echten Monitor sind noch nicht beurteilt.
- Unbekannte Zeichen in Namen zeichnet die Asteroids-Schrift als Leerzeichen.

## Offene Aufgaben

- [ ] Am Automaten ansehen: Lesbarkeit der Namen, Tempo des Regens, Häufigkeit der Namen
- [ ] Unter SETTINGS → SCREENSAVERS → SELECT auswählen (das geht nur am Automaten)
