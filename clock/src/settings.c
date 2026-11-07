/* settings.c — see settings.h. */
#include "settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define make_dir(p) _mkdir(p)
#else
#include <sys/stat.h>
#define make_dir(p) mkdir(p, 0755)
#endif

settings_t SET = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, "", 0, 0 };

static char dir[512];                   /* the program's folder, with its slash */

static void trim(char *s)
{
    char *e = s + strlen(s);
    while (e > s && (e[-1] == '\n' || e[-1] == '\r' || e[-1] == ' ' || e[-1] == '\t')) *--e = 0;
    char *b = s;
    while (*b == ' ' || *b == '\t') b++;
    memmove(s, b, strlen(b) + 1);
}

static int read_file(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    char ln[256];
    int lat = 0, lon = 0;
    while (fgets(ln, sizeof ln, f)) {
        char *c = strchr(ln, ';');
        if (c) *c = 0;
        char *eq = strchr(ln, '=');
        if (!eq) continue;
        *eq = 0;
        char *k = ln, *v = eq + 1;
        trim(k);
        trim(v);
        int i = atoi(v);
        if (!strcmp(k, "language")) SET.lang = i;
        else if (!strcmp(k, "face")) SET.face = i;
        else if (!strcmp(k, "change")) SET.change = i;
        else if (!strcmp(k, "color")) SET.theme = i;
        else if (!strcmp(k, "seconds")) SET.seconds = i;
        else if (!strcmp(k, "date")) SET.date = i;
        else if (!strcmp(k, "weather")) SET.weather = i;
        else if (!strcmp(k, "hours24")) SET.h24 = i;
        else if (!strcmp(k, "title")) SET.title = i;
        else if (!strcmp(k, "place")) snprintf(SET.place, sizeof SET.place, "%s", v);
        else if (!strcmp(k, "lat")) { SET.lat = atof(v); lat = 1; }
        else if (!strcmp(k, "lon")) { SET.lon = atof(v); lon = 1; }
    }
    fclose(f);
    if (lat && lon && SET.place[0]) SET.has_place = 1;
    else if (!SET.place[0]) SET.has_place = 0;
    return 1;
}

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

void settings_load(const char *exe_path)
{
    snprintf(dir, sizeof dir, "%s", exe_path ? exe_path : "");
    char *slash = strrchr(dir, '/'), *bslash = strrchr(dir, '\\');
    if (bslash > slash) slash = bslash;
    if (slash) slash[1] = 0;
    else dir[0] = 0;
    char path[600];
    snprintf(path, sizeof path, "%sclock.ini", dir);        /* the defaults */
    read_file(path);
    snprintf(path, sizeof path, "%ssave/clock.ini", dir);   /* the menu's */
    read_file(path);
    SET.lang = clampi(SET.lang, 0, 3);
    SET.face = clampi(SET.face, 0, 9);
    SET.change = clampi(SET.change, 0, 5);
    SET.theme = clampi(SET.theme, 0, 8);
}

void settings_save(void)
{
    char path[600];
    snprintf(path, sizeof path, "%ssave", dir);
    make_dir(path);
    snprintf(path, sizeof path, "%ssave/clock.ini", dir);
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "; Clock: set in its menu (START 1 while it runs)\n");
    fprintf(f, "language = %d\nface = %d\nchange = %d\ncolor = %d\n", SET.lang, SET.face, SET.change, SET.theme);
    fprintf(f, "seconds = %d\ndate = %d\nweather = %d\nhours24 = %d\ntitle = %d\n", SET.seconds, SET.date,
            SET.weather, SET.h24, SET.title);
    fprintf(f, "place = %s\n", SET.has_place ? SET.place : "");
    if (SET.has_place) fprintf(f, "lat = %.4f\nlon = %.4f\n", SET.lat, SET.lon);
    fclose(f);
}
