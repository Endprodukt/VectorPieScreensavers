# Bauen

Alle Screensaver werden gemeinsam gebaut. Aufbau und Werkzeuge sind wie bei Vector Racer (`F:\Vector Racer`). Das SDK liegt als Kopie in `vendor/VectorPie`. Es stammt aus `F:\Release Game\VectorPie` (SDK 1.1.13, Stand 2026-11-03) und ist die erste Fassung mit Screensaver-Apps (`vp_saver.h`). Wie Screensaver funktionieren, steht in `vendor/VectorPie/sdk/docs/09-screensavers.md`.

Jeder Screensaver landet in `build/<pc|pi>/<ordner>/`, z. B. `build/pc/neondrive/saver_neondrive.exe`.

## PC (Windows, nativ)

Voraussetzung: MSYS2 mit MinGW-w64-GCC und SDL2 unter `C:\msys64\mingw64`, dazu CMake und Ninja.

```bash
export PATH="/c/msys64/mingw64/bin:$PATH"
cmake -S . -B build/pc -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_PREFIX_PATH=C:/msys64/mingw64
cmake --build build/pc
```

Zum Testen braucht ein Screensaver Musik. Ohne das Menü des Automaten spielt das SDK selbst Musik ab und analysiert sie. Die Musik wird als `.ogg`- oder `.wav`-Datei oder als Ordner übergeben, entweder auf der Kommandozeile oder in `VP_SAVER_MUSIC`. Einen Test-Track mit Beat-Map erzeugt einmalig:

```bash
python tools/make_test_track.py        # music/test_120bpm.wav + .beats
```

Am einfachsten startest du einen Screensaver mit **`<ordner>/run_pc.bat` per Doppelklick**. Das nimmt die Musik aus `music/`. Esc beendet, START 2 springt zum nächsten Track.

`VP_STATS=1` gibt einmal pro Sekunde die Linien pro Bild und die Zeiten aus.

## Raspberry Pi (arm64)

Wie bei Vector Racer: clang und lld auf dem PC, gegen die Kopie der Bibliotheken des Automaten (`F:\Tempest 2000 Port\build\pi-sysroot`).

```bash
export PATH="/c/Users/ocare/.local/share/retcomm/toolchains/cmake-clang-v1/latest/bin:$PATH"
cmake -S . -B build/pi -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-pi-clang.cmake -DCMAKE_BUILD_TYPE=Release "-DVPS_PI_SYSROOT=F:/Tempest 2000 Port/build/pi-sysroot"
cmake --build build/pi
```

## Paket und Installation auf dem Automaten

```bash
sh tools/make_vps.sh neondrive      # build/<id>-<version>.vps, zum Installieren per USB
sh tools/deploy.sh neondrive        # direkt per SSH nach /persistent/screensavers/<id>
```

Weil `package/game.ini` `kind = screensaver` enthält, wird daraus eine `.vps` und keine `.vpk`. Auf dem Automaten: SETTINGS → SCREENSAVERS → INSTALL (USB), dann unter SETTINGS → SCREENSAVERS → SELECT auswählen.

`deploy.sh` nutzt denselben Schlüssel und Automaten wie Vector Racer (`~/.ssh/id_ed25519_vectorpie`, `pi@192.168.0.122`). Es ersetzt das Programm auch dann, wenn der Screensaver gerade läuft.

Die Version eines Screensavers steht an zwei Stellen und muss an beiden geändert werden: in `<ordner>/package/game.ini` (`version`) und in `<ordner>/CMakeLists.txt` (`project(... VERSION ...)`).

## Einen neuen Screensaver anlegen

1. Einen Ordner anlegen mit `CMakeLists.txt` (wie `neondrive/CMakeLists.txt`), `src/`, `package/game.ini` (mit `kind = screensaver`, eigener `id` mit Präfix `saver_`) und `run_pc.bat`.
2. In der obersten `CMakeLists.txt` `add_subdirectory(<ordner>)` ergänzen.
3. In `README.md` in die Liste eintragen.
