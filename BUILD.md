# Building

All screensavers are built together. The layout and tools are the same as for Vector Racer (`F:\Vector Racer`). The SDK is a copy in `vendor/VectorPie`. It comes from `F:\Release Game\VectorPie` (SDK 1.1.13, as of 2026-11-03) and is the first version with screensaver apps (`vp_saver.h`). How screensavers work is described in `vendor/VectorPie/sdk/docs/09-screensavers.md`.

Each screensaver ends up in `build/<pc|pi>/<folder>/`, e.g. `build/pc/neondrive/saver_neondrive.exe`.

## PC (Windows, native)

Requirements: MSYS2 with MinGW-w64 GCC and SDL2 in `C:\msys64\mingw64`, plus CMake and Ninja.

```bash
export PATH="/c/msys64/mingw64/bin:$PATH"
cmake -S . -B build/pc -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_PREFIX_PATH=C:/msys64/mingw64
cmake --build build/pc
```

For testing, a screensaver needs music. Without the cabinet's menu the SDK plays and analyses the music itself. The music is given as an `.ogg` or `.wav` file or as a folder, either on the command line or in `VP_SAVER_MUSIC`. To make a test track with a beat map once:

```bash
python tools/make_test_track.py        # music/test_120bpm.wav + .beats
```

The easiest way to start a screensaver is to **double-click `<folder>/run_pc.bat`**. It takes the music from `music/`. Esc quits, START 2 skips to the next track.

`VP_STATS=1` prints the lines per frame and the timings once a second.

## Raspberry Pi (arm64)

As for Vector Racer: clang and lld on the PC, against the copy of the cabinet's libraries (`F:\Tempest 2000 Port\build\pi-sysroot`).

```bash
export PATH="/c/Users/ocare/.local/share/retcomm/toolchains/cmake-clang-v1/latest/bin:$PATH"
cmake -S . -B build/pi -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-pi-clang.cmake -DCMAKE_BUILD_TYPE=Release "-DVPS_PI_SYSROOT=F:/Tempest 2000 Port/build/pi-sysroot"
cmake --build build/pi
```

## Packaging and installing on the cabinet

```bash
sh tools/make_vps.sh neondrive      # build/<id>-<version>.vps, to install from USB
sh tools/deploy.sh neondrive        # directly over SSH to /persistent/screensavers/<id>
```

Because `package/game.ini` contains `kind = screensaver`, this makes a `.vps`, not a `.vpk`. On the cabinet: SETTINGS → SCREENSAVERS → INSTALL (USB), then choose it under SETTINGS → SCREENSAVERS → SELECT.

`deploy.sh` uses the same key and cabinet as Vector Racer (`~/.ssh/id_ed25519_vectorpie`, `pi@192.168.0.122`). It replaces the program even while the screensaver is running.

A screensaver's version is in two places and must be changed in both: in `<folder>/package/game.ini` (`version`) and in `<folder>/CMakeLists.txt` (`project(... VERSION ...)`).

## Adding a new screensaver

1. Make a folder with `CMakeLists.txt` (like `neondrive/CMakeLists.txt`), `src/`, `package/game.ini` (with `kind = screensaver` and its own `id` with the prefix `saver_`) and `run_pc.bat`.
2. Add `add_subdirectory(<folder>)` to the top-level `CMakeLists.txt`.
3. Add it to the list in `README.md`.
