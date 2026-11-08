# Weitermachen

**Clock** ist der siebte Screensaver im Repository „VectorPie Screensavers“ (Ordner `clock/`). Bauen, Paketieren und Installieren: siehe `../BUILD.md`. Stand: siehe `STATUS.md`.

## Wünsche des Nutzers (2026-11-07)

- Eine Uhr mit den verschiedensten Zifferblättern: digital, analog, Klappzahlen, binär und was sonst einfällt.
- Je ein Zifferblatt im Stil von Tempest und von Asteroids. Umgesetzt als TUNNEL und ROCKS, mit eigenen Formen und ohne Namen oder Grafiken der Spiele.
- Datum, Wetter „und solche Geschichten“, weil der Automat online ist.
- **Ein Menü im laufenden Screensaver.** Der Nutzer hat Weg 1 gewählt: Die Uhr übernimmt die Steuerung selbst.
- Im Menü einstellbar:
  - Stadt und Sprache: Englisch, Deutsch, Französisch, Spanisch. **Standard ist Englisch.**
  - Farben
  - der Ort für Zeit und Wetter
  - das Zifferblatt selbst zusammenstellen: mit oder ohne Datum, Wetter usw.
- **Wechsel mit dem Track ist nur eine Option, nicht der Standard.** Standard ist: kein Wechsel.
- Unterschiedliche Farben für Uhr, Datum und Wetter (2026-11-07).
- Der Wetterbericht als Vollbild in einem einstellbaren Takt, auch kurz: nach 1, 2 Minuten usw. (2026-11-08).
- Das Zifferblatt kann zufällig sein: beim Start, nach einem Song, nach einer Zeit oder nach dem Wetterbericht.
- Den Wetterbericht gibt es in mehreren Ansichten: einfach, nur heute, mehrere Tage, über den Tag, einfarbig oder bunt.
- Nichts darf blinken oder pulsieren. Musik ändert Dinge nur gleitend.

## Wie die Uhr die Steuerung übernimmt (`src/input.c`)

Laut SDK (`vendor/VectorPie/sdk/docs/09-screensavers.md`) liest das Menü des Automaten die Steuerung selbst. Bei jeder Taste außer NEXT TRACK beendet es den Screensaver.

Die Uhr öffnet deshalb, wenn sie als Screensaver läuft (`vp_saver_active()`), die `/dev/input/event*` der Steuerung und greift sie mit `EVIOCGRAB`. Steuerung heißt: Tastatur (`KEY_1` oder `KEY_LEFTCTRL`), Maus und Spinner (`BTN_LEFT`), Joystick (`BTN_TRIGGER`, `BTN_SOUTH`) oder Achsen. Einschaltknopf, Lautstärketasten und HDMI (`vc4-hdmi`) bleiben dem System. Dann sieht das Menü des Automaten nichts mehr, und die Uhr macht selbst, was das Menü sonst tut:
- **F2 oder Tab** (`KEY_F2`, `KEY_TAB`, im Code `IN_MENU`) öffnet das Uhr-Menü und schließt es wieder. START 1 weckt wie jede andere Taste (Wunsch 2026-11-08: START 1 taugt nicht fürs Menü).
- **START 2** ruft `vp_saver_skip()` auf (NEXT TRACK).
- **Der Spinner** (ein Gerät mit „spin“ im Namen, `REL_X` oder `REL_DIAL`, 60 Zählschritte je Schritt) schaltet durch die Zifferblätter. Nach dem letzten kommt der Wetterbericht als Halt (`spin()`, `G.report_spun`). Im Menü kommt er als hoch und runter (Wunsch 2026-11-08).
- **Jede andere Taste** gibt die Geräte frei (`input_release`), damit das Menü des Automaten wenigstens das Loslassen sieht, und beendet die Uhr (`vp_game_quit`).
- **Im Uhr-Menü** gehören alle Tasten dem Menü. Analoge Sticks zählen erst ab drei Vierteln Ausschlag, außerhalb des Menüs gar nicht (Joystick-Drift).

Noch am Automaten zu prüfen, siehe STATUS „Offen“. Hilfen dabei:
- `CLOCK_INPUT_LOG=1` schreibt Geräte und Ereignisse nach stderr. Von Hand gestartet braucht es dazu `VP_SAVER=1`, sonst greift die Uhr nichts.
- `CLOCK_NO_GRAB=1` schaltet das Greifen ab.

## Aufbau

- `src/main.c`:
  - Alle Linien laufen über `mv` und `ln` (Makros für `vp_moveto` und `vp_lineto`) und werden dabei quer zusammengezogen. Textpositionen ziehen `SX()` zusammen, `SQUEEZE = 0` schaltet das aus.
  - die Zifferblätter (`face_*`), das Überblenden, `info_line`, die Wortuhr (`WC_*`, `words_target` je Sprache)
  - die Steuerung: `control` reicht an das Menü weiter oder öffnet es, `wake` beendet die Uhr
  - `follow_settings`: Was das Menü in `SET` ändert, wird übernommen, etwa ein neuer Ort für das Wetter oder neue Wörter für die Wortuhr.
  - Die Farbe kommt aus `color()`: Bei einem Farbthema (`PAINT`) wird jede Farbe durch die des Themas ersetzt, mit der Helligkeit der ursprünglichen. `PAINT` steht beim Zeichnen auf der Uhrfarbe, für Datum und Wetter auf deren Farbe.
  - Das Wetter: `weather_view(style)` mit `wx_overview`, `wx_simple`, `wx_today`, `wx_days`, `wx_hours`. Dazu `text_deg`: Text mit `^` an der Stelle eines Gradzeichens, das gezeichnet wird.
  - Der Wetterbericht: `report_update`. Den nächsten Wechsel bestimmt `next_face`, dort steckt auch der Zufall.
- `src/menu.c`: Menü, Ortsliste, Bildschirmtastatur und Suchtreffer. Es ändert `SET` sofort.
- `src/settings.c`: `SET`. Die Vorgaben stehen in `clock.ini` neben dem Programm, gespeichert wird in `save/clock.ini`.
- `src/weather.c`: ein Thread, der über `curl` oder `wget` holt:
  - das Wetter und den UTC-Abstand des Orts (`weather_set_place`)
  - die Ortssuche (`weather_search`)
- `src/lang.h`: alle Wörter in den vier Sprachen. `src/cities.h`: die Städteliste.
- Zum Testen am PC:
  - `SAVER_FACE=<0..9>` und `SAVER_CYCLE=<s>`
  - `SAVER_SCRIPT="<s>:<Taste>,..."`: drückt zu diesen Zeiten Tasten (U D L R OK BACK MENU S1 S2)
  - `SAVER_REPORT_EVERY=<s>`: der Wetterbericht so oft statt im eingestellten Takt
  - `SAVER_SHOTS=<Ordner>`

## Rund auf der Röhre

Das Board hat ein quadratisches Koordinatenfeld, der Monitor zieht es aber auf 4:3 (bei senkrechtem Monitor 3:4). Was in 2D rund gezeichnet wird, erscheint deshalb um ein Drittel zu breit. Die 3D-Screensaver gleichen das über `view.aspect` aus. In 2D gilt: quer zur Mitte mit 0,75 zusammenziehen (`ASPECT_X`; bei `VP_VERTICAL=1` stattdessen die Höhe). Aufgefallen ist das dem Nutzer an der Uhr (2026-11-07).
