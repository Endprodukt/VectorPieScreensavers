# Weitermachen

**Digital Rain** ist der zweite Screensaver im Repository „VectorPie Screensavers“ (Ordner `digitalrain/`): grüner Code-Regen mit Spielnamen. Bauen, Paketieren und Installieren: siehe `../BUILD.md`. Stand: siehe `STATUS.md`.

## Wünsche des Nutzers

- Matrix-Stil, wie man ihn kennt. Der Gag: Ab und zu fahren Namen aus der Spieleliste und VECTORPIE durchs Bild, aber **nicht ständig**.
- Wie bei allen Screensavern hier: nichts pulsiert oder blinkt (siehe `../neondrive/RESUME.md`).

## Aufbau (alles in `src/main.c`)

- `GLYPHS`: die Zeichen als Striche in einer Zelle 0..1. Bit 7 im Zellwert bedeutet gespiegelt.
- `C[COLS]`: pro Spalte die Tropfen (`head` in Zeilen von oben, `speed`, `len`) und die Zeichen jeder Zeile.
- `N`: der aktuelle Namen-Auftritt (`SLIDE`, `FALL`, `DECODE`). `start_name` wählt Art und Platz, `names_update` beendet ihn, danach `N.next` = 20 bis 40 s Pause.
- Spieleliste: `read_gamelist` (Format `Hersteller|Name|...`) und `read_games` (`/persistent/games/*/game.ini`), `add_name` macht Großbuchstaben, entfernt `(...)` und Doppelte.
- Linienbudget: etwa 550 Linien pro Bild, 40 fps auf dem Board. Es gibt also Luft für mehr Spalten oder längere Schweife.
- Eine Kopie der Spieleliste für den PC holt man vom Automaten:

  ```bash
  scp -i ~/.ssh/id_ed25519_vectorpie pi@192.168.0.122:/usr/local/share/advance/gamelist.ini <datei>
  ```

  Dann `SAVER_GAMELIST=<datei>` setzen.
