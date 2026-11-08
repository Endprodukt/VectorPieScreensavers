/* settings.h — what the clock's menu sets, kept in save/clock.ini beside
 * the program (the package's clock.ini, beside the program, gives the
 * defaults). */
#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct {
    int lang;                   /* LANG_EN .. (lang.h); English is the standard */
    int face;                   /* 0 .. 9 */
    int change;                 /* 0 off, 1 with each track, 2.. every CHANGE_MIN minutes */
    int theme;                  /* the clock's color: 0 the faces' own, else one (lang.h THEME_RGB) */
    int date_theme, wx_theme;   /* the date's and the weather's: the same */
    int random;                 /* the face: 0 as set, 1 a random one at start, 2 random at each change too */
    int report, report_len;     /* the weather report: REPORT_MIN (below), REPORT_LEN_S (lang.h) */
    int wx_style, wx_colorful;  /* how the weather is shown (WS_*); 1 in its own colors, else wx_theme */
    int seconds, date, weather, h24, title;   /* 1: shown (24 hours: else 12) */
    int has_place;              /* 0: the cabinet's own time, no weather */
    char place[48];
    double lat, lon;
} settings_t;

/* the weather report's rhythm: off, or every so many minutes. Saved as
 * minutes (report_minutes), so the list can grow. */
#define NREPORT 9
static const int REPORT_MIN[NREPORT] = { 0, 1, 2, 3, 5, 10, 15, 30, 60 };

extern settings_t SET;

void settings_load(const char *exe_path);
void settings_save(void);

#endif
