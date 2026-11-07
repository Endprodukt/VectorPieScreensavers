@echo off
rem run_pc.bat <folder>: a screensaver on the PC, with the music in the
rem repository's music folder. Each screensaver's folder has its own
rem run_pc.bat to double-click, calling this. Esc quits.
set saver=%1
for /f "tokens=2 delims==; " %%e in ('findstr /b "executable" "%~dp0..\%saver%\package\game.ini"') do set exe=%%e
cd /d "%~dp0..\build\pc\%saver%"
start "" %exe%.exe "%~dp0..\music"
