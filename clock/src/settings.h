/* settings.h — what the clock's menu sets, kept in save/clock.ini beside
 * the program (the package's clock.ini, beside the program, gives the
 * defaults). */
#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct {
    int lang;                   /* LANG_EN .. (lang.h); English is the standard */
    int face;                   /* 0 .. 9 */
    int change;                 /* 0 off, 1 with each track, 2.. every CHANGE_MIN minutes */
    int theme;                  /* 0 the faces' own colors, else one (lang.h THEME_RGB) */
    int seconds, date, weather, h24, title;   /* 1: shown (24 hours: else 12) */
    int has_place;              /* 0: the cabinet's own time, no weather */
    char place[48];
    double lat, lon;
} settings_t;

extern settings_t SET;

void settings_load(const char *exe_path);
void settings_save(void);

#endif
