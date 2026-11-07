# VectorPie Screensavers

Screensaver-Apps für den **VectorPie**-Vektor-Automaten. Der Automat startet eine solche App statt seines eigenen Screensavers. Sie zeichnet auf den Vektor-Monitor, während das Menü seine Musik weiterspielt.

Jeder Screensaver hat einen eigenen Ordner:

| Ordner | Name | Was |
|---|---|---|
| [`neondrive/`](neondrive/) | **Neon Drive** | Eine endlose Fahrt im 80er-Synthwave-Stil in 3D: Grid, Sonne, wechselnde Strecken und Kulissen, Verkehr in beide Richtungen, Vögel und ein Flugzeug mit VECTORPIE-Banner. |
| [`digitalrain/`](digitalrain/) | **Digital Rain** | Grüner Code-Regen im Matrix-Stil. Mit dem Regen fallen immer wieder Namen von Spielen des Automaten herunter, mehrere gleichzeitig, und VECTORPIE. |
| [`labyrinth/`](labyrinth/) | **Labyrinth** | Ein Gang durch ein Labyrinth nach dem anderen, aus Sicht des Wanderers: Wände mit Mustern und VECTORPIE-Schildern, drehende Kristalle und als Ausgang ein Kuchen. |
| [`neonpeaks/`](neonpeaks/) | **Neon Peaks** | Retrowave: ein Flug durch ein Neon-Gittertal auf eine gestreifte Sonne zu. Die Berge am Horizont formt die Musik (ihr Spektrum), Tempo und Farben folgen ihr gleitend. |
| [`neonridge/`](neonridge/) | **Neon Ridge** | Wie die Retrowave-Mountains-Videos: Sternenhimmel, cyan leuchtender Horizont, ein Bergmassiv aus Höhenlinien, davor eine Ebene aus Linien und eine pinke Lichtspur. Die Kamera steht still. |
| [`constellations/`](constellations/) | **Constellations** | Ein Rundflug über den echten Nachthimmel: 26 Sternbilder zeichnen sich nacheinander ein, mit deutschen und lateinischen Namen und benannten hellen Sternen. Die Sterne funkeln, die Milchstraße zieht durch, ab und zu fällt eine Sternschnuppe. |
| [`clock/`](clock/) | **Clock** | Die Uhrzeit auf zehn Zifferblättern (analog, digital, Klappzahlen, binär, Wortuhr, Nixie, Orbit, Tunnel, Rocks, Wetter), mit Datum und Wetter aus dem Netz. NEXT TRACK schaltet das Zifferblatt um. |

## Gemeinsam

- `vendor/VectorPie` – das VectorPie-SDK (Kopie), Doku unter `vendor/VectorPie/sdk/docs/`, für Screensaver besonders `09-screensavers.md`
- `cmake/` – die Werkzeugkette für den Raspberry Pi des Automaten
- `tools/` – Paket bauen (`make_vps.sh`), auf den Automaten bringen (`deploy.sh`), am PC starten (`run_pc.bat`), Test-Musik erzeugen (`make_test_track.py`)
- `BUILD.md` – bauen, testen, installieren, einen neuen Screensaver anlegen

## Installieren

Pi-Build bauen (siehe `BUILD.md`), dann `sh tools/make_vps.sh <ordner>`. Die `.vps` per USB in SETTINGS → SCREENSAVERS → INSTALL installieren und unter SELECT auswählen. Oder direkt per SSH: `sh tools/deploy.sh <ordner>`.
