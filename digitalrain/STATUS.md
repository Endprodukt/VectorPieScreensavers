# Status

Stand 2026-11-07, Version 0.1.0: **Digital Rain** (`saver_digitalrain`), ein Matrix-artiger Code-Regen.

## Was da ist (`src/main.c`)

- **Regen:** 22 Spalten mit 20 Zeilen. Pro Spalte fallen bis zu 2 Tropfen, jeder mit heller Spitze und einem Schweif, der nach hinten dunkler wird (6 bis 17 Zeichen lang).
- **Zeichen:** 16 eigene Linienzeichen im Stil von Halbbreiten-Katakana und Ziffern, etwa 30 % davon gespiegelt. Gelegentlich wechselt ein Zeichen still, ohne Aufblitzen.
- **Namen:** alle 20 bis 40 s einer, der erste nach 8 s. Etwa 30 % der Auftritte sind **VECTORPIE**, der Rest sind Spielnamen in Großbuchstaben, ohne Zusätze in Klammern. Drei Arten, zufällig gewählt:
  - **Gleiten:** Der Name fährt groß von rechts nach links durchs Bild, der Regen macht ihm Platz.
  - **Fallen:** Er fällt als Spalte seiner Buchstaben herunter.
  - **Einrasten:** In einer Zeile rasten die Buchstaben nacheinander ein, bleiben etwa 5 s stehen und zerfallen wieder.
- **Spieleliste**, beim Start auf dem Automaten gelesen:
  - `/usr/local/share/advance/gamelist.ini` und `gamelist.additions` (Zeilen `Hersteller|Name|...`, auskommentierte mit `#` ausgenommen)
  - die `name =` aller `/persistent/games/*/game.ini`

  Doppelte Namen werden entfernt. Neue Spiele erscheinen damit automatisch.
- **Musik:** Der Regen fällt bei lauter Musik gleitend schneller. Nichts pulsiert oder blinkt.
- Für den PC: `SAVER_GAMELIST=<datei>` liest eine Kopie der Spieleliste, `SAVER_SHOTS=<ordner>` speichert alle 2 s ein Bild. Ohne Spieleliste erscheint nur VECTORPIE.

## Geprüft (2026-11-07)

- PC- und Pi-Build ohne Warnungen im eigenen Code. `build/saver_digitalrain-0.1.0.vps` wird erzeugt.
- Am PC mit einer Kopie der Spieleliste des Automaten (44 Namen): Bilder angesehen, Regen, Einrasten (VECTORPIE) und Fallen (OMEGA RACE) gesehen. Etwa 550 Linien pro Bild.
- **Auf dem Automaten installiert** (`sh tools/deploy.sh digitalrain`) und 25 s auf dem Board getestet: 65 Namen gefunden, gleichmäßig 40 Bilder pro Sekunde, etwa 440 bis 630 Linien, keine verlorenen Linien.

## Bekannte Probleme und Grenzen

- Das Gleiten habe ich in den Testbildern nicht erwischt, es ist nur im Code geprüft.
- Helligkeit und Lesbarkeit der Namen am echten Monitor sind noch nicht beurteilt.
- Unbekannte Zeichen in Namen zeichnet die Asteroids-Schrift als Leerzeichen.

## Offene Aufgaben

- [ ] Am Automaten ansehen: Lesbarkeit der Namen, Tempo des Regens, Häufigkeit der Namen
- [ ] Unter SETTINGS → SCREENSAVERS → SELECT auswählen (das geht nur am Automaten)
