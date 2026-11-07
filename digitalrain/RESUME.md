# Weitermachen

**Digital Rain** ist der zweite Screensaver im Repository „VectorPie Screensavers“ (Ordner `digitalrain/`): grüner Code-Regen mit Spielnamen. Bauen, Paketieren und Installieren: siehe `../BUILD.md`. Stand: siehe `STATUS.md`.

## Wünsche des Nutzers

- Matrix-Stil, wie man ihn kennt. Der Gag: Namen aus der Spieleliste und VECTORPIE.
- Die Namen **laufen mit dem Regen** (fallen als Tropfen, nicht quer durchs Bild). Sie sollen gern öfter kommen, auch mehrere gleichzeitig (Rückmeldung 2026-11-07). Gleiten und Einrasten aus 0.1.0 sind deshalb entfernt.
- Wie bei allen Screensavern hier: nichts pulsiert oder blinkt (siehe `../neondrive/RESUME.md`).

## Aufbau (alles in `src/main.c`)

- `GLYPHS`: die Zeichen als Striche in einer Zelle 0..1. Bit 7 im Zellwert bedeutet gespiegelt.
- `C[COLS]`: pro Spalte die Tropfen (`head` in Zeilen von oben, `speed`, `len`) und die Zeichen jeder Zeile.
- `ND[NAME_DROPS]`: die fallenden Namen (Spalte, Buchstaben, Schweif, `head`, `speed`). `start_name` sucht eine freie Spalte ohne Namen daneben, `names_update` bewegt sie im Tempo des Regens und startet alle 3 bis 8 s einen neuen. In einer Spalte mit Namen entstehen keine neuen Regentropfen, und die Regenzellen unter dem Namen bleiben frei (`name_cell`).
- Spieleliste: `read_gamelist` (Format `Hersteller|Name|...`) und `read_games` (`/persistent/games/*/game.ini`). `add_name` macht Großbuchstaben, entfernt `(...)` und Doppelte und lässt Namen über `NAME_FIT` (18) Zeichen aus.
- Linienbudget: etwa 550 Linien pro Bild, 40 fps auf dem Board. Es gibt also Luft für mehr Spalten oder längere Schweife.
- Eine Kopie der Spieleliste für den PC holt man vom Automaten:

  ```bash
  scp -i ~/.ssh/id_ed25519_vectorpie pi@192.168.0.122:/usr/local/share/advance/gamelist.ini <datei>
  ```

  Dann `SAVER_GAMELIST=<datei>` setzen.
