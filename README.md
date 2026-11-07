# VectorPie Screensavers

Screensaver-Apps für den **VectorPie**-Vektor-Automaten. Der Automat startet eine solche App statt seines eigenen Screensavers. Sie zeichnet auf den Vektor-Monitor, während das Menü seine Musik weiterspielt.

Jeder Screensaver hat einen eigenen Ordner:

| Ordner | Name | Was |
|---|---|---|
| [`neondrive/`](neondrive/) | **Neon Drive** | Eine endlose Fahrt im 80er-Synthwave-Stil in 3D: Grid, Sonne, wechselnde Strecken und Kulissen, Verkehr in beide Richtungen, Vögel und ein Flugzeug mit VECTORPIE-Banner. |
| [`digitalrain/`](digitalrain/) | **Digital Rain** | Grüner Code-Regen im Matrix-Stil. Ab und zu gleitet, fällt oder rastet darin der Name eines Spiels vom Automaten ein, oder VECTORPIE. |

## Gemeinsam

- `vendor/VectorPie` – das VectorPie-SDK (Kopie), Doku unter `vendor/VectorPie/sdk/docs/`, für Screensaver besonders `09-screensavers.md`
- `cmake/` – die Werkzeugkette für den Raspberry Pi des Automaten
- `tools/` – Paket bauen (`make_vps.sh`), auf den Automaten bringen (`deploy.sh`), am PC starten (`run_pc.bat`), Test-Musik erzeugen (`make_test_track.py`)
- `BUILD.md` – bauen, testen, installieren, einen neuen Screensaver anlegen

## Installieren

Pi-Build bauen (siehe `BUILD.md`), dann `sh tools/make_vps.sh <ordner>`. Die `.vps` per USB in SETTINGS → SCREENSAVERS → INSTALL installieren und unter SELECT auswählen. Oder direkt per SSH: `sh tools/deploy.sh <ordner>`.
