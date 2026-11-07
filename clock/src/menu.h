/* menu.h — the clock's own menu, on the vector screen: what it shows, how
 * it changes faces, its colors and parts, the place (from a list or found
 * by name) and the language. It changes SET (settings.h) at once; the clock
 * follows. */
#ifndef MENU_H
#define MENU_H

#include <vectorpie/vp.h>

int  menu_is_open(void);
void menu_open(void);
/* a control (input.h IN_*); returns 1 when the menu closed with it */
int  menu_input(int in);
/* returns 1 when it closed by itself (left alone a while) */
int  menu_update(float dt);
void menu_draw(vp_dvg *d);

#endif
