# Weitermachen

**Neon Drive** ist ein VectorPie-Screensaver: eine endlose Synthwave-Fahrt in 3D zur Musik des Automaten. Er ist der erste Screensaver im Repository „VectorPie Screensavers“ (Ordner `neondrive/`). Bauen, Paketieren und Installieren: siehe `../BUILD.md`. Aktueller Stand und offene Punkte: siehe `STATUS.md`.

## Wünsche des Nutzers (dauerhaft gültig)

- **Nichts pulsiert, blitzt oder blinkt.** Keine Reaktion auf Beats, auch nicht von der Sonne. Die Musik darf nur gleitend Tempo und Verkehrsdichte steuern.
- Keine Sterne neben der Sonne.
- Alles am Straßenrand steht fest an der Strecke: nichts springt, nichts fährt mit dem Auto mit.
- Kulissen am Horizont wechseln langsam (Minuten).
- VECTORPIE soll vorkommen: auf LKW, Schilderbrücken, Straßenschildern, Werbetafeln und dem Flugzeug-Banner.

## Aufbau

| Datei | Inhalt |
|---|---|
| `src/saver.h` | gemeinsamer Zustand `G` (Weg, Tempo, Kurs, Kamera, Musik), Maße der Straße, Prototypen |
| `src/main.c` | Musik-Feed, Tempo, Titel, die Szene pro Bild, `SAVER_SHOTS`; mit `SAVER_SOAK` der Dauertest |
| `src/road.c` | Kurven, Straßentabelle (`road_point`, `road_heading`), Abschnitte (`zone_kind`), Gegenfahrbahn (`road_oncoming`), Grid und Fahrbahnlinien |
| `src/models.c` | alle Meshes aus Quadern und Einzellinien: Auto, 7 Fahrzeugtypen, Randobjekte; die VECTORPIE-Linienschrift (`vectorpie_strokes`, `word`) |
| `src/props.c` | Randobjekte je Abschnittsart |
| `src/traffic.c` | Verkehr in beide Richtungen, Ausweich-Logik, Tempobegrenzung hinter Fahrzeugen |
| `src/flyers.c` | Vogelschwärme, Flugzeug mit VECTORPIE-Banner |
| `src/car.c` | das eigene Auto (Neigen beim Spurwechsel) und die Kamera (Positionen, Gleiten) |
| `src/scenery.c` | Kulissen-Ring, Auftauchen und Versinken, Sonne, Horizontlinie |

## Grundlagen

- In jedem Bild steht das Auto im Ursprung, die Straße zeigt dort nach +z. `road_build()` legt die Straße ab 40 m hinter bis 420 m vor dem Auto aus. `road_point(d, x, y)` liefert den Punkt d Meter voraus, x quer, y hoch.
- Alles läuft durch eine einzige `vp3d_scene`. Feste Meshes und `vp3d_scene_polygon` verdecken, Linien und Drahtmodelle verdecken nichts. Mit `vp3d_scene_set_contact(0.3)` bleiben Linien auf dem Boden sichtbar. Ein unsichtbares Bodenpolygon verdeckt alles unter y=0.
- Kulissen sind flache Polygone auf einem Ring mit R=3500 m. Ihre Details liegen 3 m davor, damit das eigene Polygon sie nicht verdeckt.
- **Randobjekte hängen an der Strecke.** Positionen, Seiten und Abwechslung leiten sich aus dem Abstand ab Abschnittsbeginn ab, nie aus einem Zähler über die gerade sichtbaren Objekte. Ein solcher Zähler hat die Palmen springen lassen. Pfosten und Wiederholungen beginnen am Abschnittsanfang, nicht am Auto. So hatten die Leitplanken-Pfosten mitgefahren.
- **Keine Würfel-Schleifen** wie `do x = zufall(); while (x == alt);` mit einem Würfelwert, der sich in der Schleife nicht ändert. Das hat das Programm aufgehängt (Kamerawechsel). Stattdessen: `(alt + 1 + zufall * (n - 1)) % n`.
- Linienbudget: Bei etwa 2000 Linien pro Bild wird es auf dem Board knapp (Erfahrung aus Vector Racer). Messen mit `VP_STATS=1` oder dem Dauertest.
- Zum Ansehen am PC: `neondrive/run_pc.bat` per Doppelklick. Aus der Claude-Umgebung gestartete Programme werden dort manchmal von außen beendet. Bilder: `SAVER_SHOTS=<ordner>`, gespeichert wird am Ende von `draw()`.
- Dauertest: `build/pc/neondrive/saver_soak.exe <stunden> <seed>` simuliert ohne Musik etwa 25 Minuten Fahrt pro Sekunde, gibt die schwersten Bilder aus und findet Hänger (dann läuft er nicht zu Ende).
- **Nie durch andere Fahrzeuge:** `traffic_limit()` begrenzt den Weg des Autos (Front nie über das Heck eines Fahrzeugs in seinem Weg). `steer()` lässt das Auto nicht seitlich in ein Fahrzeug daneben ziehen. Fahrzeuge hinter dem Auto übernehmen sein Tempo. Wer daran etwas ändert, prüft mit dem Dauertest und einer vorübergehenden Überlappungsprüfung in `traffic_update` (zuletzt 0 in 25 simulierten Stunden) und entfernt die Prüfung danach wieder.
- Räder: `wheel()` in `models.c` merkt sich je Fahrzeugtyp die Radpositionen. `model_spokes()` zeichnet pro Rad drei Speichen, gedreht um Weg mal `WHEEL_TURN`. Das ist bewusst langsamer als echt, sonst gäbe es den Speichenrad-Effekt durch die Bildrate.
- Kamera: `SHOTS` in `car.c`. Das Feld `swing` lässt eine Einstellung um die Wagenmitte nach rechts herum und zurück schwingen (sin²-Verlauf, `SWING_TIME`). Vorher holt `traffic_hold_lane(RIGHT_LANE)` das Auto auf die rechte Spur, und der Schwenk startet erst bei `traffic_room` größer als `SWING_ROOM`.
- **Die Kamera darf nie in einem Fahrzeug stecken.** Dafür sorgen drei Dinge: Seitliche Einstellungen bleiben neben der Fahrbahn, beim Gleiten über die Spuren steigt die Kamera auf `OVER_TRAFFIC`, und Fahrzeuge hinter dem Auto halten `BEHIND` Abstand. Geprüft wird mit einer vorübergehenden Punkt-in-Fahrzeug-Prüfung im Dauertest (zuletzt 0 Fälle) und mit der gefahrenen Strecke (kein Tempoverlust).
- Der Dauertest `saver_soak` versteckt sein Fenster und beantwortet die Fensternachrichten. Sonst zeigt Windows es als „Keine Rückmeldung“ an, was wie ein Absturz aussieht.

## SDK

- Die Doku liegt in `vendor/VectorPie/sdk/docs/`: `05-3d.md` (3D, Szenen) und `09-screensavers.md`. Die API steht in `vp_3d.h`, `vp_saver.h` und `vp_music_feed.h`.
- Ein Screensaver ist ein normales `vp_run`-Spiel mit `kind = screensaver` im `game.ini`. Den Einbrennschutz übernimmt das SDK.
- Das SDK neu übernehmen: `F:\Release Game\VectorPie` (oder neuer) nach `vendor/VectorPie` kopieren, ohne `.git`.
