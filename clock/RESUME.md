# Continuing

**Clock** is the seventh screensaver in the "VectorPie Screensavers" repository (folder `clock/`). Building, packaging and installing: see `../BUILD.md`. Status: see `STATUS.md`.

## User's wishes (2026-11-07)

- A clock with all sorts of clock faces: digital, analogue, flip numbers, binary and whatever else comes to mind.
- One clock face each in the style of Tempest and of Asteroids. Implemented as TUNNEL and ROCKS, with their own shapes and without the games' names or graphics.
- Date, weather "and that sort of thing", because the cabinet is online.
- **A menu inside the running screensaver.** The user chose route 1: the clock takes over the controls itself.
- Settable in the menu:
  - city and language: English, German, French, Spanish. **English is the default.**
  - colours
  - the place for time and weather
  - putting together the clock face yourself: with or without date, weather, etc.
- **Changing with the track is only an option, not the default.** The default is: no change.
- Different colours for clock, date and weather (2026-11-07).
- The weather report in full screen at an adjustable interval, also short: after 1, 2 minutes, etc. (2026-11-08).
- The clock face can be random: at start, after a song, after a time or after the weather report.
- The weather report comes in several views: simple, today only, several days, over the day, single colour or multicoloured.
- Nothing may blink or pulse. Music only changes things smoothly.

## How the clock takes over the controls (`src/input.c`)

According to the SDK (`vendor/VectorPie/sdk/docs/09-screensavers.md`), the cabinet's menu reads the controls itself. On any key except NEXT TRACK it ends the screensaver.

So when it runs as a screensaver (`vp_saver_active()`), the clock opens the `/dev/input/event*` devices of the controls and grabs them with `EVIOCGRAB`. Controls means: keyboard (`KEY_1` or `KEY_LEFTCTRL`), mouse and spinner (`BTN_LEFT`), joystick (`BTN_TRIGGER`, `BTN_SOUTH`) or axes. Power button, volume keys and HDMI (`vc4-hdmi`) stay with the system. Then the cabinet's menu no longer sees anything, and the clock itself does what the menu would otherwise do:
- **F2 or Tab** (`KEY_F2`, `KEY_TAB`, in the code `IN_MENU`) opens the clock menu and closes it again. START 1 wakes like any other key (wish 2026-11-08: START 1 is not suitable for the menu).
- **START 2** calls `vp_saver_skip()` (NEXT TRACK).
- **Quitting:** `main()` ends with `_Exit()`, not `return`. A normal exit would wait for a running weather fetch (see STATUS 0.7.1). Anything that still needs saving at the end must be saved before `_Exit()`.
- **Switching** (`SET.switch_by`, wish 2026-11-08): spinner, left/right, a custom key (learned in the menu: `input_learn`, `IN_LEARNED`, saved as a Linux key code in `switch_key`) or nothing. The decision is made by `input_key()` in `input.c`. `SAVER_SCRIPT` can press a key with `K<code>`.
- **The spinner** (a device with "spin" in its name, `REL_X` or `REL_DIAL`, `SPIN_STEP` 240 counts per step, then a `SPIN_REST` pause of 0.35 s; 60 was too sensitive) switches through the clock faces. After the last one, the weather report comes as a stop (`spin()`, `G.report_spun`). In the menu it acts as up and down (wish 2026-11-08).
- **Any other key** releases the devices (`input_release`), so that the cabinet's menu at least sees the release, and ends the clock (`vp_game_quit`).
- **In the clock menu** all keys belong to the menu. Analogue sticks only count from three quarters deflection, and outside the menu not at all (joystick drift).

Still to be checked on the cabinet, see STATUS "Open". Aids for this:
- `CLOCK_INPUT_LOG=1` writes devices and events to stderr. When started by hand this also needs `VP_SAVER=1`, otherwise the clock grabs nothing.
- `CLOCK_NO_GRAB=1` switches grabbing off.

## Structure

- `src/main.c`:
  - All lines go through `mv` and `ln` (macros for `vp_moveto` and `vp_lineto`) and are compressed horizontally in the process. Text positions are compressed by `SX()`; `SQUEEZE = 0` switches this off.
  - the clock faces (`face_*`), the cross-fade, `info_line`, the word clock (`WC_*`, `words_target` per language)
  - the controls: `control` passes on to the menu or opens it, `wake` ends the clock
  - `follow_settings`: whatever the menu changes in `SET` is adopted, for example a new place for the weather or new words for the word clock.
  - Every clock face has its own bright colours (wish 2026-11-08). The analogue ones from `dial_*` take them from a `dial_pal` (`DP`); the others are directly in the code.
  - RANDOM MULTI (`THEME_RANDOM_MULTI`): `color()` rotates every colour with `hue_turn()` by `PAINT_TURN` (random per clock face and part, `TURN` in `pick_colors()`). White and grey stay.
  - The colour comes from `color()`: with a colour theme (`PAINT`), every colour is replaced by that of the theme, with the brightness of the original. While drawing, `PAINT` is set to the clock colour, and for date and weather to their colour.
  - The weather: `weather_view(style)` with `wx_overview`, `wx_simple`, `wx_today`, `wx_days`, `wx_hours`. Plus `text_deg`: text with `^` in place of a degree sign, which is drawn.
  - The weather report: `report_update`. The next change is determined by `next_face`, which is also where the randomness lives.
- `src/menu.c`: menu, place list, on-screen keyboard and search results. It changes `SET` immediately.
- `src/settings.c`: `SET`. The defaults are in `clock.ini` next to the program; saving goes to `save/clock.ini`.
- `src/weather.c`: a thread that fetches via `curl` or `wget`:
  - the weather and the UTC offset of the place (`weather_set_place`)
  - the place search (`weather_search`)
- `src/lang.h`: all words in the four languages. `src/cities.h`: the city list.
- For testing on the PC:
  - `SAVER_FACE=<0..9>` and `SAVER_CYCLE=<s>`
  - `SAVER_SCRIPT="<s>:<control>,..."`: presses keys at these times (U D L R OK BACK MENU S1 S2)
  - `SAVER_REPORT_EVERY=<s>`: the weather report this often instead of at the set interval
  - `SAVER_SHOTS=<folder>`

## Round on the tube

The board has a square coordinate field, but the monitor stretches it to 4:3 (3:4 with a vertical monitor). Anything drawn round in 2D therefore appears a third too wide. The 3D screensavers compensate for this via `view.aspect`. In 2D the rule is: compress horizontally towards the centre by 0.75 (`ASPECT_X`; with `VP_VERTICAL=1` the height instead). The user noticed this on the clock (2026-11-07).
