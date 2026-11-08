# Status

Status as of 2026-11-08, version 0.7.1: **Clock** (`saver_clock`).

## What it can do

- **Fifteen clock faces** (see README), six of them analogue, each with its own hands. One cross-fades into the next in 1.4 s, and the name of the new one is shown briefly at the bottom.
- **Its own menu** (F2 or Tab, likewise to close) has 19 rows and scrolls. You can set:
  - clock face and randomness (off, at start, always)
  - change (off, with every track, every 1, 5, 15 or 60 minutes, after every weather report)
  - switching with the spinner, joystick left/right, a learned key or not at all
  - second hand off, sweeping or ticking
  - the colours of clock, date and weather, each on its own, also RANDOM: a new one of the seven plain colours with every clock face. During the cross-fade the old clock face keeps its colour.
  - seconds, date, weather line, 24 hours, track title
  - weather report (every 1, 2, 3, 5, 10, 15, 30 or 60 minutes) and its duration (10 to 60 s)
  - weather view and weather colours (multicoloured or single colour)
  - place and language
  - A change takes effect immediately.
  - On closing, it is saved to `save/clock.ini`.
  - Without input, the menu closes after 60 s.
- **Place:**
  - Local time of the cabinet, one of 59 cities, or a search via an on-screen keyboard (Open-Meteo geocoding, up to 6 results).
  - Time and weather follow the place. Open-Meteo supplies the offset from UTC, including daylight saving time, together with the weather.
  - Without a place, the cabinet's local time applies and there is no weather.
- **Languages:** English (default), German, French and Spanish. This applies to weekdays, months, weather words, menu, the names of the clock faces and the word clock. The word clock has its own grid and its own grammar for each language, for example "ES IST FUENF VOR HALB DREI", "IL EST DEUX HEURES MOINS LE QUART" or "SON LAS DOS Y MEDIA".
- **Weather report:** It fades in full screen over the clock and back again. If "after every weather report" is set, the next clock face follows. With the weather report switched on, the random choice does not pick the WEATHER clock face, otherwise the clock would stay on it.
- **Weather views:** OVERVIEW, SIMPLE, TODAY, DAYS (7 days), HOURS (24 hours as a curve) or ROTATING. Each is available multicoloured or in a single colour. The data comes from Open-Meteo: now, hourly for 24 hours, daily for 7 days, with sunrise and sunset.
- **Date and weather** are on one line below every clock face except WEATHER, each in its own colour. They are on or off depending on the menu; the degree sign is drawn.
- **Music, everything smooth:** With loud music, rings and marks glow a little brighter. Nothing blinks.

## Checked on the cabinet (2026-11-08)

- 0.7.1: Waking up is immediate. Previously, quitting hung while a weather fetch was running: the normal `exit()` flushes all streams and in doing so waited on the pipe to the fetch. If open-meteo was unreachable, this took 15 s for curl and then up to 20 × 15 s for wget, i.e. minutes. Now the clock ends with `_Exit()`, curl gives up after 6 s without a connection, and wget tries only once. Measured: ends immediately after the key press, even while open-meteo was not responding.

- 0.3.0 and 0.3.1 installed, 40 frames per second, about 500 lines, no dropped lines.
- Taking over the controls works. The screensaver runs as `pi`, which belongs to the group `input`.
  - Grabbed are the control panel keyboard (Arduino Micro Keyboard), the spinner (Ultimarc SpinTrak), the Arduino mouse and the 2.4G wireless device with keyboard, mouse and Consumer Control.
  - Since 0.3.1 the power button (`pwr_button`), the wireless device's system keys, the USB audio device and HDMI are left untouched. In 0.3.0 the clock had also grabbed the power button.
  - The SDK sees no joystick; the controls come in as a keyboard. The keys follow the SDK's standard keys (START 1 = `KEY_1`, Button 1 = `KEY_LEFTCTRL`, Button 2 = `KEY_LEFTALT`), and the clock's mapping matches.
  - In the test someone pressed Button 3 (`KEY_SPACE`): the clock ended as required. Turning the spinner does not wake it.
- `curl` and `wget` are present. Open-Meteo weather and place search respond.

## Checked (on the PC)

- 0.7.0: All clock faces in their own matching colours (ORBIT, TUNNEL and WEATHER already were). New colour option RANDOM MULTI; RANDOM is now called RANDOM SINGLE. All clock faces viewed on the PC, plus four with RANDOM MULTI. On the cabinet 40 frames per second.

- 0.6.0:
  - Five new analogue clock faces (MINIMAL, NUMERALS, ROMAN, SQUARE, FULL SCREEN) with their own hands viewed. On the cabinet 40 frames per second, about 440 to 520 lines.
  - The date window of ANALOG is larger (380 × 230 instead of 220 × 160, number 110 instead of 90 high).
  - Switching with a custom key via `SAVER_SCRIPT` (`K57`): learned as "CUSTOM: BUTTON 3", saved, after which Button 3 switches onward.
  - Saved clock faces: WEATHER was number 9 and is now 14. Anyone who had saved WEATHER lands on MINIMAL once.

- 0.5.1: The spinner was too sensitive. A step now needs 240 instead of 60 counts, after which it rests for 0.35 s: one spin, one step. Still to be judged by hand on the cabinet.

- 0.5.0: Spinner checked via `SAVER_SCRIPT` (`SL`, `SR`): ROCKS, WEATHER, weather report, ANALOG and back. The analogue clock face sits higher and is a little smaller (centre 2150, radius 1430), with space to the line below. On the cabinet the SpinTrak is recognised as a spinner.

- 0.4.2: Weather report also every 1, 2 or 3 minutes. It is now saved in minutes (`report_minutes`); an old `report = <slot>` is converted. Checked on the PC: old "every 5" read, set to 1 minute in the menu, the report came one minute after closing.

- 0.4.1: ROCKS fixed. The rocks jumped 300 units downwards every frame (bug in the wrap-around at the edge) and therefore seemed to move at breakneck speed. Now they drift slowly (14 to 36 units per second) and rotate leisurely. Checked on the PC over 8 s.

- 0.4.0: Menu opened and closed with F2/Tab via `SAVER_SCRIPT` (`MENU`), setting saved in the process. RANDOM for all three colours: every clock face came in a different colour, date and weather changed along with it. On the cabinet 40 frames per second, no dropped lines.

- 0.3.0:
  - All weather views viewed with real data for Berlin, multicoloured and single colour.
  - Checked the weather report at a fast rate (`SAVER_REPORT_EVERY`) over 60 s: random clock faces, a new one after every report, the views alternating.
  - Separate colours for clock, date and weather.
  - The scrolling menu.
  - Fixed along the way: with "after weather report" the clock face changed constantly, and the random choice could get stuck on WEATHER.

- 0.2.1: Circles are round on the 4:3 monitor. The lines are compressed horizontally by 0.75; text keeps its shape. All clock faces were viewed afterwards.

- PC and Pi builds without warnings.
- The menu was played through via `SAVER_SCRIPT`:
  - clock face, colour and language changed
  - Cologne chosen from the list: weather and local time in French, "SAM 7 NOV COLOGNE 7°C COUVERT"
  - "OSLO" searched via the on-screen keyboard: 6 results, Oslo in Norway chosen
  - the English word clock ("IT IS TEN PAST NINE") checked in amber
  - saving and loading the settings
- Version 0.1.0 ran on the PC at 40 frames per second and up to about 900 lines.

## Open

- **Still to be checked by hand on the cabinet:**
  - whether F2 or Tab opens the menu and joystick and buttons work in it
  - whether the cabinet's menu reappears after waking and does not immediately restart the screensaver
  - With `CLOCK_INPUT_LOG=1` (and `VP_SAVER=1` when started by hand) the clock shows every key. With `CLOCK_NO_GRAB=1` grabbing can be switched off.
- **Time zone:** Without a place, or as long as no weather has arrived from the internet, the clock shows the Pi's time. Its time zone was set to America/New_York and has been changed to Europe/Berlin since 2026-11-08 (`timedatectl set-timezone`, plus `/etc/timezone`). With a place under PLACE, its time zone applies after the first weather. If the internet drops out after that, this offset remains; it is correct until the next daylight saving change.
- Not yet judged on the real monitor.
