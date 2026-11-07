/* menu.c — see menu.h. Drawn like the cabinet's own settings: a red title,
 * rows with their values between arrows, the chosen row in a box. */
#include "menu.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cities.h"
#include "input.h"
#include "lang.h"
#include "settings.h"
#include "weather.h"

#define W     VP_DVG_COORD_MAX
#define CX    (W / 2.0f)
#define IDLE_S 60.0f                    /* left alone this long, it closes */

enum { M_FACE, M_RANDOM, M_CHANGE, M_COLOR, M_DATE_COLOR, M_WX_COLOR, M_SECONDS, M_DATE, M_WEATHER, M_H24,
       M_TITLE, M_REPORT, M_REPORT_LEN, M_WX_STYLE, M_WX_COLORS, M_PLACE, M_LANG, M_CLOSE, NROW };
enum { SCR_MAIN, SCR_PLACE, SCR_KEYS, SCR_FOUND };

static struct {
    int open, screen, row;
    int place_row;                      /* in the place list: 0 local, 1 search, 2.. the cities */
    int key_r, key_c;
    char query[25];
    int found_row;
    float idle;
} M;

static const char *KEYS[3] = { "ABCDEFGHI", "JKLMNOPQR", "STUVWXYZ-" };

int menu_is_open(void) { return M.open; }

void menu_open(void)
{
    M.open = 1;
    M.screen = SCR_MAIN;
    M.row = 0;
    M.idle = 0;
}

static int wrap(int v, int n) { return ((v % n) + n) % n; }

static void place_city(int i)
{
    SET.has_place = 1;
    snprintf(SET.place, sizeof SET.place, "%s", CITY[i].name);
    SET.lat = CITY[i].lat;
    SET.lon = CITY[i].lon;
}

/* the place list's row of the place set now */
static int place_row_now(void)
{
    if (!SET.has_place) return 0;
    for (int i = 0; i < NCITY; i++)
        if (!strcmp(SET.place, CITY[i].name)) return 2 + i;
    return 1;
}

static void change(int dir)
{
    switch (M.row) {
    case M_FACE:    SET.face = wrap(SET.face + dir, 10); break;
    case M_CHANGE:  SET.change = wrap(SET.change + dir, NCHANGE); break;
    case M_COLOR:   SET.theme = wrap(SET.theme + dir, NTHEME); break;
    case M_RANDOM:  SET.random = wrap(SET.random + dir, 3); break;
    case M_DATE_COLOR: SET.date_theme = wrap(SET.date_theme + dir, NTHEME); break;
    case M_WX_COLOR: SET.wx_theme = wrap(SET.wx_theme + dir, NTHEME); break;
    case M_REPORT:  SET.report = wrap(SET.report + dir, NREPORT); break;
    case M_REPORT_LEN: SET.report_len = wrap(SET.report_len + dir, NREPORT_LEN); break;
    case M_WX_STYLE: SET.wx_style = wrap(SET.wx_style + dir, NWSTYLE); break;
    case M_WX_COLORS: SET.wx_colorful ^= 1; break;
    case M_SECONDS: SET.seconds ^= 1; break;
    case M_DATE:    SET.date ^= 1; break;
    case M_WEATHER: SET.weather ^= 1; break;
    case M_H24:     SET.h24 ^= 1; break;
    case M_TITLE:   SET.title ^= 1; break;
    case M_LANG:    SET.lang = wrap(SET.lang + dir, NLANG); break;
    case M_PLACE: {                     /* through the list: local time, then the cities */
        int r = place_row_now();
        r = r <= 1 ? (dir > 0 ? 2 : 1 + NCITY) : r + dir;
        if (r < 2 || r > 1 + NCITY) { SET.has_place = 0; SET.place[0] = 0; }
        else place_city(r - 2);
        break;
    }
    }
}

int menu_input(int in)
{
    if (!M.open || in == IN_NONE) return 0;
    M.idle = 0;
    if (in == IN_START1) in = IN_OK;
    if (in == IN_START2) in = IN_BACK;
    switch (M.screen) {
    case SCR_MAIN:
        if (in == IN_UP) M.row = wrap(M.row - 1, NROW);
        else if (in == IN_DOWN) M.row = wrap(M.row + 1, NROW);
        else if (in == IN_LEFT) change(-1);
        else if (in == IN_RIGHT) change(1);
        else if (in == IN_OK) {
            if (M.row == M_CLOSE) { M.open = 0; return 1; }
            if (M.row == M_PLACE) { M.screen = SCR_PLACE; M.place_row = place_row_now(); }
            else change(1);
        } else if (in == IN_BACK) { M.open = 0; return 1; }
        break;
    case SCR_PLACE:
        if (in == IN_UP) M.place_row = wrap(M.place_row - 1, NCITY + 2);
        else if (in == IN_DOWN) M.place_row = wrap(M.place_row + 1, NCITY + 2);
        else if (in == IN_LEFT) M.place_row = M.place_row - 8 < 0 ? 0 : M.place_row - 8;
        else if (in == IN_RIGHT) M.place_row = M.place_row + 8 > NCITY + 1 ? NCITY + 1 : M.place_row + 8;
        else if (in == IN_OK) {
            if (M.place_row == 0) { SET.has_place = 0; SET.place[0] = 0; M.screen = SCR_MAIN; }
            else if (M.place_row == 1) { M.screen = SCR_KEYS; M.query[0] = 0; M.key_r = M.key_c = 0; }
            else { place_city(M.place_row - 2); M.screen = SCR_MAIN; }
        } else if (in == IN_BACK) M.screen = SCR_MAIN;
        break;
    case SCR_KEYS: {
        int cols = M.key_r == 3 ? 3 : 9;
        if (in == IN_UP) { M.key_r = wrap(M.key_r - 1, 4); }
        else if (in == IN_DOWN) { M.key_r = wrap(M.key_r + 1, 4); }
        else if (in == IN_LEFT) M.key_c = wrap(M.key_c - 1, cols);
        else if (in == IN_RIGHT) M.key_c = wrap(M.key_c + 1, cols);
        else if (in == IN_BACK) M.screen = SCR_PLACE;
        else if (in == IN_OK) {
            size_t n = strlen(M.query);
            if (M.key_r < 3) {
                if (n + 1 < sizeof M.query) { M.query[n] = KEYS[M.key_r][M.key_c]; M.query[n + 1] = 0; }
            } else if (M.key_c == 0) {
                if (n) M.query[n - 1] = 0;
            } else if (M.key_c == 1) {
                if (n && n + 1 < sizeof M.query && M.query[n - 1] != ' ') { M.query[n] = ' '; M.query[n + 1] = 0; }
            } else if (n) {
                weather_search(M.query, SET.lang);
                M.screen = SCR_FOUND;
                M.found_row = 0;
            }
        }
        if (M.key_r == 3 && M.key_c > 2) M.key_c = 2;
        break;
    }
    case SCR_FOUND: {
        place_t res[6];
        int n = 0, st = weather_search_state(res, 6, &n);
        if (in == IN_BACK) M.screen = SCR_KEYS;
        else if (st == 2 && n > 0) {
            if (in == IN_UP) M.found_row = wrap(M.found_row - 1, n);
            else if (in == IN_DOWN) M.found_row = wrap(M.found_row + 1, n);
            else if (in == IN_OK) {
                SET.has_place = 1;
                snprintf(SET.place, sizeof SET.place, "%s", res[M.found_row].name);
                SET.lat = res[M.found_row].lat;
                SET.lon = res[M.found_row].lon;
                M.screen = SCR_MAIN;
            }
        } else if (in == IN_OK && st >= 2) M.screen = SCR_KEYS;
        break;
    }
    }
    return 0;
}

int menu_update(float dt)
{
    if (!M.open) return 0;
    if ((M.idle += dt) >= IDLE_S) { M.open = 0; return 1; }
    return 0;
}

/* ---- drawing ------------------------------------------------------------ */

static void col(vp_dvg *d, int r, int g, int b) { vp_color(d, (uint8_t)r, (uint8_t)g, (uint8_t)b); }

static void box(vp_dvg *d, float x0, float y0, float x1, float y1)
{
    vp_moveto(d, (int)x0, (int)y0);
    vp_lineto(d, (int)x1, (int)y0);
    vp_lineto(d, (int)x1, (int)y1);
    vp_lineto(d, (int)x0, (int)y1);
    vp_lineto(d, (int)x0, (int)y0);
}

static const char *S(int i) { return L_STR[SET.lang][i]; }

static void value_of(int row, char *buf, size_t size)
{
    const char *on = S(S_ON), *off = S(S_OFF);
    switch (row) {
    case M_FACE:    snprintf(buf, size, "%s", L_FACE[SET.lang][SET.face]); break;
    case M_CHANGE:
        if (SET.change == 0) snprintf(buf, size, "%s", off);
        else if (SET.change == 1) snprintf(buf, size, "%s", S(S_WITH_TRACK));
        else if (SET.change == CHANGE_AFTER_REPORT) snprintf(buf, size, "%s", S(S_AFTER_REPORT));
        else snprintf(buf, size, S(S_EVERY_MIN), CHANGE_MIN[SET.change]);
        break;
    case M_COLOR:   snprintf(buf, size, "%s", L_THEME[SET.lang][SET.theme]); break;
    case M_RANDOM:  snprintf(buf, size, "%s", SET.random == 0 ? off : SET.random == 1 ? S(S_AT_START) : S(S_ALWAYS)); break;
    case M_DATE_COLOR: snprintf(buf, size, "%s", L_THEME[SET.lang][SET.date_theme]); break;
    case M_WX_COLOR: snprintf(buf, size, "%s", L_THEME[SET.lang][SET.wx_theme]); break;
    case M_REPORT:
        if (SET.report == 0) snprintf(buf, size, "%s", off);
        else snprintf(buf, size, S(S_EVERY_MIN), REPORT_MIN[SET.report]);
        break;
    case M_REPORT_LEN: snprintf(buf, size, S(S_SEC), REPORT_LEN_S[SET.report_len]); break;
    case M_WX_STYLE: snprintf(buf, size, "%s", L_WSTYLE[SET.lang][SET.wx_style]); break;
    case M_WX_COLORS: snprintf(buf, size, "%s", SET.wx_colorful ? S(S_COLORFUL) : S(S_ONE_COLOR)); break;
    case M_SECONDS: snprintf(buf, size, "%s", SET.seconds ? on : off); break;
    case M_DATE:    snprintf(buf, size, "%s", SET.date ? on : off); break;
    case M_WEATHER: snprintf(buf, size, "%s", SET.weather ? on : off); break;
    case M_H24:     snprintf(buf, size, "%s", SET.h24 ? on : off); break;
    case M_TITLE:   snprintf(buf, size, "%s", SET.title ? on : off); break;
    case M_PLACE:   snprintf(buf, size, "%s", SET.has_place ? SET.place : S(S_LOCAL)); break;
    case M_LANG:    snprintf(buf, size, "%s", LANG_NAME[SET.lang]); break;
    default:        buf[0] = 0;
    }
}

static void title(vp_dvg *d, const char *s)
{
    col(d, 255, 60, 60);
    vp_text_fit(d, s, CX, 3500, 140, W * 0.8f, VP_TEXT_CENTER);
}

static void draw_main(vp_dvg *d)
{
    static const int LABEL[NROW] = { S_FACE, S_RANDOM, S_CHANGE, S_CLOCK_COLOR, S_DATE_COLOR, S_WX_COLOR, S_SECONDS,
                                     S_DATE, S_WX_LINE, S_H24, S_TITLE, S_REPORT, S_REPORT_LEN, S_WX_STYLE,
                                     S_WX_COLORS, S_PLACE, S_LANGUAGE, S_CLOSE };
    title(d, S(S_SETTINGS));
    /* more rows than fit: a window of them round the chosen one */
    const int SHOW = 11;
    int first = M.row - SHOW / 2;
    if (first > NROW - SHOW) first = NROW - SHOW;
    if (first < 0) first = 0;
    const float top = 3150, step = 240, h = 95;
    col(d, 150, 160, 190);
    if (first > 0) vp_text(d, "...", CX, top + 170, 60, VP_TEXT_CENTER);
    if (first + SHOW < NROW) vp_text(d, "...", CX, top - SHOW * step + 150, 60, VP_TEXT_CENTER);
    for (int r = first; r < first + SHOW && r < NROW; r++) {
        float y = top - (r - first) * step;
        if (r == M.row) { col(d, 255, 220, 60); box(d, 330, y - 75, W - 330, y + h + 75); }
        if (r == M_CLOSE) {
            col(d, 230, 235, 255);
            vp_text(d, S(S_CLOSE), CX, y, h, VP_TEXT_CENTER);
            continue;
        }
        col(d, 230, 235, 255);
        vp_text_fit(d, S(LABEL[r]), 450, y, h, 1500, VP_TEXT_LEFT);
        char buf[64];
        value_of(r, buf, sizeof buf);
        col(d, 80, 220, 255);
        vp_text_fit(d, buf, W - 600, y, h, 1500, VP_TEXT_RIGHT);
        if (r == M.row && r != M_PLACE) {           /* the arrows: left and right change it */
            vp_text(d, "<", W - 600 - fminf(vp_text_width(buf, h), 1500) - 150, y, h, VP_TEXT_LEFT);
            vp_text(d, ">", W - 520, y, h, VP_TEXT_LEFT);
        } else if (r == M.row) {
            vp_text(d, ">", W - 520, y, h, VP_TEXT_LEFT);
        }
    }
    col(d, 150, 160, 190);
    vp_text(d, S(S_BACK_HINT), CX, 120, 70, VP_TEXT_CENTER);
}

static void draw_list(vp_dvg *d, const char *head, int n, int sel, const char *(*name)(int))
{
    title(d, head);
    const int SHOW = 11;
    int first = sel - SHOW / 2;
    if (first > n - SHOW) first = n - SHOW;
    if (first < 0) first = 0;
    const float top = 3150, step = 240, h = 95;
    for (int i = 0; i < SHOW && first + i < n; i++) {
        float y = top - i * step;
        int r = first + i;
        if (r == sel) { col(d, 255, 220, 60); box(d, 330, y - 75, W - 330, y + h + 75); }
        col(d, 230, 235, 255);
        vp_text_fit(d, name(r), CX, y, h, W - 900, VP_TEXT_CENTER);
    }
    col(d, 150, 160, 190);
    if (first > 0) vp_text(d, "...", CX, top + 170, 60, VP_TEXT_CENTER);
    if (first + SHOW < n) vp_text(d, "...", CX, top - SHOW * step + 150, 60, VP_TEXT_CENTER);
}

static const char *place_name(int r)
{
    return r == 0 ? S(S_LOCAL) : r == 1 ? S(S_SEARCH) : CITY[r - 2].name;
}

static place_t shown[6];
static const char *found_name(int r) { return shown[r].name; }

static void draw_keys(vp_dvg *d)
{
    title(d, S(S_SEARCH));
    /* what is typed so far */
    char buf[40];
    snprintf(buf, sizeof buf, "%s_", M.query);
    col(d, 80, 220, 255);
    vp_text(d, buf, CX, 2900, 150, VP_TEXT_CENTER);
    const float cw = 330, ch = 380, top = 2300, h = 130;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 9; c++) {
            float x = CX + (c - 4) * cw, y = top - r * ch;
            char k[2] = { KEYS[r][c], 0 };
            if (r == M.key_r && c == M.key_c) { col(d, 255, 220, 60); box(d, x - 130, y - 90, x + 130, y + h + 90); }
            col(d, 230, 235, 255);
            vp_text(d, k, x, y, h, VP_TEXT_CENTER);
        }
    const int SPECIAL[3] = { S_DELETE, S_SPACE, S_GO };
    for (int c = 0; c < 3; c++) {
        float x = CX + (c - 1) * 1050, y = top - 3 * ch;
        if (M.key_r == 3 && M.key_c == c) { col(d, 255, 220, 60); box(d, x - 450, y - 90, x + 450, y + 95 + 90); }
        col(d, c == 2 ? 80 : 230, c == 2 ? 220 : 235, 255);
        vp_text_fit(d, S(SPECIAL[c]), x, y, 95, 820, VP_TEXT_CENTER);
    }
}

static void draw_found(vp_dvg *d)
{
    int n = 0, st = weather_search_state(shown, 6, &n);
    if (st == 1) {
        title(d, S(S_SEARCH));
        col(d, 80, 220, 255);
        vp_text(d, S(S_SEARCHING), CX, 2000, 120, VP_TEXT_CENTER);
    } else if (n == 0) {
        title(d, S(S_SEARCH));
        col(d, 255, 120, 100);
        vp_text(d, S(S_NOTHING_FOUND), CX, 2000, 120, VP_TEXT_CENTER);
    } else {
        draw_list(d, M.query, n, M.found_row, found_name);
    }
}

void menu_draw(vp_dvg *d)
{
    if (!M.open) return;
    vp_text_font(VP_FONT_ASTEROIDS);
    switch (M.screen) {
    case SCR_MAIN:  draw_main(d); break;
    case SCR_PLACE: draw_list(d, S(S_PLACE), NCITY + 2, M.place_row, place_name); break;
    case SCR_KEYS:  draw_keys(d); break;
    case SCR_FOUND: draw_found(d); break;
    }
}
