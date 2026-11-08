/* input.h — the clock's own controls on the cabinet.
 *
 * The cabinet's menu ends a screensaver on any control (only NEXT TRACK
 * keeps it). For its own menu the clock takes the controls over while it
 * runs: it opens the input devices and grabs them (EVIOCGRAB), so the menu
 * sees nothing, and does itself what the menu would do. F2 or Tab opens (and
 * closes) the clock's menu, START 2 is NEXT TRACK, what the menu sets steps
 * through the faces (and the weather report): the spinner, left and right,
 * a button of one's own, or nothing. Any other control (START 1 too) wakes
 * the cabinet:
 * the clock lets the devices go (so the menu sees that control let go) and
 * ends.
 *
 * Only on the cabinet (Linux, run as its screensaver); elsewhere (a PC)
 * input_start does nothing and the game's own controls are read instead.
 */
#ifndef INPUT_H
#define INPUT_H

enum { IN_NONE, IN_UP, IN_DOWN, IN_LEFT, IN_RIGHT, IN_OK, IN_BACK, IN_START1, IN_START2, IN_MENU,
       IN_SPIN_L, IN_SPIN_R,    /* a step through the faces (the spinner: in the menu it comes as IN_UP / IN_DOWN) */
       IN_LEARNED,              /* learning: a button was pressed, input_learned() says which */
       IN_WAKE };

/* Grab the devices. 1: the clock has the controls. */
int  input_start(void);

/* The next control pressed since the last call, IN_NONE when none. `menu`:
 * whether the clock's menu is open (there the stick moves, and every button
 * is the menu's: none wakes). */
int  input_next(int menu);

/* Let the devices go (before ending). */
void input_release(void);

/* Learning a button (for SW_CUSTOM): from now the next button pressed
 * comes as IN_LEARNED (F2/Tab as IN_MENU: given up). 0 stops it. */
void input_learn(int on);
int  input_learned(void);

/* A button pressed (its Linux key code): what it is to the clock. On the
 * cabinet input_next uses it; SAVER_SCRIPT's K<code> too. */
int  input_key(int code, int menu);

/* A button's name, for the menu: "BUTTON 1", "START 1" ..., else "KEY <code>". */
void input_key_name(int code, char *buf, int size);

#endif
