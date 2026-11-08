/* Clock — a VectorPie screensaver: the time, on one face after another.
 *
 * Ten faces: ANALOG (a watch dial with a sweeping second hand), DIGITAL
 * (seven segments, the date under it), FLIP (a flip clock, its cards
 * folding over), BINARY (the digits in bits), WORDS (the time in words, in
 * the clock's language), NIXIE (glowing tubes), ORBIT (hours, minutes and
 * seconds circling
 * like planets), TUNNEL (a web of lanes down a tube, a claw running round its
 * rim: in the manner of the old vector tube shooters), ROCKS (the time among
 * drifting rocks, a little ship circling it: in the manner of the old vector
 * rock shooters) and WEATHER (the weather now and the next days). Under each
 * of the others a line with the date and the weather.
 *
 * ITS MENU (menu.c), opened with F2 or Tab while it runs: the face (or a
 * random one, at start or at each change), whether it changes by itself
 * (off, with each track, every so many minutes, after each weather report),
 * the colors of the clock, the date and the weather (their own or one each),
 * which parts show (seconds, date, weather line, 24 hours, the track's
 * title), the weather report (full screen every so many minutes, for so
 * many seconds), how the weather is shown (overview, simple, today, days,
 * through the day, or each in turn; in its own colors or one), the place
 * (its time and weather: from a list or searched by name) and the language
 * (English, German, French, Spanish). Kept in save/clock.ini (settings.c). For this the clock
 * takes the cabinet's controls over while it runs (input.c): START 2 is
 * still NEXT TRACK, any other control still wakes the cabinet.
 *
 * The weather and the place's time come from Open-Meteo over the cabinet's
 * network (weather.c); with no place the clock shows the cabinet's own time
 * and no weather.
 *
 * To the music, smoothly: the faces' rings and marks glow a little brighter
 * when it is louder. Nothing flashes or blinks (the colons stand still). A
 * new track shows its title.
 *
 * On the cabinet: installed in SETTINGS -> SCREENSAVERS (tools/deploy.sh or
 * the .vps). On a PC: saver_clock <a .ogg/.wav or a folder of them>. Esc
 * quits; Enter or M opens the menu, the arrow keys move in it (and change
 * the face outside it), Backspace goes back, N plays the next track.
 * SAVER_FACE=<0..9> starts on that face, SAVER_CYCLE=<s> switches
 * every s seconds, SAVER_SHOTS=<folder> saves a picture every 2 s (all for
 * development).
 */
#include <vectorpie/vp.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "input.h"
#include "lang.h"
#include "menu.h"
#include "settings.h"
#include "weather.h"

#define W      VP_DVG_COORD_MAX
#define CX     (W / 2.0f)
#define PI     3.14159265f
#define NFACE  NFACES          /* lang.h: the faces' names */
#define FACE_WEATHER (NFACE - 1)  /* the last: random faces and the report count on it */
#define BLEND  1.4f                     /* s from one face to the next */

/* h: 0..23; hd: the hour as shown (1..12 when the menu says 12 hours) */
typedef struct { int h, m, s, wday, mday, mon, year; float frac; int hd; } now_t;

static struct {
    float time, loud, cycle, cycle_t;
    int face, from;                     /* the face shown, the one it comes from */
    float blend;                        /* 0..1 from `from` to `face` */
    float name_time;                    /* the face's name under it */
    now_t now;
    /* the flip clock's cards: shown digit, the one before, how far it has folded */
    int flip_cur[6], flip_old[6];
    float flip_t[6];
    /* the nixie tubes: the same, fading */
    int nix_cur[6], nix_old[6];
    float nix_t[6];
    float bits[6][4];                   /* the binary face's lamps, 0..1 */
    float words[32];                    /* the word clock's words, 0..1 */
    int words_lang;                     /* the language they are in */
    weather_t wx;                       /* the latest weather (for the place set, else not ok) */
    int place_serial;                   /* the place asked for (weather_set_place) */
    settings_t was;                     /* the settings last seen: what the menu changed */
    float report_t, report_k;           /* the weather report: its timer; how far it is shown, 0..1 */
    int report_on, report_turn;         /* shown now; how many so far (the styles in turn) */
    int report_spun;                    /* the report now came from the spinner (not the rhythm) */
    int grabbed;                        /* the clock has the cabinet's controls */
    vp_game *game;
    vp_music_feed m;
    int have, started;
    unsigned last_track;
    float title_time;
    char title[VP_MUSIC_TEXT];
} G;

/* RANDOM colors (THEME_RANDOM): the colors picked for the clock, the date
 * and the weather (THEME_*); [0] the face now, [1] the one it blends from */
enum { PART_CLOCK, PART_DATE, PART_WX, NPART };
static int PICK[2][NPART], PICK_SLOT;
static float TURN[2][NPART];            /* RANDOM MULTI: how far round the color wheel (0..1) */
static float PAINT_TURN;                /* the turn now drawing */
static int theme(int v, int part)
{
    if (v == THEME_RANDOM_MULTI) PAINT_TURN = TURN[PICK_SLOT][part];
    return v == THEME_RANDOM ? PICK[PICK_SLOT][part] : v;
}

/* new colors at random for the next face, each unlike the last */
static void pick_colors(void)
{
    for (int p = 0; p < NPART; p++) {
        PICK[1][p] = PICK[0][p];
        int c;
        do c = 1 + rand() % 7; while (c == PICK[1][p]);    /* one of the plain colors */
        PICK[0][p] = c;
        TURN[1][p] = TURN[0][p];
        TURN[0][p] = 0.12f + 0.76f * (float)rand() / (float)RAND_MAX;   /* never near the own colors */
    }
}

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }

/* ROUND ON THE TUBE. The board's square of coordinates is stretched to the
 * monitor's 4:3 (vp_dvg.h), so a circle drawn round comes out a third too
 * wide. Every line here is drawn through these two, which pull it together
 * across the middle (along the height on a vertical 3:4 monitor). Text
 * (vp_text) keeps the font's own shape; only where it stands is pulled in
 * (SX). SQUEEZE 0 for a part laid out as text (the line under the faces). */
static float ASPECT_X = 0.75f, ASPECT_Y = 1;
static int SQUEEZE = 1;
static int PAINT;                       /* the color theme now drawing (lang.h THEME_RGB): 0 the own colors */
static float SX(float x) { return SQUEEZE ? CX + (x - CX) * ASPECT_X : x; }
static float SY(float y) { return SQUEEZE ? W / 2.0f + (y - W / 2.0f) * ASPECT_Y : y; }
static void mv(vp_dvg *d, float x, float y) { vp_moveto(d, (int)SX(x), (int)SY(y)); }
static void ln(vp_dvg *d, float x, float y) { vp_lineto(d, (int)SX(x), (int)SY(y)); }
#define vp_moveto(d, x, y) mv(d, (float)(x), (float)(y))
#define vp_lineto(d, x, y) ln(d, (float)(x), (float)(y))
static float smooth01(float t) { t = clampf(t, 0, 1); return t * t * (3 - 2 * t); }

/* a color turned round the color wheel by `turn` (0..1): its brightness
 * and how colorful it is stay */
static void hue_turn(float *r, float *g, float *b, float turn)
{
    float mx = fmaxf(*r, fmaxf(*g, *b)), mn = fminf(*r, fminf(*g, *b)), c = mx - mn;
    if (c < 1) return;                                  /* white, grey: no hue to turn */
    float h;
    if (mx == *r)      h = fmodf((*g - *b) / c + 6, 6);
    else if (mx == *g) h = (*b - *r) / c + 2;
    else               h = (*r - *g) / c + 4;
    h = fmodf(h + turn * 6, 6);
    float x = c * (1 - fabsf(fmodf(h, 2) - 1));
    float rr = 0, gg = 0, bb = 0;
    switch ((int)h) {
    case 0: rr = c; gg = x; break;
    case 1: rr = x; gg = c; break;
    case 2: gg = c; bb = x; break;
    case 3: gg = x; bb = c; break;
    case 4: rr = x; bb = c; break;
    default: rr = c; bb = x; break;
    }
    *r = rr + mn; *g = gg + mn; *b = bb + mn;
}

static void color(vp_dvg *d, float r, float g, float b, float k)
{
    k = clampf(k, 0, 1);
    if (PAINT == THEME_RANDOM_MULTI) {
        hue_turn(&r, &g, &b, PAINT_TURN);               /* the own colors, all turned alike */
    } else if (PAINT > 0) {
        /* one color for all: as bright as the face's own was */
        float lum = (0.3f * r + 0.59f * g + 0.11f * b) / 255, f = 0.45f + 0.55f * clampf(lum / 0.85f, 0, 1);
        if (PAINT == THEME_RAINBOW) {                      /* the rainbow: the hue wanders, slowly */
            float h = fmodf(G.time * 0.01f, 1) * 6, x = 1 - fabsf(fmodf(h, 2) - 1);
            float c[6][3] = { { 1, x, 0 }, { x, 1, 0 }, { 0, 1, x }, { 0, x, 1 }, { x, 0, 1 }, { 1, 0, x } };
            int i = (int)h % 6;
            r = (60 + 195 * c[i][0]) * f; g = (60 + 195 * c[i][1]) * f; b = (60 + 195 * c[i][2]) * f;
        } else {
            r = THEME_RGB[PAINT][0] * f; g = THEME_RGB[PAINT][1] * f; b = THEME_RGB[PAINT][2] * f;
        }
    }
    vp_color(d, (uint8_t)clampf(r * k, 0, 255), (uint8_t)clampf(g * k, 0, 255), (uint8_t)clampf(b * k, 0, 255));
}

static void line(vp_dvg *d, float x0, float y0, float x1, float y1)
{
    vp_moveto(d, (int)x0, (int)y0);
    vp_lineto(d, (int)x1, (int)y1);
}

/* an ellipse, or part of one (degrees, counterclockwise), as one stroke */
static void arc(vp_dvg *d, float cx, float cy, float rx, float ry, float a0, float a1, int n)
{
    for (int i = 0; i <= n; i++) {
        float a = (a0 + (a1 - a0) * i / n) * PI / 180;
        int x = (int)(cx + rx * cosf(a)), y = (int)(cy + ry * sinf(a));
        if (i == 0) vp_moveto(d, x, y);
        else        vp_lineto(d, x, y);
    }
}

static void circle(vp_dvg *d, float cx, float cy, float r, int n) { arc(d, cx, cy, r, r, 90, 450, n); }

static void text(vp_dvg *d, const char *s, float x, float y, float h, int align)
{
    vp_text_font(VP_FONT_ASTEROIDS);
    vp_text(d, s, x, y, h, align);
}

/* ---- the time --------------------------------------------------------------- */

/* the time here, or at the place the menu set (its offset from UTC comes
 * with its weather: until then, here) */
static now_t read_now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t t = (time_t)tv.tv_sec;
    struct tm *lt;
    if (SET.has_place && G.wx.ok && G.wx.serial == G.place_serial) {
        t += G.wx.utc_offset;
        lt = gmtime(&t);
    } else {
        lt = localtime(&t);
    }
    now_t n = { lt->tm_hour, lt->tm_min, lt->tm_sec, lt->tm_wday, lt->tm_mday, lt->tm_mon + 1,
                lt->tm_year + 1900, (float)tv.tv_usec / 1e6f, lt->tm_hour };
    if (!SET.h24) n.hd = n.h % 12 == 0 ? 12 : n.h % 12;
    return n;
}

static void digits6(const now_t *n, int out[6])
{
    out[0] = n->hd / 10; out[1] = n->hd % 10;
    out[2] = n->m / 10; out[3] = n->m % 10;
    out[4] = n->s / 10; out[5] = n->s % 10;
}

/* the date in the language: long ("SATURDAY 7 NOV 2026") or short ("SAT 7 NOV") */
static void date_text(char *buf, size_t size, const now_t *n, int longer)
{
    int l = SET.lang;
    if (l == LANG_DE) {
        if (longer) snprintf(buf, size, "%s  %02d.%02d.%04d", L_WEEKDAY[l][n->wday], n->mday, n->mon, n->year);
        else        snprintf(buf, size, "%s %02d.%02d.", L_WEEKDAY_SHORT[l][n->wday], n->mday, n->mon);
    } else {
        if (longer) snprintf(buf, size, "%s  %d %s %04d", L_WEEKDAY[l][n->wday], n->mday, L_MONTH[l][n->mon - 1], n->year);
        else        snprintf(buf, size, "%s %d %s", L_WEEKDAY_SHORT[l][n->wday], n->mday, L_MONTH[l][n->mon - 1]);
    }
}

static const char *S(int i) { return L_STR[SET.lang][i]; }

/* ---- digits of our own (they fold and squash: the fonts cannot) ------------- */

typedef struct { int n; float x[64], y[64]; } stroke;
static stroke GLYPH[10][2];
static int GLYPH_N[10];

static void g_pt(stroke *s, float x, float y) { if (s->n < 64) { s->x[s->n] = x; s->y[s->n] = y; s->n++; } }
static void g_arc(stroke *s, float cx, float cy, float rx, float ry, float a0, float a1, int n)
{
    for (int i = 0; i <= n; i++) {
        float a = (a0 + (a1 - a0) * i / n) * PI / 180;
        g_pt(s, cx + rx * cosf(a), cy + ry * sinf(a));
    }
}

/* in a box 0.6 wide, 1 high */
static void glyphs_make(void)
{
    memset(GLYPH, 0, sizeof GLYPH);
    stroke *s;
    s = &GLYPH[0][0]; g_arc(s, 0.3f, 0.5f, 0.3f, 0.5f, 90, 450, 32); GLYPH_N[0] = 1;
    s = &GLYPH[1][0]; g_pt(s, 0.1f, 0.78f); g_pt(s, 0.36f, 1); g_pt(s, 0.36f, 0); GLYPH_N[1] = 1;
    s = &GLYPH[2][0]; g_arc(s, 0.3f, 0.71f, 0.28f, 0.29f, 160, -40, 14); g_pt(s, 0.02f, 0); g_pt(s, 0.6f, 0);
    GLYPH_N[2] = 1;
    s = &GLYPH[3][0]; g_arc(s, 0.3f, 0.75f, 0.26f, 0.25f, 150, -90, 14);
    g_arc(s, 0.3f, 0.25f, 0.29f, 0.25f, 90, -150, 14); GLYPH_N[3] = 1;
    s = &GLYPH[4][0]; g_pt(s, 0.46f, 0); g_pt(s, 0.46f, 1); g_pt(s, 0.0f, 0.32f); g_pt(s, 0.6f, 0.32f);
    GLYPH_N[4] = 1;
    s = &GLYPH[5][0]; g_pt(s, 0.56f, 1); g_pt(s, 0.1f, 1); g_pt(s, 0.07f, 0.57f);
    g_arc(s, 0.3f, 0.32f, 0.29f, 0.32f, 125, -150, 16); GLYPH_N[5] = 1;
    s = &GLYPH[6][0]; g_pt(s, 0.5f, 0.98f); g_arc(s, 0.3f, 0.3f, 0.29f, 0.3f, 160, 160 + 360, 28); GLYPH_N[6] = 1;
    s = &GLYPH[7][0]; g_pt(s, 0, 1); g_pt(s, 0.6f, 1); g_pt(s, 0.18f, 0); GLYPH_N[7] = 1;
    s = &GLYPH[8][0]; g_arc(s, 0.3f, 0.77f, 0.22f, 0.23f, 270, 270 + 360, 24);
    s = &GLYPH[8][1]; g_arc(s, 0.3f, 0.27f, 0.29f, 0.27f, 90, 90 + 360, 28); GLYPH_N[8] = 2;
    s = &GLYPH[9][0]; g_arc(s, 0.3f, 0.7f, 0.29f, 0.3f, -20, -20 + 360, 28); g_pt(s, 0.12f, 0.02f); GLYPH_N[9] = 1;
}

/* part of a digit, between glyph heights ylo and yhi, `w` x `h` at (ox, oy);
 * squashed by sq towards the height hg (a card folding there) */
static void glyph_part(vp_dvg *d, int g, float ox, float oy, float w, float h, float ylo, float yhi, float hg, float sq)
{
    for (int k = 0; k < GLYPH_N[g]; k++) {
        const stroke *s = &GLYPH[g][k];
        for (int i = 0; i + 1 < s->n; i++) {
            float x0 = s->x[i], y0 = s->y[i], x1 = s->x[i + 1], y1 = s->y[i + 1];
            /* cut to ylo..yhi */
            if ((y0 < ylo && y1 < ylo) || (y0 > yhi && y1 > yhi)) continue;
            float t0 = 0, t1 = 1, dy = y1 - y0;
            if (dy != 0) {
                float a = (ylo - y0) / dy, b = (yhi - y0) / dy;
                if (a > b) { float t = a; a = b; b = t; }
                t0 = fmaxf(t0, a);
                t1 = fminf(t1, b);
                if (t0 >= t1) continue;
            }
            float ax = x0 + (x1 - x0) * t0, ay = y0 + dy * t0, bx = x0 + (x1 - x0) * t1, by = y0 + dy * t1;
            line(d, ox + ax / 0.6f * w, oy + (hg + (ay - hg) * sq) * h, ox + bx / 0.6f * w, oy + (hg + (by - hg) * sq) * h);
        }
    }
}

static void glyph(vp_dvg *d, int g, float ox, float oy, float w, float h)
{
    glyph_part(d, g, ox, oy, w, h, -1, 2, 0, 1);
}

/* ---- the faces ------------------------------------------------------------ */

static float accent(void) { return 0.72f + 0.28f * G.loud; }   /* the music, slowly */

/* a watch dial */
/* where the second hand stands: gliding (SECONDS SMOOTH), or a quick step
 * at each second (TICK: there in an eighth of a second, eased) */
static float sec_hand(const now_t *n)
{
    if (SET.seconds != SEC_TICK) return n->s + n->frac;
    float t = fminf(1, n->frac / 0.125f);
    return n->s - 1 + t * t * (3 - 2 * t);
}

static void face_analog(vp_dvg *d, float k)
{
    const now_t *n = &G.now;
    float cx = CX, cy = 2150, R = 1430;                 /* well clear of the line below */
    color(d, 120, 200, 255, k * accent());
    circle(d, cx, cy, R, 96);
    circle(d, cx, cy, R * 1.04f, 96);
    for (int i = 0; i < 60; i++) {                     /* the marks */
        float a = PI / 2 - i * 2 * PI / 60, c = cosf(a), s = sinf(a);
        float in = i % 5 == 0 ? R * 0.86f : R * 0.94f;
        if (i % 5 == 0) color(d, 235, 240, 255, k);
        else            color(d, 120, 200, 255, k * 0.6f * accent());
        line(d, cx + c * in, cy + s * in, cx + c * R * 0.985f, cy + s * R * 0.985f);
    }
    color(d, 255, 200, 110, k);                         /* the numbers: amber */
    const int NUM[4] = { 12, 3, 6, 9 };
    for (int i = 0; i < 4; i++) {
        float a = PI / 2 - i * PI / 2, r = R * 0.7f, h = 210;
        float x = cx + cosf(a) * r, y = cy + sinf(a) * r - h / 2;
        if (NUM[i] >= 10) {
            glyph(d, 1, x - h * 0.62f, y, h * 0.6f, h);
            glyph(d, 2, x + h * 0.04f, y, h * 0.6f, h);
        } else {
            glyph(d, NUM[i], x - h * 0.3f, y, h * 0.6f, h);
        }
    }
    color(d, 120, 200, 255, k * 0.8f);
    text(d, "VECTORPIE", cx, cy + R * 0.36f, 85, VP_TEXT_CENTER);
    if (SET.date) {                                     /* the date window at three */
        PAINT = theme(SET.date_theme, PART_DATE);
        char buf[8];
        snprintf(buf, sizeof buf, "%02d", n->mday);
        float wx = cx + R * 0.42f, wy = cy;
        color(d, 120, 200, 255, k * 0.7f);
        const float bw = 190, bh = 115;                 /* the window: room round the number */
        line(d, wx - bw, wy - bh, wx + bw, wy - bh); vp_lineto(d, (int)(wx + bw), (int)(wy + bh));
        vp_lineto(d, (int)(wx - bw), (int)(wy + bh)); vp_lineto(d, (int)(wx - bw), (int)(wy - bh));
        color(d, 255, 200, 110, k);
        text(d, buf, SX(wx), wy - 55, 110, VP_TEXT_CENTER);
        PAINT = theme(SET.theme, PART_CLOCK);
    }
    /* the hands, sweeping */
    float sec = sec_hand(n), min = n->m + sec / 60, hour = (n->h % 12) + min / 60;
    struct { float a, len, wid, r, g, b; } hand[2] = {
        { hour / 12 * 2 * PI, R * 0.55f, 70, 235, 240, 255 },
        { min / 60 * 2 * PI, R * 0.84f, 46, 110, 230, 255 },
    };
    for (int i = 0; i < 2; i++) {
        float a = PI / 2 - hand[i].a, c = cosf(a), s = sinf(a), px = -s, py = c, w = hand[i].wid;
        color(d, hand[i].r, hand[i].g, hand[i].b, k);
        vp_moveto(d, (int)(cx - c * w * 2), (int)(cy - s * w * 2));
        vp_lineto(d, (int)(cx + px * w), (int)(cy + py * w));
        vp_lineto(d, (int)(cx + c * hand[i].len), (int)(cy + s * hand[i].len));
        vp_lineto(d, (int)(cx - px * w), (int)(cy - py * w));
        vp_lineto(d, (int)(cx - c * w * 2), (int)(cy - s * w * 2));
    }
    float a = PI / 2 - sec / 60 * 2 * PI, c = cosf(a), s = sinf(a);
    color(d, 255, 90, 150, k);
    if (SET.seconds) {
        line(d, cx - c * R * 0.18f, cy - s * R * 0.18f, cx + c * R * 0.92f, cy + s * R * 0.92f);
        circle(d, cx - c * R * 0.12f, cy - s * R * 0.12f, 40, 12);
    }
    circle(d, cx, cy, 34, 12);
}

/* ---- more analog dials: one set of hands, many dials ---------------------
 * MINIMAL (no rim, no numbers), NUMERALS (only the numbers), ROMAN, SQUARE
 * and FULL SCREEN (the marks along the screen's edges). A dial is a circle
 * (radius r) or a rectangle (half sizes rw, rh); the hands reach as far as
 * its edge lets them. */

/* a dial's colors: rim, hour and minute marks, numbers, the name, hour,
 * minute and second hands */
typedef struct { float rim[3], mh[3], mm[3], num[3], brand[3], hh[3], hm[3], sec[3]; } dial_pal;
static const dial_pal PAL_MINIMAL = { { 0 }, { 255, 150, 60 }, { 255, 225, 190 }, { 0 }, { 0 },
                                      { 255, 235, 215 }, { 255, 160, 70 }, { 255, 70, 90 } };
static const dial_pal PAL_NUMERALS = { { 0 }, { 0 }, { 0 }, { 90, 255, 190 }, { 60, 235, 255 },
                                       { 235, 240, 255 }, { 90, 255, 190 }, { 255, 90, 200 } };
static const dial_pal PAL_ROMAN = { { 255, 200, 90 }, { 255, 220, 150 }, { 200, 160, 80 }, { 255, 240, 210 },
                                    { 255, 200, 90 }, { 255, 240, 210 }, { 255, 200, 90 }, { 100, 160, 255 } };
static const dial_pal PAL_SQUARE = { { 255, 90, 220 }, { 235, 240, 255 }, { 255, 90, 220 }, { 190, 150, 255 },
                                     { 255, 90, 220 }, { 235, 240, 255 }, { 190, 150, 255 }, { 255, 230, 60 } };
static const dial_pal PAL_FULL = { { 0 }, { 60, 235, 255 }, { 90, 130, 255 }, { 235, 240, 255 },
                                   { 60, 235, 255 }, { 235, 240, 255 }, { 60, 235, 255 }, { 255, 140, 40 } };
static const dial_pal *DP = &PAL_FULL;  /* the dial drawing now */
static void pc(vp_dvg *d, const float c[3], float k) { color(d, c[0], c[1], c[2], k); }

static const char *const ROMAN[13] = { "", "I", "II", "III", "IIII", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII" };

/* how far the dial's edge is from its middle, at angle a */
static float dial_reach(float a, float r, float rw, float rh)
{
    if (rw <= 0) return r;
    float c = fabsf(cosf(a)), s = fabsf(sinf(a));
    return fminf(c > 1e-4f ? rw / c : 1e9f, s > 1e-4f ? rh / s : 1e9f);
}

/* a number on the dial, its middle at x, y */
static void dial_number(vp_dvg *d, int n, float x, float y, float h)
{
    y -= h / 2;
    if (n >= 10) {
        glyph(d, 1, x - h * 0.62f, y, h * 0.6f, h);
        glyph(d, n - 10, x + h * 0.04f, y, h * 0.6f, h);
    } else {
        glyph(d, n, x - h * 0.3f, y, h * 0.6f, h);
    }
}

/* the hands' shapes: each dial its own */
enum { HAND_STICK, HAND_BATON, HAND_BREGUET, HAND_ARROW, HAND_TAPER };

/* one hand from (cx, cy) at angle a, len long, w its width */
static void hand_shape(vp_dvg *d, float cx, float cy, float a, float len, float w, int style)
{
    float c = cosf(a), s = sinf(a);
    switch (style) {
    case HAND_STICK:                                    /* a thin bar */
    case HAND_BATON: {                                  /* a broad bar with a line inside */
        float q = style == HAND_STICK ? w * 0.22f : w * 0.6f, back = len * (style == HAND_STICK ? 0.1f : 0.16f);
        float x, y;
        x = cx + c * -back - s * q; y = cy + s * -back + c * q; vp_moveto(d, (int)x, (int)y);
        x = cx + c * len - s * q;   y = cy + s * len + c * q;   vp_lineto(d, (int)x, (int)y);
        x = cx + c * len + s * q;   y = cy + s * len - c * q;   vp_lineto(d, (int)x, (int)y);
        x = cx + c * -back + s * q; y = cy + s * -back - c * q; vp_lineto(d, (int)x, (int)y);
        x = cx + c * -back - s * q; y = cy + s * -back + c * q; vp_lineto(d, (int)x, (int)y);
        if (style == HAND_BATON) line(d, cx + c * len * 0.3f, cy + s * len * 0.3f, cx + c * len * 0.92f, cy + s * len * 0.92f);
        break;
    }
    case HAND_BREGUET: {                                /* thin, a ring before the tip */
        float rr = w * 1.5f, at = len * 0.72f;
        line(d, cx - c * len * 0.15f, cy - s * len * 0.15f, cx + c * (at - rr), cy + s * (at - rr));
        circle(d, cx + c * at, cy + s * at, rr, 14);
        vp_moveto(d, (int)(cx + c * (at + rr) - s * w * 0.35f), (int)(cy + s * (at + rr) + c * w * 0.35f));
        vp_lineto(d, (int)(cx + c * len), (int)(cy + s * len));
        vp_lineto(d, (int)(cx + c * (at + rr) + s * w * 0.35f), (int)(cy + s * (at + rr) - c * w * 0.35f));
        break;
    }
    case HAND_ARROW: {                                  /* a shaft and an open head */
        float head = len * 0.8f;
        line(d, cx - c * len * 0.12f, cy - s * len * 0.12f, cx + c * head, cy + s * head);
        vp_moveto(d, (int)(cx + c * head - s * w * 1.3f), (int)(cy + s * head + c * w * 1.3f));
        vp_lineto(d, (int)(cx + c * len), (int)(cy + s * len));
        vp_lineto(d, (int)(cx + c * head + s * w * 1.3f), (int)(cy + s * head - c * w * 1.3f));
        vp_lineto(d, (int)(cx + c * head - s * w * 1.3f), (int)(cy + s * head + c * w * 1.3f));
        break;
    }
    case HAND_TAPER:                                    /* broad at the middle, to a long point */
        vp_moveto(d, (int)(cx - s * w), (int)(cy + c * w));
        vp_lineto(d, (int)(cx + c * len), (int)(cy + s * len));
        vp_lineto(d, (int)(cx + s * w), (int)(cy - c * w));
        vp_lineto(d, (int)(cx - s * w), (int)(cy + c * w));
        break;
    }
}

/* the hands, in the dial's own shape; long as the dial's edge lets them */
static void dial_hands(vp_dvg *d, float cx, float cy, float r, float rw, float rh, int style, float k)
{
    const now_t *n = &G.now;
    float base = rw > 0 ? fminf(rw, rh) : r, f = base / 1430;
    float sec = sec_hand(n), min = n->m + sec / 60, hour = (n->h % 12) + min / 60;
    struct { float a, len, wid; } hand[2] = {
        { PI / 2 - hour / 12 * 2 * PI, 0.55f, 70 * f },
        { PI / 2 - min / 60 * 2 * PI, 0.84f, 46 * f },
    };
    for (int i = 0; i < 2; i++) {
        float a = hand[i].a;
        float len = (i == 0 ? fminf(dial_reach(a, r, rw, rh), base * 1.15f) : dial_reach(a, r, rw, rh)) * hand[i].len;
        pc(d, i == 0 ? DP->hh : DP->hm, k);
        hand_shape(d, cx, cy, a, len, hand[i].wid, style);
    }
    float a = PI / 2 - sec / 60 * 2 * PI, c = cosf(a), s = sinf(a), len = dial_reach(a, r, rw, rh) * 0.92f;
    pc(d, DP->sec, k);
    if (SET.seconds) {
        line(d, cx - c * base * 0.18f, cy - s * base * 0.18f, cx + c * len, cy + s * len);
        if (style == HAND_BREGUET || style == HAND_ARROW)   /* a counterweight */
            circle(d, cx - c * base * 0.12f, cy - s * base * 0.12f, 40 * f, 12);
    }
    circle(d, cx, cy, (style == HAND_STICK ? 22 : 34) * f, 12);
}

/* the 60 marks, inward from the dial's edge: hours longer and brighter */
static void dial_marks(vp_dvg *d, float cx, float cy, float r, float rw, float rh, float hour_len, float min_len, float k)
{
    for (int i = 0; i < 60; i++) {
        float a = PI / 2 - i * 2 * PI / 60, c = cosf(a), s = sinf(a), e = dial_reach(a, r, rw, rh) * 0.985f;
        float in = e - (i % 5 == 0 ? hour_len : min_len);
        if (i % 5 == 0) pc(d, DP->mh, k);
        else            pc(d, DP->mm, k * 0.6f * accent());
        line(d, cx + c * in, cy + s * in, cx + c * e, cy + s * e);
    }
}

/* the twelve numbers, `inset` in from the dial's edge; roman or not */
static void dial_numbers(vp_dvg *d, float cx, float cy, float r, float rw, float rh, float inset, float h, int roman, float k)
{
    pc(d, DP->num, k);
    for (int i = 1; i <= 12; i++) {
        float a = PI / 2 - i * 2 * PI / 12, e = rw > 0 ? dial_reach(a, r, rw - inset, rh - inset) : r - inset;   /* square in from the edge */
        float x = cx + cosf(a) * e, y = cy + sinf(a) * e;
        if (roman) text(d, ROMAN[i], SX(x), y - h / 2, h, VP_TEXT_CENTER);
        else       dial_number(d, i, x, y, h);
    }
}

static void dial_brand(vp_dvg *d, float cx, float cy, float off, float k)
{
    pc(d, DP->brand, k * 0.8f);
    text(d, "VECTORPIE", SX(cx), cy + off, 85, VP_TEXT_CENTER);
}

/* MINIMAL: no rim, no numbers: twelve bars and the hands */
static void face_minimal(vp_dvg *d, float k)
{
    const float cx = CX, cy = 2150, r = 1430;
    DP = &PAL_MINIMAL;
    for (int i = 0; i < 12; i++) {
        float a = PI / 2 - i * 2 * PI / 12, c = cosf(a), s = sinf(a), px = -s * 22, py = c * 22;
        float in = r * (i % 3 == 0 ? 0.74f : 0.84f), out = r;
        if (i % 3 == 0) pc(d, DP->mh, k);                /* 12, 3, 6, 9 */
        else            pc(d, DP->mm, k * 0.8f);
        vp_moveto(d, (int)(cx + c * in + px), (int)(cy + s * in + py));     /* a bar: a thin outline */
        vp_lineto(d, (int)(cx + c * out + px), (int)(cy + s * out + py));
        vp_lineto(d, (int)(cx + c * out - px), (int)(cy + s * out - py));
        vp_lineto(d, (int)(cx + c * in - px), (int)(cy + s * in - py));
        vp_lineto(d, (int)(cx + c * in + px), (int)(cy + s * in + py));
    }
    dial_hands(d, cx, cy, r, 0, 0, HAND_STICK, k);
}

/* NUMERALS: no rim, no marks: the twelve numbers */
static void face_numerals(vp_dvg *d, float k)
{
    const float cx = CX, cy = 2150, r = 1430;
    DP = &PAL_NUMERALS;
    dial_numbers(d, cx, cy, r, 0, 0, 170, 240, 0, k);
    dial_brand(d, cx, cy, r * 0.36f, k);
    dial_hands(d, cx, cy, r, 0, 0, HAND_BATON, k);
}

/* ROMAN: a thin double rim, roman numerals */
static void face_roman(vp_dvg *d, float k)
{
    const float cx = CX, cy = 2150, r = 1430;
    DP = &PAL_ROMAN;
    pc(d, DP->rim, k * accent());
    circle(d, cx, cy, r, 96);
    circle(d, cx, cy, r * 0.97f, 96);
    dial_marks(d, cx, cy, r * 0.97f, 0, 0, 60, 35, k * 0.8f);
    dial_numbers(d, cx, cy, r, 0, 0, 340, 100, 1, k);
    dial_brand(d, cx, cy, r * 0.36f, k);
    dial_hands(d, cx, cy, r, 0, 0, HAND_BREGUET, k);
}

/* SQUARE: a square dial, the marks and numbers along its sides */
static void face_square(vp_dvg *d, float k)
{
    const float cx = CX, cy = 2150, s = 1400;
    DP = &PAL_SQUARE;
    pc(d, DP->rim, k * accent());
    for (int j = 0; j < 2; j++) {
        float e = j == 0 ? s * 1.04f : s;
        vp_moveto(d, (int)(cx - e), (int)(cy - e));
        vp_lineto(d, (int)(cx + e), (int)(cy - e));
        vp_lineto(d, (int)(cx + e), (int)(cy + e));
        vp_lineto(d, (int)(cx - e), (int)(cy + e));
        vp_lineto(d, (int)(cx - e), (int)(cy - e));
    }
    dial_marks(d, cx, cy, 0, s, s, 160, 70, k);
    dial_numbers(d, cx, cy, 0, s, s, 330, 200, 0, k);
    dial_brand(d, cx, cy, s * 0.36f, k);
    dial_hands(d, cx, cy, 0, s, s, HAND_ARROW, k);
}

/* FULL SCREEN: no frame; the marks along the screen's edges, the hands
 * reaching out to them */
static void face_full(vp_dvg *d, float k)
{
    /* the screen in true shape (before the squeeze), above the line under the faces */
    float left = CX - CX / ASPECT_X + 130, right = CX + CX / ASPECT_X - 130;
    float bottom = W / 2.0f + (620 - W / 2.0f) / ASPECT_Y, top = W / 2.0f + (3975 - W / 2.0f) / ASPECT_Y;
    float cx = (left + right) / 2, cy = (bottom + top) / 2, rw = (right - left) / 2, rh = (top - bottom) / 2;
    DP = &PAL_FULL;
    dial_marks(d, cx, cy, 0, rw, rh, 260, 110, k);
    dial_numbers(d, cx, cy, 0, rw, rh, 470, 250, 0, k);
    dial_brand(d, cx, cy, rh * 0.4f, k);
    dial_hands(d, cx, cy, 0, rw, rh, HAND_TAPER, k);
}

/* seven segments: a, b, c, d, e, f, g; on: lit, else faintly there */
static const unsigned char SEG7[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };

static void seg7(vp_dvg *d, int v, float x, float y, float w, float h, float k, float r, float g, float b)
{
    /* the corners, slanted */
    const float sl = 0.1f * h, gap = 0.06f * h;
    float X[6], Y[6];
    for (int i = 0; i < 6; i++) {
        float fy = i / 2 * 0.5f;                     /* 0, 0, .5, .5, 1, 1 */
        float fx = i & 1 ? 1 : 0;
        Y[i] = y + fy * h;
        X[i] = x + fx * w + fy * sl;
    }
    /* each segment between two corners: a 4-5, b 3-5, c 1-3, d 0-1, e 0-2, f 2-4, g 2-3 */
    const int A[7] = { 4, 3, 1, 0, 0, 2, 2 }, B[7] = { 5, 5, 3, 1, 2, 4, 3 };
    for (int sgm = 0; sgm < 7; sgm++) {
        int on = (SEG7[v] >> sgm) & 1;
        float x0 = X[A[sgm]], y0 = Y[A[sgm]], x1 = X[B[sgm]], y1 = Y[B[sgm]];
        float dx = x1 - x0, dy = y1 - y0, l = sqrtf(dx * dx + dy * dy), ux = dx / l, uy = dy / l;
        x0 += ux * gap; y0 += uy * gap; x1 -= ux * gap; y1 -= uy * gap;
        if (on) {
            float t = w * 0.07f;                     /* lit: the segment's outline, pointed ends */
            color(d, r, g, b, k);
            vp_moveto(d, (int)x0, (int)y0);
            vp_lineto(d, (int)(x0 + ux * t - uy * t), (int)(y0 + uy * t + ux * t));
            vp_lineto(d, (int)(x1 - ux * t - uy * t), (int)(y1 - uy * t + ux * t));
            vp_lineto(d, (int)x1, (int)y1);
            vp_lineto(d, (int)(x1 - ux * t + uy * t), (int)(y1 - uy * t - ux * t));
            vp_lineto(d, (int)(x0 + ux * t + uy * t), (int)(y0 + uy * t - ux * t));
            vp_lineto(d, (int)x0, (int)y0);
        } else {
            color(d, r, g, b, k * 0.06f);
            line(d, x0, y0, x1, y1);
        }
    }
}

static void face_digital(vp_dvg *d, float k)
{
    const now_t *n = &G.now;
    int dg[6];
    digits6(n, dg);
    const float h = 1200, w = 500, y = 1900, sp = 200;
    float x = CX - (4 * w + 3 * sp + 260 + (SET.seconds ? 60 + 2 * w * 0.38f + 90 : -sp)) / 2;
    for (int i = 0; i < 4; i++) {
        if (i < 2) seg7(d, dg[i], x, y, w, h, k, 60, 235, 255);     /* hours: cyan */
        else       seg7(d, dg[i], x, y, w, h, k, 255, 80, 220);     /* minutes: magenta */
        x += w + sp;
        if (i == 1) {                                /* the colon, standing still */
            color(d, 220, 230, 255, k);
            for (int j = 0; j < 2; j++) {
                float cy = y + h * (j ? 0.68f : 0.32f), cx = x + 40 + (cy - y) * 0.1f;
                circle(d, cx, cy, 38, 10);
                circle(d, cx, cy, 18, 8);
            }
            x += 260;
        }
    }
    if (SET.seconds) {
        seg7(d, dg[4], x - 40, y, w * 0.38f, h * 0.38f, k * 0.85f, 255, 220, 60);            /* seconds: yellow */
        seg7(d, dg[5], x - 40 + w * 0.38f + 90, y, w * 0.38f, h * 0.38f, k * 0.85f, 255, 220, 60);
    }
    if (SET.date) {
        char buf[64];
        date_text(buf, sizeof buf, n, 1);
        PAINT = theme(SET.date_theme, PART_DATE);
        color(d, 60, 235, 255, k * 0.75f * accent());
        vp_text_font(VP_FONT_ASTEROIDS);
        vp_text_fit(d, buf, CX, 1300, 150, W * 0.8f, VP_TEXT_CENTER);
        PAINT = theme(SET.theme, PART_CLOCK);
    }
}

/* a card of the flip clock: its digit, folding from the old one */
static void flip_card(vp_dvg *d, int i, float x, float y, float cw, float ch, float k)
{
    float pad = ch * 0.14f, gh = ch - 2 * pad, gw = gh * 0.6f, gx = x + (cw - gw) / 2, gy = y + pad;
    float hinge = y + ch / 2;
    int cur = G.flip_cur[i], old = G.flip_old[i];
    float t = G.flip_t[i];
    /* the card, its split */
    color(d, 200, 205, 215, k * 0.55f * accent());
    float r = cw * 0.08f;
    vp_moveto(d, (int)(x + r), (int)y);
    vp_lineto(d, (int)(x + cw - r), (int)y);
    arc(d, x + cw - r, y + r, r, r, -90, 0, 4);
    vp_lineto(d, (int)(x + cw), (int)(y + ch - r));
    arc(d, x + cw - r, y + ch - r, r, r, 0, 90, 4);
    vp_lineto(d, (int)(x + r), (int)(y + ch));
    arc(d, x + r, y + ch - r, r, r, 90, 180, 4);
    vp_lineto(d, (int)x, (int)(y + r));
    arc(d, x + r, y + r, r, r, 180, 270, 4);
    color(d, 200, 205, 215, k * 0.3f);
    line(d, x + 6, hinge, x + cw - 6, hinge);
    static const float DIGIT[3][3] = { { 255, 190, 90 }, { 245, 245, 250 }, { 60, 235, 255 } };
    color(d, DIGIT[i / 2][0], DIGIT[i / 2][1], DIGIT[i / 2][2], k);   /* hours, minutes, seconds */
    if (t >= 1) {
        glyph(d, cur, gx, gy, gw, gh);
        return;
    }
    if (t < 0.5f) {
        /* the top flap, the old digit's top, folds down over the new one */
        float c = cosf(t / 0.5f * PI / 2);
        glyph_part(d, cur, gx, gy, gw, gh, 0.5f + 0.5f * c, 1, 0, 1);      /* new top, above the flap */
        glyph_part(d, old, gx, gy, gw, gh, 0.5f, 1, 0.5f, c);              /* the flap */
        glyph_part(d, old, gx, gy, gw, gh, 0, 0.5f, 0, 1);                 /* old bottom */
        color(d, 200, 205, 215, k * 0.5f);
        line(d, x + 4, hinge + ch / 2 * c, x + cw - 4, hinge + ch / 2 * c);
    } else {
        /* the bottom flap, the new digit's bottom, comes down over the old one */
        float s = sinf((t - 0.5f) / 0.5f * PI / 2);
        glyph_part(d, cur, gx, gy, gw, gh, 0.5f, 1, 0, 1);                 /* new top */
        glyph_part(d, cur, gx, gy, gw, gh, 0, 0.5f, 0.5f, s);              /* the flap */
        glyph_part(d, old, gx, gy, gw, gh, 0, 0.5f - 0.5f * s, 0, 1);      /* old bottom, below it */
        color(d, 200, 205, 215, k * 0.5f);
        line(d, x + 4, hinge - ch / 2 * s, x + cw - 4, hinge - ch / 2 * s);
    }
}

static void face_flip(vp_dvg *d, float k)
{
    const float cw = 520, ch = 900, gap = 40, pair = 200;
    int cards = SET.seconds ? 6 : 4;
    float total = SET.seconds ? 4 * cw + 2 * 0.62f * cw + 3 * gap + 2 * pair : 4 * cw + 2 * gap + pair;
    float x = CX - total / 2, y = 1750;
    for (int i = 0; i < cards; i++) {
        float sc = i >= 4 ? 0.62f : 1;              /* the seconds smaller */
        flip_card(d, i, x, y, cw * sc, ch * sc, k);
        x += cw * sc + (i & 1 ? pair : gap);
    }
    if (SET.date) {
        char buf[64];
        date_text(buf, sizeof buf, &G.now, 0);
        PAINT = theme(SET.date_theme, PART_DATE);
        color(d, 200, 205, 215, k * 0.6f * accent());
        text(d, buf, CX, 1250, 120, VP_TEXT_CENTER);
        PAINT = theme(SET.theme, PART_CLOCK);
    }
}

/* bits: each digit a column, from the top 8 4 2 1 */
static void face_binary(vp_dvg *d, float k)
{
    const int NB[6] = { 2, 4, 3, 4, 3, 4 };
    const float step = 420, r = 140, pair = 260;
    int cols = SET.seconds ? 6 : 4;
    float total = (cols - 1) * step + (cols / 2 - 1) * pair, x0 = CX - total / 2 + 120, y0 = 1350;
    color(d, 80, 255, 140, k * 0.4f * accent());
    const char *VAL[4] = { "1", "2", "4", "8" };
    for (int b = 0; b < 4; b++) text(d, VAL[b], SX(x0 - 380), y0 + b * step - 45, 90, VP_TEXT_CENTER);
    float x = x0;
    static const float COL[3][3] = { { 80, 255, 140 }, { 60, 220, 255 }, { 255, 220, 60 } };   /* h, m, s */
    for (int c = 0; c < cols; c++) {
        const float *cc = COL[c / 2];
        for (int b = 0; b < NB[c]; b++) {
            float y = y0 + b * step, on = G.bits[c][b];
            color(d, cc[0], cc[1], cc[2], k * (0.25f + 0.2f * accent()));
            circle(d, x, y, r, 24);
            if (on > 0.02f) {                       /* lit: filled with rings */
                color(d, cc[0], cc[1], cc[2], k * on);
                circle(d, x, y, r * 0.75f, 20);
                circle(d, x, y, r * 0.5f, 16);
                circle(d, x, y, r * 0.25f, 10);
            }
        }
        x += step + (c & 1 ? pair : 0);
    }
    /* the time in figures, small, under it */
    char buf[16];
    if (SET.seconds) snprintf(buf, sizeof buf, "%02d:%02d:%02d", G.now.hd, G.now.m, G.now.s);
    else             snprintf(buf, sizeof buf, "%02d:%02d", G.now.hd, G.now.m);
    color(d, 200, 215, 240, k * 0.55f);
    text(d, buf, CX, 760, 130, VP_TEXT_CENTER);
}

/* the word clock, in the clock's language, to five minutes; the minutes
 * between as dots. Each language its own rows of words (glue: no space
 * before it); words_target lights the ones that say the time. */
typedef struct { const char *s; int row, glue; } wc_word;
static const wc_word WC_EN[] = {
    { "IT", 0, 0 }, { "IS", 0, 0 }, { "HALF", 0, 0 }, { "TEN", 0, 0 },
    { "QUARTER", 1, 0 }, { "TWENTY", 1, 0 },
    { "FIVE", 2, 0 }, { "TO", 2, 0 }, { "PAST", 2, 0 },
    { "ONE", 3, 0 }, { "TWO", 3, 0 }, { "THREE", 3, 0 },
    { "FOUR", 4, 0 }, { "FIVE", 4, 0 }, { "SIX", 4, 0 },
    { "SEVEN", 5, 0 }, { "EIGHT", 5, 0 }, { "NINE", 5, 0 },
    { "TEN", 6, 0 }, { "ELEVEN", 6, 0 }, { "TWELVE", 6, 0 },
    { "OCLOCK", 7, 0 },
};
static const wc_word WC_DE[] = {
    { "ES", 0, 0 }, { "IST", 0, 0 }, { "FUENF", 0, 0 }, { "ZEHN", 0, 0 },
    { "ZWANZIG", 1, 0 }, { "VIERTEL", 1, 0 },
    { "VOR", 2, 0 }, { "NACH", 2, 0 }, { "HALB", 2, 0 },
    { "EIN", 3, 0 }, { "S", 3, 1 }, { "ZWEI", 3, 0 }, { "DREI", 3, 0 },
    { "VIER", 4, 0 }, { "FUENF", 4, 0 }, { "SECHS", 4, 0 },
    { "SIEBEN", 5, 0 }, { "ACHT", 5, 0 }, { "NEUN", 5, 0 },
    { "ZEHN", 6, 0 }, { "ELF", 6, 0 }, { "ZWOELF", 6, 0 },
    { "UHR", 7, 0 },
};
static const wc_word WC_FR[] = {
    { "IL", 0, 0 }, { "EST", 0, 0 }, { "DEUX", 0, 0 }, { "QUATRE", 0, 0 },
    { "TROIS", 1, 0 }, { "NEUF", 1, 0 }, { "UNE", 1, 0 }, { "SEPT", 1, 0 },
    { "HUIT", 2, 0 }, { "SIX", 2, 0 }, { "CINQ", 2, 0 },
    { "MIDI", 3, 0 }, { "DIX", 3, 0 }, { "MINUIT", 3, 0 }, { "ONZE", 3, 0 },
    { "HEURE", 4, 0 }, { "S", 4, 1 },
    { "MOINS", 5, 0 }, { "LE", 5, 0 }, { "DIX", 5, 0 },
    { "ET", 6, 0 }, { "QUART", 6, 0 }, { "VINGT", 6, 0 }, { "-", 6, 1 }, { "CINQ", 6, 1 },
    { "ET", 7, 0 }, { "DEMIE", 7, 0 },
};
static const wc_word WC_ES[] = {
    { "ES", 0, 0 }, { "SON", 0, 0 }, { "LA", 0, 0 }, { "LAS", 0, 0 },
    { "UNA", 1, 0 }, { "DOS", 1, 0 }, { "TRES", 1, 0 },
    { "CUATRO", 2, 0 }, { "CINCO", 2, 0 }, { "SEIS", 2, 0 },
    { "SIETE", 3, 0 }, { "OCHO", 3, 0 }, { "NUEVE", 3, 0 },
    { "DIEZ", 4, 0 }, { "ONCE", 4, 0 }, { "DOCE", 4, 0 },
    { "Y", 5, 0 }, { "MENOS", 5, 0 },
    { "CINCO", 6, 0 }, { "DIEZ", 6, 0 }, { "CUARTO", 6, 0 },
    { "VEINTE", 7, 0 }, { "VEINTICINCO", 7, 0 }, { "MEDIA", 7, 0 },
};
#define NWORD 32
static const struct { const wc_word *w; int n; } WC[NLANG] = {
    { WC_EN, (int)(sizeof WC_EN / sizeof WC_EN[0]) }, { WC_DE, (int)(sizeof WC_DE / sizeof WC_DE[0]) },
    { WC_FR, (int)(sizeof WC_FR / sizeof WC_FR[0]) }, { WC_ES, (int)(sizeof WC_ES / sizeof WC_ES[0]) },
};

/* light the nth (from 0) word written so */
static void lit(float on[NWORD], int lang, const char *word, int nth)
{
    for (int i = 0; i < WC[lang].n; i++)
        if (!strcmp(WC[lang].w[i].s, word) && nth-- == 0) { on[i] = 1; return; }
}

static void words_target(int lang, const now_t *n, float on[NWORD])
{
    for (int i = 0; i < NWORD; i++) on[i] = 0;
    int five = n->m / 5;
    if (lang == LANG_EN) {
        static const char *HOUR[12] = { "TWELVE", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT",
                                        "NINE", "TEN", "ELEVEN" };
        int h = (n->h + (five >= 7)) % 12;
        lit(on, lang, "IT", 0); lit(on, lang, "IS", 0);
        static const char *MIN[12][3] = { { "OCLOCK" }, { "FIVE", "PAST" }, { "TEN", "PAST" }, { "QUARTER", "PAST" },
            { "TWENTY", "PAST" }, { "TWENTY", "FIVE", "PAST" }, { "HALF", "PAST" }, { "TWENTY", "FIVE", "TO" },
            { "TWENTY", "TO" }, { "QUARTER", "TO" }, { "TEN", "TO" }, { "FIVE", "TO" } };
        for (int i = 0; i < 3 && MIN[five][i]; i++) lit(on, lang, MIN[five][i], 0);
        lit(on, lang, HOUR[h], !strcmp(HOUR[h], "FIVE") || !strcmp(HOUR[h], "TEN"));
    } else if (lang == LANG_DE) {
        static const char *HOUR[12] = { "ZWOELF", "EIN", "ZWEI", "DREI", "VIER", "FUENF", "SECHS", "SIEBEN", "ACHT",
                                        "NEUN", "ZEHN", "ELF" };
        int h = (n->h + (five >= 5)) % 12;              /* from 25 past: towards the next hour */
        lit(on, lang, "ES", 0); lit(on, lang, "IST", 0);
        static const char *MIN[12][3] = { { "UHR" }, { "FUENF", "NACH" }, { "ZEHN", "NACH" }, { "VIERTEL", "NACH" },
            { "ZWANZIG", "NACH" }, { "FUENF", "VOR", "HALB" }, { "HALB" }, { "FUENF", "NACH", "HALB" },
            { "ZWANZIG", "VOR" }, { "VIERTEL", "VOR" }, { "ZEHN", "VOR" }, { "FUENF", "VOR" } };
        for (int i = 0; i < 3 && MIN[five][i]; i++) lit(on, lang, MIN[five][i], 0);
        lit(on, lang, HOUR[h], !strcmp(HOUR[h], "FUENF") || !strcmp(HOUR[h], "ZEHN"));
        if (h == 1 && five != 0) lit(on, lang, "S", 0);   /* EIN UHR, but FUENF NACH EINS */
    } else if (lang == LANG_FR) {
        static const char *HOUR[12] = { "", "UNE", "DEUX", "TROIS", "QUATRE", "CINQ", "SIX", "SEPT", "HUIT", "NEUF",
                                        "DIX", "ONZE" };
        int H = (n->h + (five >= 7)) % 24;
        lit(on, lang, "IL", 0); lit(on, lang, "EST", 0);
        if (H == 0) lit(on, lang, "MINUIT", 0);
        else if (H == 12) lit(on, lang, "MIDI", 0);
        else {
            lit(on, lang, HOUR[H % 12], 0);
            lit(on, lang, "HEURE", 0);
            if (H % 12 > 1) lit(on, lang, "S", 0);
        }
        switch (five) {
        case 1: lit(on, lang, "CINQ", 1); break;
        case 2: lit(on, lang, "DIX", 1); break;
        case 3: lit(on, lang, "ET", 0); lit(on, lang, "QUART", 0); break;
        case 4: lit(on, lang, "VINGT", 0); break;
        case 5: lit(on, lang, "VINGT", 0); lit(on, lang, "-", 0); lit(on, lang, "CINQ", 1); break;
        case 6: lit(on, lang, "ET", 1); lit(on, lang, "DEMIE", 0); break;
        case 7: lit(on, lang, "MOINS", 0); lit(on, lang, "VINGT", 0); lit(on, lang, "-", 0); lit(on, lang, "CINQ", 1); break;
        case 8: lit(on, lang, "MOINS", 0); lit(on, lang, "VINGT", 0); break;
        case 9: lit(on, lang, "MOINS", 0); lit(on, lang, "LE", 0); lit(on, lang, "QUART", 0); break;
        case 10: lit(on, lang, "MOINS", 0); lit(on, lang, "DIX", 1); break;
        case 11: lit(on, lang, "MOINS", 0); lit(on, lang, "CINQ", 1); break;
        }
    } else {
        static const char *HOUR[12] = { "DOCE", "UNA", "DOS", "TRES", "CUATRO", "CINCO", "SEIS", "SIETE", "OCHO",
                                        "NUEVE", "DIEZ", "ONCE" };
        int h = (n->h + (five >= 7)) % 12;
        if (h == 1) { lit(on, lang, "ES", 0); lit(on, lang, "LA", 0); }
        else        { lit(on, lang, "SON", 0); lit(on, lang, "LAS", 0); }
        lit(on, lang, HOUR[h], 0);
        static const char *MIN[12][2] = { { 0 }, { "Y", "CINCO" }, { "Y", "DIEZ" }, { "Y", "CUARTO" }, { "Y", "VEINTE" },
            { "Y", "VEINTICINCO" }, { "Y", "MEDIA" }, { "MENOS", "VEINTICINCO" }, { "MENOS", "VEINTE" },
            { "MENOS", "CUARTO" }, { "MENOS", "DIEZ" }, { "MENOS", "CINCO" } };
        for (int i = 0; i < 2 && MIN[five][i]; i++)
            lit(on, lang, MIN[five][i], !strcmp(MIN[five][i], "CINCO") || !strcmp(MIN[five][i], "DIEZ"));
    }
}

static void face_words(vp_dvg *d, float k)
{
    const float row_h = 400, top = 3500, space = 130;
    const wc_word *WD = WC[G.words_lang].w;
    int nw = WC[G.words_lang].n;
    vp_text_font(VP_FONT_ASTEROIDS);
    for (int row = 0; row < 8; row++) {
        float h = 230, width = 0;
        for (int i = 0; i < nw; i++)
            if (WD[i].row == row) width += vp_text_width(WD[i].s, h) + (WD[i].glue || width == 0 ? 0 : space);
        if (width > W * 0.92f) { h *= W * 0.92f / width; width = W * 0.92f; }   /* a long row: smaller */
        float x = CX - width / 2, y = top - row * row_h, sp = space * h / 230;
        int first = 1;
        for (int i = 0; i < nw; i++) {
            if (WD[i].row != row) continue;
            if (!first && !WD[i].glue) x += sp;
            first = 0;
            float on = G.words[i];
            static const float ROW[8][3] = { { 255, 190, 90 }, { 255, 160, 80 }, { 255, 130, 110 }, { 255, 110, 160 },
                                             { 240, 110, 220 }, { 190, 120, 255 }, { 130, 150, 255 }, { 90, 200, 255 } };
            color(d, ROW[row][0], ROW[row][1], ROW[row][2], k * (0.1f + 0.9f * on));
            vp_text(d, WD[i].s, x, y, h, VP_TEXT_LEFT);
            x += vp_text_width(WD[i].s, h);
        }
    }
    /* the minutes past the five: dots */
    int extra = G.now.m % 5;
    for (int i = 0; i < 4; i++) {
        float x = CX + (i - 1.5f) * 260, y = top - 8 * row_h + 260;
        color(d, 90, 220, 255, k * (i < extra ? 1 : 0.12f));
        circle(d, x, y, 45, 12);
        if (i < extra) circle(d, x, y, 20, 8);
    }
}

/* nixie tubes */
static void face_nixie(vp_dvg *d, float k)
{
    const float tw = 470, th = 1150, gap = 70, pair = 230;
    int tubes = SET.seconds ? 6 : 4;
    float total = tubes * tw + (tubes / 2) * gap + (tubes / 2 - 1) * pair, x = CX - total / 2, y = 1500;
    for (int i = 0; i < tubes; i++) {
        /* the glass: round at the top, the sockets' pins under it */
        float r = tw / 2;
        color(d, 140, 165, 200, k * 0.4f * accent());   /* the glass: cool */
        vp_moveto(d, (int)x, (int)y);
        vp_lineto(d, (int)x, (int)(y + th - r));
        arc(d, x + r, y + th - r, r, r, 180, 0, 16);
        vp_lineto(d, (int)(x + tw), (int)y);
        vp_lineto(d, (int)x, (int)y);
        for (int p = 0; p < 4; p++) line(d, x + tw * (0.2f + p * 0.2f), y - 10, x + tw * (0.2f + p * 0.2f), y - 90);
        /* the mesh in front, faint */
        color(d, 140, 165, 200, k * 0.15f);
        for (int m = 1; m < 6; m++) line(d, x + tw * 0.12f, y + th * 0.13f * m, x + tw * 0.88f, y + th * 0.13f * m);
        /* the digit, glowing; the old one fades as the new one lights */
        float gh = th * 0.62f, gw = gh * 0.6f, gx = x + (tw - gw) / 2, gy = y + th * 0.12f;
        float t = smooth01(G.nix_t[i]);
        if (t < 1) {
            color(d, 255, 130, 40, k * (1 - t));
            glyph(d, G.nix_old[i], gx, gy, gw, gh);
        }
        color(d, 255, 130, 40, k * t);
        glyph(d, G.nix_cur[i], gx, gy, gw, gh);
        color(d, 255, 200, 140, k * t * 0.5f);                      /* a hot core */
        glyph(d, G.nix_cur[i], gx + 6, gy + 6, gw, gh);
        x += tw;
        if ((i == 1 || i == 3) && i < tubes - 1) {                    /* the separators */
            float sx = x + pair / 2;
            color(d, 255, 130, 40, k * 0.8f);
            circle(d, sx, y + th * 0.33f, 30, 10);
            circle(d, sx, y + th * 0.6f, 30, 10);
            x += pair;
        } else if (i < 5) x += gap;
    }
    if (SET.date) {
        char buf[64];
        date_text(buf, sizeof buf, &G.now, 1);
        PAINT = theme(SET.date_theme, PART_DATE);
        color(d, 255, 130, 40, k * 0.45f * accent());
        text(d, buf, CX, 1000, 120, VP_TEXT_CENTER);
        PAINT = theme(SET.theme, PART_CLOCK);
    }
}

/* hours, minutes, seconds as planets on their rings */
static void face_orbit(vp_dvg *d, float k)
{
    const now_t *n = &G.now;
    float cx = CX, cy = 2030;
    float sec = sec_hand(n), min = n->m + sec / 60, hour = (n->h % 12) + min / 60;
    struct { float r, a, pr, cr, cg, cb; int ticks; } ring[3] = {
        { 650, hour / 12, 95, 255, 120, 220, 12 },
        { 1080, min / 60, 70, 120, 160, 255, 12 },
        { 1480, sec / 60, 45, 120, 230, 255, 60 },
    };
    for (int i = 0; i < (SET.seconds ? 3 : 2); i++) {
        float r = ring[i].r, a = PI / 2 - ring[i].a * 2 * PI;
        color(d, ring[i].cr, ring[i].cg, ring[i].cb, k * 0.35f * accent());
        circle(d, cx, cy, r, 96);
        for (int t = 0; t < ring[i].ticks; t++) {
            float ta = PI / 2 - t * 2 * PI / ring[i].ticks, len = t % (ring[i].ticks / 12 ? ring[i].ticks / 12 : 1) == 0 ? 40 : 18;
            line(d, cx + cosf(ta) * (r - len), cy + sinf(ta) * (r - len), cx + cosf(ta) * (r + len), cy + sinf(ta) * (r + len));
        }
        /* the planet, with a short trail behind it */
        float px = cx + cosf(a) * r, py = cy + sinf(a) * r;
        color(d, ring[i].cr, ring[i].cg, ring[i].cb, k);
        circle(d, px, py, ring[i].pr, 20);
        circle(d, px, py, ring[i].pr * 0.5f, 12);
        color(d, ring[i].cr, ring[i].cg, ring[i].cb, k * 0.6f);
        arc(d, cx, cy, r, r, a * 180 / PI + 4, a * 180 / PI + 30, 10);
    }
    /* the sun: the time */
    char buf[8];
    snprintf(buf, sizeof buf, "%02d:%02d", n->hd, n->m);
    color(d, 255, 215, 140, k);
    circle(d, cx, cy, 300, 32);
    text(d, buf, cx, cy - 55, 110, VP_TEXT_CENTER);
}

/* ---- TUNNEL: a web of lanes down a tube; the hour's lane lit, the minute
 * a marker on the rim, the seconds a claw running round it ------------------ */

#define LANES 12

static void face_tunnel(vp_dvg *d, float k)
{
    const now_t *n = &G.now;
    const float cx = CX, cy = 2030, R = 1560, r = 230;
    float sec = sec_hand(n), min = n->m + sec / 60, hour = (n->h % 12) + min / 60;
    float vx[5][LANES], vy[5][LANES];
    for (int ring = 0; ring < 5; ring++) {
        float t = ring / 4.0f, rr = r + (R - r) * t * t;         /* far rings closer together */
        for (int i = 0; i < LANES; i++) {
            float a = PI / 2 - (i - 0.5f) * 2 * PI / LANES;      /* lane h lies round the hour h */
            vx[ring][i] = cx + cosf(a) * rr;
            vy[ring][i] = cy + sinf(a) * rr;
        }
    }
    /* the web: the rings, the spokes */
    for (int ring = 0; ring < 5; ring++) {
        color(d, 70, 110, 255, k * (ring == 4 || ring == 0 ? 0.9f : 0.35f) * accent());
        vp_moveto(d, (int)vx[ring][0], (int)vy[ring][0]);
        for (int i = 1; i <= LANES; i++) vp_lineto(d, (int)vx[ring][i % LANES], (int)vy[ring][i % LANES]);
    }
    color(d, 70, 110, 255, k * 0.75f * accent());
    for (int i = 0; i < LANES; i++) line(d, vx[0][i], vy[0][i], vx[4][i], vy[4][i]);
    /* the hour's lane, lit: its edges and a line down its middle */
    int h = ((int)hour) % LANES;
    color(d, 255, 230, 60, k);
    line(d, vx[0][h], vy[0][h], vx[4][h], vy[4][h]);
    int h1 = (h + 1) % LANES;
    line(d, vx[4][h1], vy[4][h1], vx[0][h1], vy[0][h1]);
    for (int ring = 0; ring < 5; ring++) line(d, vx[ring][h], vy[ring][h], vx[ring][h1], vy[ring][h1]);
    /* the minute: a red marker on the rim */
    float am = PI / 2 - min / 60 * 2 * PI, rm = R * 1.07f;
    float mx = cx + cosf(am) * rm, my = cy + sinf(am) * rm, ux = cosf(am), uy = sinf(am);
    color(d, 255, 70, 70, k);
    vp_moveto(d, (int)(mx + ux * 90), (int)(my + uy * 90));
    vp_lineto(d, (int)(mx - uy * 60), (int)(my + ux * 60));
    vp_lineto(d, (int)(mx - ux * 90), (int)(my - uy * 90));
    vp_lineto(d, (int)(mx + uy * 60), (int)(my - ux * 60));
    vp_lineto(d, (int)(mx + ux * 90), (int)(my + uy * 90));
    /* the seconds: a claw running round the rim */
    float as = PI / 2 - sec / 60 * 2 * PI;
    float sx = cx + cosf(as) * R, sy = cy + sinf(as) * R, nx = cosf(as), ny = sinf(as), tx = -ny, ty = nx;
    const float CL[7][2] = { { -1, 0.15f }, { -0.55f, -0.35f }, { -0.2f, 0.25f }, { 0, -0.1f },
                             { 0.2f, 0.25f }, { 0.55f, -0.35f }, { 1, 0.15f } };
    color(d, 255, 230, 60, k);
    for (int i = 0; i < 7 && SET.seconds; i++) {
        float px = sx + (tx * CL[i][0] + nx * CL[i][1]) * 120, py = sy + (ty * CL[i][0] + ny * CL[i][1]) * 120;
        if (i == 0) vp_moveto(d, (int)px, (int)py);
        else        vp_lineto(d, (int)px, (int)py);
    }
    /* the time, far down the tube */
    char buf[8];
    snprintf(buf, sizeof buf, "%02d:%02d", n->hd, n->m);
    color(d, 255, 230, 60, k * 0.9f);
    text(d, buf, cx, cy - 40, 80, VP_TEXT_CENTER);
}

/* ---- ROCKS: the time among drifting rocks, a little ship circling it; at
 * each new minute the old minutes break up and drift away ------------------ */

#define NROCK 9
#define NFRAG 160

static struct {
    float x, y, vx, vy, a, spin, size;
    float rad[10];
} ROCK[NROCK];
static struct { float x0, y0, x1, y1, vx, vy, life; } FRAG[NFRAG];
static int rock_min = -1;
static uint32_t rock_seed = 12345;

static float rock_rnd(void)
{
    rock_seed = rock_seed * 1664525u + 1013904223u;
    return (float)(rock_seed >> 8) / 16777215.0f;
}

static void rocks_make(void)
{
    for (int i = 0; i < NROCK; i++) {
        ROCK[i].x = CX + (rock_rnd() - 0.5f) * W / ASPECT_X;
        ROCK[i].y = rock_rnd() * W;
        float a = rock_rnd() * 2 * PI, v = 14 + 22 * rock_rnd();       /* slow: minutes to cross the screen */
        ROCK[i].vx = cosf(a) * v;
        ROCK[i].vy = sinf(a) * v;
        ROCK[i].a = rock_rnd() * 2 * PI;
        ROCK[i].spin = (rock_rnd() - 0.5f) * 0.16f;
        ROCK[i].size = i < 3 ? 260 : i < 6 ? 170 : 100;
        for (int j = 0; j < 10; j++) ROCK[i].rad[j] = 0.7f + 0.3f * rock_rnd();
    }
}

/* the old minutes into pieces: each stroke of their digits drifts off */
static void rocks_break(int old_min)
{
    int nf = 0;
    const float h = 900, w = h * 0.6f, y = 1600, x0 = CX + 140;
    for (int dgt = 0; dgt < 2; dgt++) {
        int g = dgt == 0 ? old_min / 10 : old_min % 10;
        float ox = x0 + dgt * (w + 170);
        for (int s = 0; s < GLYPH_N[g]; s++) {
            const stroke *st = &GLYPH[g][s];
            for (int i = 0; i + 1 < st->n && nf < NFRAG; i += 2) {
                int j = i + 2 < st->n ? i + 2 : i + 1;
                float ax = ox + st->x[i] / 0.6f * w, ay = y + st->y[i] * h;
                float bx = ox + st->x[j] / 0.6f * w, by = y + st->y[j] * h;
                float mx = (ax + bx) / 2 - (ox + w / 2), my = (ay + by) / 2 - (y + h / 2);
                float l = sqrtf(mx * mx + my * my) + 1;
                float sp = 120 + 160 * rock_rnd();
                FRAG[nf].x0 = ax; FRAG[nf].y0 = ay; FRAG[nf].x1 = bx; FRAG[nf].y1 = by;
                FRAG[nf].vx = mx / l * sp + (rock_rnd() - 0.5f) * 60;
                FRAG[nf].vy = my / l * sp + (rock_rnd() - 0.5f) * 60;
                FRAG[nf].life = 1;
                nf++;
            }
        }
    }
    for (; nf < NFRAG; nf++) FRAG[nf].life = 0;
}

static void rocks_update(float dt)
{
    if (rock_min < 0) { rocks_make(); rock_min = G.now.m; }
    if (G.now.m != rock_min) { rocks_break(rock_min); rock_min = G.now.m; }
    for (int i = 0; i < NROCK; i++) {
        float span = W / ASPECT_X + 600, left = CX - span / 2;                 /* round the edges */
        ROCK[i].x = fmodf(ROCK[i].x - left + ROCK[i].vx * dt + span, span) + left;
        ROCK[i].y = fmodf(ROCK[i].y + 300 + ROCK[i].vy * dt + W + 600, W + 600) - 300;   /* -300..W+300 */
        ROCK[i].a += ROCK[i].spin * dt;
    }
    for (int i = 0; i < NFRAG; i++) {
        if (FRAG[i].life <= 0) continue;
        float dx = FRAG[i].vx * dt, dy = FRAG[i].vy * dt;
        FRAG[i].x0 += dx; FRAG[i].x1 += dx; FRAG[i].y0 += dy; FRAG[i].y1 += dy;
        FRAG[i].life -= dt / 3.5f;
    }
}

static void face_rocks(vp_dvg *d, float k)
{
    const now_t *n = &G.now;
    /* the rocks, faint behind the time */
    for (int i = 0; i < NROCK; i++) {
        color(d, 200, 165, 130, k * 0.45f * accent());  /* the rocks: brown */
        for (int j = 0; j <= 10; j++) {
            float a = ROCK[i].a + j * 2 * PI / 10, rr = ROCK[i].size * ROCK[i].rad[j % 10];
            int x = (int)(ROCK[i].x + cosf(a) * rr), y = (int)(ROCK[i].y + sinf(a) * rr);
            if (SX(x) < 0 || SX(x) > W || y < 0 || y > W) { j = 11; break; }
            if (j == 0) vp_moveto(d, x, y);
            else        vp_lineto(d, x, y);
        }
    }
    /* the time */
    const float h = 900, w = h * 0.6f, y = 1600;
    color(d, 90, 230, 255, k);                          /* the time: cyan */
    glyph(d, n->hd / 10, CX - 140 - 2 * w - 170, y, w, h);
    glyph(d, n->hd % 10, CX - 140 - w, y, w, h);
    circle(d, CX, y + h * 0.3f, 22, 8);
    circle(d, CX, y + h * 0.7f, 22, 8);
    glyph(d, n->m / 10, CX + 140, y, w, h);
    glyph(d, n->m % 10, CX + 140 + w + 170, y, w, h);
    /* the pieces of the last minute */
    for (int i = 0; i < NFRAG; i++) {
        if (FRAG[i].life <= 0) continue;
        color(d, 90, 230, 255, k * smooth01(FRAG[i].life));
        line(d, FRAG[i].x0, FRAG[i].y0, FRAG[i].x1, FRAG[i].y1);
    }
    /* the ship, gliding round the time, nose ahead (the seconds: once a minute) */
    float sec = sec_hand(n), a = PI / 2 - sec / 60 * 2 * PI;
    float sx = CX + cosf(a) * 1750, sy = y + h / 2 + sinf(a) * 1250;
    float hx = sinf(a) * 1750, hy = -cosf(a) * 1250, hl = sqrtf(hx * hx + hy * hy);
    hx /= hl; hy /= hl;                                          /* its heading: along the path */
    float px = -hy, py = hx, s = 70;
    color(d, 255, 230, 90, k);                          /* the ship: yellow */
    vp_moveto(d, (int)(sx + hx * s * 1.6f), (int)(sy + hy * s * 1.6f));
    vp_lineto(d, (int)(sx - hx * s + px * s), (int)(sy - hy * s + py * s));
    vp_lineto(d, (int)(sx - hx * s * 0.4f), (int)(sy - hy * s * 0.4f));
    vp_lineto(d, (int)(sx - hx * s - px * s), (int)(sy - hy * s - py * s));
    vp_lineto(d, (int)(sx + hx * s * 1.6f), (int)(sy + hy * s * 1.6f));
    if (SET.seconds) {                                  /* the seconds, small, under it all */
        char buf[8];
        snprintf(buf, sizeof buf, "%02d", n->s);
        color(d, 240, 240, 250, k * 0.6f);
        text(d, buf, CX, 1150, 110, VP_TEXT_CENTER);
    }
}

/* ---- the weather ------------------------------------------------------------ */

static void cloud(vp_dvg *d, float cx, float cy, float s)
{
    /* three bumps on a flat bottom */
    vp_moveto(d, (int)(cx - s * 0.9f), (int)(cy - s * 0.35f));
    vp_lineto(d, (int)(cx + s * 0.9f), (int)(cy - s * 0.35f));
    arc(d, cx + s * 0.62f, cy - s * 0.05f, s * 0.3f, s * 0.3f, -70, 110, 8);
    arc(d, cx + s * 0.05f, cy + s * 0.15f, s * 0.5f, s * 0.5f, 20, 165, 10);
    arc(d, cx - s * 0.6f, cy - s * 0.07f, s * 0.32f, s * 0.29f, 70, 250, 8);
}

static void sun(vp_dvg *d, float cx, float cy, float s, int rays)
{
    circle(d, cx, cy, s * 0.42f, 20);
    for (int i = 0; i < rays; i++) {
        float a = i * 2 * PI / rays;
        line(d, cx + cosf(a) * s * 0.58f, cy + sinf(a) * s * 0.58f, cx + cosf(a) * s * 0.8f, cy + sinf(a) * s * 0.8f);
    }
}

static void moon(vp_dvg *d, float cx, float cy, float s)
{
    arc(d, cx, cy, s * 0.5f, s * 0.5f, 60, 300, 18);
    arc(d, cx + s * 0.2f, cy, s * 0.36f, s * 0.42f, 250, 110, 14);
}

/* the weather as a picture, s about its half width */
static void weather_icon(vp_dvg *d, int c, int day, float cx, float cy, float s, float k)
{
    int wet = (c >= 51 && c <= 67) || (c >= 80 && c <= 82);
    int snow = (c >= 71 && c <= 77) || c == 85 || c == 86;
    if (c <= 2) {                                   /* sun or moon, maybe a cloud before it */
        float ox = c == 2 ? -s * 0.35f : 0, oy = c == 2 ? s * 0.3f : 0, sc = c == 2 ? 0.75f : 1;
        color(d, 255, 210, 90, k);
        if (day) sun(d, cx + ox, cy + oy, s * sc, 8);
        else     moon(d, cx + ox, cy + oy, s * sc);
        if (c == 1) {
            color(d, 200, 215, 240, k);
            cloud(d, cx + s * 0.45f, cy - s * 0.45f, s * 0.45f);
        }
    }
    if (c == 45 || c == 48) {                       /* fog */
        color(d, 200, 215, 240, k);
        for (int i = 0; i < 4; i++) {
            float y = cy + s * (0.4f - 0.27f * i), in = (i & 1) ? 0.15f : 0;
            line(d, cx - s * (0.8f - in), y, cx + s * (0.65f + in), y);
        }
        return;
    }
    if (c >= 2) {
        color(d, 200, 215, 240, k);
        if (c == 2) cloud(d, cx + s * 0.15f, cy - s * 0.05f, s * 0.75f);
        else        cloud(d, cx, cy + s * 0.15f, s * 0.95f);
    }
    if (wet) {
        color(d, 110, 170, 255, k);
        for (int i = 0; i < 4; i++) {
            float x = cx - s * 0.55f + i * s * 0.35f;
            line(d, x + s * 0.08f, cy - s * 0.3f, x - s * 0.08f, cy - s * 0.75f);
        }
    }
    if (snow) {
        color(d, 230, 240, 255, k);
        for (int i = 0; i < 3; i++) {
            float x = cx - s * 0.45f + i * s * 0.45f, y = cy - s * 0.55f - (i & 1) * s * 0.15f, a = s * 0.1f;
            line(d, x - a, y, x + a, y);
            line(d, x - a * 0.5f, y - a * 0.87f, x + a * 0.5f, y + a * 0.87f);
            line(d, x - a * 0.5f, y + a * 0.87f, x + a * 0.5f, y - a * 0.87f);
        }
    }
    if (c >= 95) {                                  /* lightning */
        color(d, 255, 220, 90, k);
        vp_moveto(d, (int)(cx + s * 0.05f), (int)(cy - s * 0.25f));
        vp_lineto(d, (int)(cx - s * 0.15f), (int)(cy - s * 0.6f));
        vp_lineto(d, (int)(cx + s * 0.05f), (int)(cy - s * 0.6f));
        vp_lineto(d, (int)(cx - s * 0.12f), (int)(cy - s * 0.95f));
    }
}

/* a temperature in our own digits with its degree sign; returns its width */
static float temperature(vp_dvg *d, float t, float x, float y, float h, int draw)
{
    int v = (int)lroundf(t), neg = v < 0;
    if (neg) v = -v;
    float w = h * 0.6f, sp = h * 0.18f, cx = x;
    if (neg) {
        if (draw) line(d, cx, y + h * 0.5f, cx + w * 0.7f, y + h * 0.5f);
        cx += w * 0.7f + sp;
    }
    if (v >= 10) {
        if (draw) glyph(d, v / 10 % 10, cx, y, w, h);
        cx += w + sp;
    }
    if (draw) glyph(d, v % 10, cx, y, w, h);
    cx += w + sp * 0.8f;
    if (draw) circle(d, cx + h * 0.1f, y + h * 0.88f, h * 0.1f, 10);
    return cx + h * 0.2f - x;
}

/* ---- text with degree signs ------------------------------------------------- */

/* s with '^' where a degree sign goes (the font has none: drawn), at x as
 * on the screen (already pulled in), returns its width */
static float text_deg(vp_dvg *d, const char *s, float x, float y, float h, int align, int draw)
{
    vp_text_font(VP_FONT_ASTEROIDS);
    char part[160];
    float deg = h * 0.42f, total = 0;
    const char *p = s;
    for (;;) {                                          /* its width first */
        const char *e = strchr(p, '^');
        size_t l = e ? (size_t)(e - p) : strlen(p);
        if (l >= sizeof part) l = sizeof part - 1;
        memcpy(part, p, l);
        part[l] = 0;
        total += vp_text_width(part, h);
        if (!e) break;
        total += deg;
        p = e + 1;
    }
    if (!draw) return total;
    float cx = align == VP_TEXT_CENTER ? x - total / 2 : align == VP_TEXT_RIGHT ? x - total : x;
    p = s;
    for (;;) {
        const char *e = strchr(p, '^');
        size_t l = e ? (size_t)(e - p) : strlen(p);
        if (l >= sizeof part) l = sizeof part - 1;
        memcpy(part, p, l);
        part[l] = 0;
        vp_text(d, part, cx, y, h, VP_TEXT_LEFT);
        cx += vp_text_width(part, h);
        if (!e) break;
        SQUEEZE = 0;                                    /* (beside the text: in its own shape) */
        arc(d, cx + deg * 0.45f, y + h * 0.86f, h * 0.13f * ASPECT_X, h * 0.13f, 90, 450, 8);
        SQUEEZE = 1;
        cx += deg;
        p = e + 1;
    }
    return total;
}

/* ---- the weather, in its styles ------------------------------------------------ */

/* in its own colors: a temperature from blue (cold) to red (hot) */
static void temp_color(vp_dvg *d, float t, float k)
{
    static const float T[6] = { -15, 0, 10, 18, 26, 34 };
    static const float C[6][3] = { { 110, 140, 255 }, { 90, 220, 255 }, { 120, 255, 170 }, { 240, 240, 110 },
                                   { 255, 170, 60 }, { 255, 80, 60 } };
    int i = 0;
    while (i < 4 && t > T[i + 1]) i++;
    float f = clampf((t - T[i]) / (T[i + 1] - T[i]), 0, 1);
    color(d, C[i][0] + (C[i + 1][0] - C[i][0]) * f, C[i][1] + (C[i + 1][1] - C[i][1]) * f,
          C[i][2] + (C[i + 1][2] - C[i][2]) * f, k);
}

static void pale(vp_dvg *d, float k) { color(d, 200, 215, 240, k); }

static const char *wind_dir(int deg) { return L_WIND_DIR[SET.lang][((deg + 22) % 360) / 45]; }

/* the place and the time, on top */
static void wx_head(vp_dvg *d, float k)
{
    char buf[96];
    snprintf(buf, sizeof buf, "%s   %02d:%02d", SET.place, G.now.hd, G.now.m);
    pale(d, k * 0.8f);
    vp_text_font(VP_FONT_ASTEROIDS);
    vp_text_fit(d, buf, CX, 3600, 120, W * 0.86f, VP_TEXT_CENTER);
}

/* "NOW  7^C  OVERCAST", one line */
static void wx_now_line(vp_dvg *d, float y, float k)
{
    char buf[128];
    snprintf(buf, sizeof buf, "%s  %d^C  %s", S(S_NOW), (int)lroundf(G.wx.temp), weather_words(G.wx.code, SET.lang));
    pale(d, k * 0.85f);
    text_deg(d, buf, CX, y, 110, VP_TEXT_CENTER, 1);
}

static void wx_overview(vp_dvg *d, float k)
{
    const weather_t *w = &G.wx;
    const now_t *n = &G.now;
    char buf[96];
    wx_head(d, k);
    weather_icon(d, w->code, w->is_day, CX - 900, 2600, 600, k);
    temp_color(d, w->temp, k);
    float th = 800, tw = temperature(d, w->temp, 0, 0, th, 0);
    temperature(d, w->temp, CX + 450 - tw / 2, 2250, th, 1);
    pale(d, k * 0.85f);
    vp_text_fit(d, weather_words(w->code, SET.lang), SX(CX + 450), 2000, 120, W * 0.5f, VP_TEXT_CENTER);
    snprintf(buf, sizeof buf, "%s %d KM/H   %s %d%%", S(S_WIND), (int)lroundf(w->wind), S(S_HUMIDITY),
             (int)lroundf(w->humidity));
    pale(d, k * 0.6f * accent());
    text(d, buf, CX, 1700, 90, VP_TEXT_CENTER);
    for (int i = 0; i < w->days && i < 4; i++) {        /* the next days */
        float x = CX + (i - 1.5f) * 900, y = 1050;
        pale(d, k * 0.75f);
        text(d, i == 0 ? S(S_TODAY) : L_WEEKDAY_SHORT[SET.lang][(n->wday + i) % 7], SX(x), y + 330, 90, VP_TEXT_CENTER);
        weather_icon(d, w->day_code[i], 1, x, y + 120, 170, k * 0.9f);
        snprintf(buf, sizeof buf, "%d^ / %d^", (int)lroundf(w->day_max[i]), (int)lroundf(w->day_min[i]));
        temp_color(d, w->day_max[i], k * 0.85f);
        text_deg(d, buf, SX(x), y - 200, 90, VP_TEXT_CENTER, 1);
    }
}

static void wx_simple(vp_dvg *d, float k)
{
    const weather_t *w = &G.wx;
    wx_head(d, k);
    weather_icon(d, w->code, w->is_day, CX - 800, 2250, 780, k);
    temp_color(d, w->temp, k);
    float th = 1000, tw = temperature(d, w->temp, 0, 0, th, 0);
    temperature(d, w->temp, CX + 600 - tw / 2, 1800, th, 1);
    pale(d, k * 0.9f);
    vp_text_font(VP_FONT_ASTEROIDS);
    vp_text_fit(d, weather_words(w->code, SET.lang), CX, 1050, 170, W * 0.8f, VP_TEXT_CENTER);
}

static void wx_today(vp_dvg *d, float k)
{
    const weather_t *w = &G.wx;
    wx_head(d, k);
    weather_icon(d, w->code, w->is_day, CX - 1100, 2700, 560, k);
    temp_color(d, w->temp, k);
    float th = 680, tw = temperature(d, w->temp, 0, 0, th, 0);
    temperature(d, w->temp, CX + 350 - tw / 2, 2420, th, 1);
    pale(d, k * 0.9f);
    vp_text_fit(d, weather_words(w->code, SET.lang), SX(CX + 350), 2190, 110, W * 0.5f, VP_TEXT_CENTER);
    /* the details, in two columns: a word, then its value */
    struct { int label; char value[48]; float t; } row[8];
    int nr = 0;
    row[nr].label = S_FEELS; snprintf(row[nr].value, 48, "%d^C", (int)lroundf(w->feels)); row[nr++].t = w->feels;
    row[nr].label = S_WIND; snprintf(row[nr].value, 48, "%d KM/H %s", (int)lroundf(w->wind), wind_dir(w->wind_dir));
    row[nr++].t = -99;
    row[nr].label = S_MINMAX;
    snprintf(row[nr].value, 48, "%d^ / %d^", w->days ? (int)lroundf(w->day_min[0]) : 0, w->days ? (int)lroundf(w->day_max[0]) : 0);
    row[nr++].t = w->days ? w->day_max[0] : -99;
    row[nr].label = S_HUMIDITY; snprintf(row[nr].value, 48, "%d%%", (int)lroundf(w->humidity)); row[nr++].t = -99;
    row[nr].label = S_RAIN; snprintf(row[nr].value, 48, "%d%%", w->days && w->day_pop[0] >= 0 ? w->day_pop[0] : 0);
    row[nr++].t = -99;
    if (w->sunrise >= 0) {
        row[nr].label = S_SUNRISE; snprintf(row[nr].value, 48, "%02d:%02d", w->sunrise / 60, w->sunrise % 60);
        row[nr++].t = -99;
    }
    if (w->sunset >= 0) {
        row[nr].label = S_SUNSET; snprintf(row[nr].value, 48, "%02d:%02d", w->sunset / 60, w->sunset % 60);
        row[nr++].t = -99;
    }
    for (int i = 0; i < nr; i++) {
        float x = i & 1 ? CX + 150 : CX - 2050, y = 1700 - (i / 2) * 290;
        pale(d, k * 0.55f);
        vp_text_font(VP_FONT_ASTEROIDS);
        vp_text_fit(d, S(row[i].label), SX(x), y, 75, 750, VP_TEXT_LEFT);
        if (row[i].t > -99) temp_color(d, row[i].t, k);
        else                pale(d, k);
        text_deg(d, row[i].value, SX(x + 1900), y, 85, VP_TEXT_RIGHT, 1);
    }
}

static void wx_days(vp_dvg *d, float k)
{
    const weather_t *w = &G.wx;
    char buf[48];
    wx_head(d, k);
    wx_now_line(d, 3250, k);
    int n = w->days < WX_DAYS ? w->days : WX_DAYS;
    for (int i = 0; i < n; i++) {
        float x = CX + (i - (n - 1) / 2.0f) * 560;
        pale(d, k * 0.75f);
        text(d, i == 0 ? S(S_TODAY) : L_WEEKDAY_SHORT[SET.lang][(G.now.wday + i) % 7], SX(x), 2600, 75, VP_TEXT_CENTER);
        weather_icon(d, w->day_code[i], 1, x, 2200, 190, k);
        snprintf(buf, sizeof buf, "%d^", (int)lroundf(w->day_max[i]));
        temp_color(d, w->day_max[i], k);
        text_deg(d, buf, SX(x), 1700, 120, VP_TEXT_CENTER, 1);
        snprintf(buf, sizeof buf, "%d^", (int)lroundf(w->day_min[i]));
        temp_color(d, w->day_min[i], k * 0.6f);
        text_deg(d, buf, SX(x), 1430, 95, VP_TEXT_CENTER, 1);
        if (w->day_pop[i] >= 0) {
            snprintf(buf, sizeof buf, "%d%%", w->day_pop[i]);
            color(d, 110, 170, 255, k * 0.8f);
            text(d, buf, SX(x), 1130, 75, VP_TEXT_CENTER);
        }
    }
}

/* through the day: the temperature as a curve, the chance of rain as bars */
static void wx_hours(vp_dvg *d, float k)
{
    const weather_t *w = &G.wx;
    char buf[48];
    wx_head(d, k);
    wx_now_line(d, 3250, k);
    int n = w->hours;
    if (n < 2) return;
    float lo = w->hour_temp[0], hi = lo;
    for (int i = 1; i < n; i++) { lo = fminf(lo, w->hour_temp[i]); hi = fmaxf(hi, w->hour_temp[i]); }
    if (hi - lo < 4) { float m = (hi + lo) / 2; lo = m - 2; hi = m + 2; }
    const float x0 = 450, x1 = W - 450, y0 = 1350, y1 = 2350, rb = 750, rh = 300;
    float X[WX_HOURS], Y[WX_HOURS];
    for (int i = 0; i < n; i++) {
        X[i] = x0 + (x1 - x0) * i / (n - 1);
        Y[i] = y0 + (y1 - y0) * (w->hour_temp[i] - lo) / (hi - lo);
    }
    /* the rain: a bar each hour, the chance of it */
    color(d, 110, 170, 255, k * 0.35f);
    line(d, x0, rb, x1, rb);
    for (int i = 0; i < n; i++) {
        if (w->hour_pop[i] <= 0) continue;
        color(d, 110, 170, 255, k * 0.85f);
        line(d, X[i], rb, X[i], rb + rh * w->hour_pop[i] / 100.0f);
    }
    /* the curve, each piece as warm or cold as it is */
    for (int i = 0; i + 1 < n; i++) {
        temp_color(d, (w->hour_temp[i] + w->hour_temp[i + 1]) / 2, k);
        line(d, X[i], Y[i], X[i + 1], Y[i + 1]);
    }
    /* every third hour: its time, its weather, its temperature */
    for (int i = 0; i < n; i += 3) {
        temp_color(d, w->hour_temp[i], k);
        circle(d, X[i], Y[i], 22, 8);
        snprintf(buf, sizeof buf, "%d^", (int)lroundf(w->hour_temp[i]));
        text_deg(d, buf, SX(X[i]), Y[i] + 70, 70, VP_TEXT_CENTER, 1);
        weather_icon(d, w->hour_code[i], w->hour_of[i] >= 7 && w->hour_of[i] < 19, X[i], 2850, 120, k * 0.9f);
        pale(d, k * 0.6f);
        snprintf(buf, sizeof buf, "%02d", w->hour_of[i]);
        text(d, buf, SX(X[i]), 1080, 65, VP_TEXT_CENTER);
    }
    pale(d, k * 0.45f);
    text(d, S(S_RAIN), SX(x0 - 60), rb + 40, 55, VP_TEXT_RIGHT);
}

/* the weather, in a style (WS_*), in its own colors or the one the menu sets */
static void weather_view(vp_dvg *d, float k, int style)
{
    int was = PAINT;
    int wt = theme(SET.wx_theme, PART_WX);
    PAINT = SET.wx_colorful ? 0 : wt ? wt : 7;
    const now_t *n = &G.now;
    if (!G.wx.ok) {
        char buf[96], date[64];
        pale(d, k * 0.7f);
        text(d, S(S_NO_WEATHER), CX, 2300, 160, VP_TEXT_CENTER);
        pale(d, k * 0.45f);
        if (!SET.has_place) text(d, S(S_NO_WEATHER_HINT), CX, 2000, 90, VP_TEXT_CENTER);
        date_text(date, sizeof date, n, 1);
        snprintf(buf, sizeof buf, "%s   %02d:%02d", date, n->hd, n->m);
        text(d, buf, CX, 1500, 110, VP_TEXT_CENTER);
    } else {
        switch (style) {
        case WS_SIMPLE: wx_simple(d, k); break;
        case WS_TODAY:  wx_today(d, k); break;
        case WS_DAYS:   wx_days(d, k); break;
        case WS_HOURS:  wx_hours(d, k); break;
        default:        wx_overview(d, k); break;
        }
    }
    PAINT = was;
}

/* the style the menu says; turning, each in turn */
static int wx_style_now(int turn)
{
    return SET.wx_style == WS_ROTATE ? turn % WS_ROTATE : SET.wx_style;
}

static void face_weather(vp_dvg *d, float k)
{
    weather_view(d, k, wx_style_now((int)(G.time / 15)));
}

/* under the other faces: the date and the weather, one line, each in its
 * color, as the menu says */
static void info_line(vp_dvg *d, float k)
{
    const now_t *n = &G.now;
    char date[96] = "", wx[128] = "";
    int has_wx = SET.weather && G.wx.ok;
    if (SET.date) date_text(date, sizeof date, n, !has_wx);
    if (has_wx) snprintf(wx, sizeof wx, "%s  %d^C  %s", SET.place, (int)lroundf(G.wx.temp), weather_words(G.wx.code, SET.lang));
    if (!date[0] && !wx[0]) return;
    vp_text_font(VP_FONT_ASTEROIDS);
    float h = 90, gap = date[0] && wx[0] ? 300 : 0;
    float wd = date[0] ? vp_text_width(date, h) : 0, ww = wx[0] ? text_deg(d, wx, 0, 0, h, VP_TEXT_LEFT, 0) : 0;
    float total = wd + gap + ww;
    if (total > W * 0.86f) { float f = W * 0.86f / total; h *= f; wd *= f; gap *= f; ww *= f; total *= f; }
    float x = CX - total / 2, y = 360;
    int was = PAINT;
    if (date[0]) {
        PAINT = theme(SET.date_theme, PART_DATE);
        color(d, 200, 215, 240, k * 0.55f);
        vp_text(d, date, x, y, h, VP_TEXT_LEFT);
    }
    if (wx[0]) {
        PAINT = theme(SET.wx_theme, PART_WX);
        color(d, 200, 215, 240, k * 0.55f);
        text_deg(d, wx, x + wd + gap, y, h, VP_TEXT_LEFT, 1);
    }
    PAINT = was;
}

static void face_draw(vp_dvg *d, int f, float k)
{
    if (k <= 0.01f) return;
    PAINT = theme(SET.theme, PART_CLOCK);
    switch (f) {
    case 0: face_analog(d, k); break;
    case 1: face_digital(d, k); break;
    case 2: face_flip(d, k); break;
    case 3: face_binary(d, k); break;
    case 4: face_words(d, k); break;
    case 5: face_nixie(d, k); break;
    case 6: face_orbit(d, k); break;
    case 7: face_tunnel(d, k); break;
    case 8: face_rocks(d, k); break;
    case 9: face_minimal(d, k); break;
    case 10: face_numerals(d, k); break;
    case 11: face_roman(d, k); break;
    case 12: face_square(d, k); break;
    case 13: face_full(d, k); break;
    case FACE_WEATHER: face_weather(d, k); break;
    }
    if (f != FACE_WEATHER) info_line(d, k);
}

/* ---- the game --------------------------------------------------------------- */

static void face_switch(int f)
{
    f = (f % NFACE + NFACE) % NFACE;
    if (f == G.face) return;
    G.from = G.face;
    G.face = f;
    G.blend = 0;
    G.name_time = 3.5f;
    pick_colors();
    SET.face = f;
    G.was.face = f;
}

/* the next face: the one after, or one at random as the menu says */
static int next_face(void)
{
    /* with weather reports the weather face is left out (it would only show
     * them again, and after it no report would come to change it) */
    int skip_wx = SET.report > 0;
    if (SET.random != 2) {
        int f = (G.face + 1) % NFACE;
        return skip_wx && f == FACE_WEATHER ? 0 : f;
    }
    for (;;) {
        int f = rand() % NFACE;
        if (f != G.face && !(skip_wx && f == FACE_WEATHER)) return f;   /* never the same again */
    }
}

/* now and then the weather, full screen, as the menu says; after it maybe
 * the next face */
static void report_update(float dt)
{
    if (G.report_on) {
        if ((G.report_t += dt) >= REPORT_LEN_S[SET.report_len] || (SET.report == 0 && !G.report_spun) || !G.wx.ok) {
            int spun = G.report_spun;
            G.report_on = 0;
            G.report_spun = 0;
            G.report_t = 0;
            if (!spun && SET.change == CHANGE_AFTER_REPORT) face_switch(next_face());
        }
    } else if (SET.report > 0 && G.wx.ok && !menu_is_open() && G.face != FACE_WEATHER) {
        static float every = -1;                        /* SAVER_REPORT_EVERY=<s>: sooner (for development) */
        if (every < 0) every = getenv("SAVER_REPORT_EVERY") ? (float)atof(getenv("SAVER_REPORT_EVERY")) : 0;
        if ((G.report_t += dt) >= (every > 0 ? every : REPORT_MIN[SET.report] * 60.0f)) {
            G.report_on = 1;
            G.report_t = 0;
            G.report_turn++;
        }
    }
    G.report_k += ((G.report_on ? 1.0f : 0.0f) - G.report_k) * fminf(1, dt / 0.6f);
}

/* the word clock's words straight to the time, in the language now */
static void words_reset(void)
{
    float on[NWORD];
    G.words_lang = SET.lang;
    words_target(G.words_lang, &G.now, on);
    for (int i = 0; i < NWORD; i++) G.words[i] = on[i];
}

/* what the menu changed: follow it */
static void follow_settings(void)
{
    if (SET.face != G.was.face) face_switch(SET.face);
    if (SET.has_place != G.was.has_place || SET.lat != G.was.lat || SET.lon != G.was.lon)
        G.place_serial = weather_set_place(SET.has_place, SET.lat, SET.lon);
    if (SET.lang != G.was.lang) words_reset();
    G.was = SET;
}

static void wake(void)
{
    input_release();                    /* so the cabinet's menu sees the control let go */
    vp_game_quit(G.game);
}

/* the spinner: a step through the faces; after the last (and before the
 * first) the weather report, when there is weather */
static void spin(int way)
{
    if (G.report_on) {                                  /* from the report: to the first face, or the last */
        G.report_on = 0;
        G.report_spun = 0;
        G.report_t = 0;
        face_switch(way > 0 ? 0 : NFACE - 1);
        return;
    }
    int f = G.face + way;
    if ((f < 0 || f >= NFACE) && G.wx.ok) {
        G.report_on = 1;
        G.report_spun = 1;
        G.report_t = 0;
        G.report_turn++;
        return;
    }
    face_switch(f);
}

/* a control: to the menu when it is open, else open it, next track, step, or wake */
static void control(int in)
{
    if (in == IN_NONE) return;
    if (menu_is_open()) {
        if (menu_input(in)) settings_save();
        return;
    }
    switch (in) {
    case IN_MENU: case IN_OK: menu_open(); break;      /* (IN_OK: Enter on a PC) */
    case IN_START2: vp_saver_skip(); break;
    case IN_SPIN_L: spin(-1); break;
    case IN_SPIN_R: spin(1); break;
    case IN_LEFT:  spin(-1); break;                     /* (a PC: the arrow keys, as the spinner) */
    case IN_RIGHT: spin(1); break;
    case IN_BACK: break;
    default: wake();
    }
}

static int init(vp_game *g)
{
    G.game = g;
    if (getenv("VP_VERTICAL") && atoi(getenv("VP_VERTICAL"))) { ASPECT_X = 1; ASPECT_Y = 0.75f; }
    glyphs_make();
    const char *f = getenv("SAVER_FACE"), *c = getenv("SAVER_CYCLE");
    srand((unsigned)time(NULL));
    if (SET.random >= 1) SET.face = rand() % (SET.report > 0 ? NFACE - 1 : NFACE);   /* a random one at start */
    pick_colors();
    if (f) SET.face = ((atoi(f) % NFACE) + NFACE) % NFACE;
    G.face = G.from = SET.face;
    G.was = SET;
    G.place_serial = weather_set_place(SET.has_place, SET.lat, SET.lon);
    weather_start();
    G.grabbed = input_start();
    G.blend = 1;
    G.name_time = 3.5f;
    G.cycle = c ? (float)atof(c) : 0;
    G.now = read_now();
    int dg[6];
    digits6(&G.now, dg);
    for (int i = 0; i < 6; i++) {
        G.flip_cur[i] = G.flip_old[i] = G.nix_cur[i] = G.nix_old[i] = dg[i];
        G.flip_t[i] = G.nix_t[i] = 1;
    }
    words_reset();
    return VP_OK;
}

static void update(vp_game *g, float dt)
{
    G.time += dt;
    weather_get(&G.wx);
    G.now = read_now();
    G.have = vp_saver_music(&G.m);
    if (G.have && G.m.track != G.last_track) {
        if (G.started && SET.change == 1) face_switch(next_face());  /* the menu says: with each track */
        G.started = 1;
        G.last_track = G.m.track;
        snprintf(G.title, sizeof G.title, "%s", G.m.title);
        G.title_time = G.title[0] ? 7 : 0;
    }
    /* the controls: the cabinet's, taken over; on a PC the keyboard */
    int ev;
    while ((ev = input_next(menu_is_open())) != IN_NONE) control(ev);
    vp_input *in = vp_game_input(g);
    if (in && !G.grabbed) {
        static const struct { const char *key; int in; } KEY[] = {
            { "key_Up", IN_UP }, { "key_Down", IN_DOWN }, { "key_Left", IN_LEFT }, { "key_Right", IN_RIGHT },
            { "key_Return", IN_OK }, { "key_M", IN_MENU }, { "key_Tab", IN_MENU }, { "key_F2", IN_MENU }, { "key_Backspace", IN_BACK }, { "key_N", IN_START2 },
        };
        for (size_t i = 0; i < sizeof KEY / sizeof KEY[0]; i++)
            if (vp_input_pressed(in, KEY[i].key)) control(KEY[i].in);
    }
    {   /* SAVER_SCRIPT="<s>:<control>,..." presses controls at those times (for development);
         * controls: U D L R OK BACK MENU SL SR (the spinner) S1 S2, K<code> (a button by its Linux key code) */
        static const char *script;
        static int checked;
        if (!checked) { script = getenv("SAVER_SCRIPT"); checked = 1; }
        while (script && *script) {
            float at = (float)atof(script);
            if (G.time < at) break;
            const char *c = strchr(script, ':');
            if (!c) { script = NULL; break; }
            c++;
            int v = *c == 'K' ? input_key(atoi(c + 1), menu_is_open())   /* a button by its Linux key code */
                  : !strncmp(c, "OK", 2) ? IN_OK : !strncmp(c, "BACK", 4) ? IN_BACK : !strncmp(c, "MENU", 4) ? IN_MENU : !strncmp(c, "SL", 2) ? IN_SPIN_L
                  : !strncmp(c, "SR", 2) ? IN_SPIN_R : !strncmp(c, "S1", 2) ? IN_START1
                  : !strncmp(c, "S2", 2) ? IN_START2 : *c == 'U' ? IN_UP : *c == 'D' ? IN_DOWN : *c == 'L' ? IN_LEFT
                  : *c == 'R' ? IN_RIGHT : IN_NONE;
            control(v);
            script = strchr(c, ',');
            if (script) script++;
        }
    }
    if (menu_update(dt)) settings_save();
    follow_settings();
    if (G.cycle > 0 && (G.cycle_t += dt) >= G.cycle) { G.cycle_t = 0; face_switch(G.face + 1); }
    else if (G.cycle <= 0 && SET.change >= 2 && SET.change < CHANGE_AFTER_REPORT && !menu_is_open()) {   /* every so many minutes */
        if ((G.cycle_t += dt) >= CHANGE_MIN[SET.change] * 60.0f) { G.cycle_t = 0; face_switch(next_face()); }
    } else if (G.cycle <= 0) G.cycle_t = 0;
    report_update(dt);

    G.title_time = fmaxf(0, G.title_time - dt);
    G.name_time = fmaxf(0, G.name_time - dt);
    G.blend = fminf(1, G.blend + dt / BLEND);
    G.loud += ((G.have ? G.m.level : 0) - G.loud) * fminf(1, dt * 0.4f);   /* slowed right down */

    int dg[6];
    digits6(&G.now, dg);
    for (int i = 0; i < 6; i++) {
        if (dg[i] != G.flip_cur[i] && G.flip_t[i] >= 1) {
            G.flip_old[i] = G.flip_cur[i];
            G.flip_cur[i] = dg[i];
            G.flip_t[i] = 0;
        }
        G.flip_t[i] = fminf(1, G.flip_t[i] + dt / 0.55f);
        if (dg[i] != G.nix_cur[i]) {
            G.nix_old[i] = G.nix_cur[i];
            G.nix_cur[i] = dg[i];
            G.nix_t[i] = 0;
        }
        G.nix_t[i] = fminf(1, G.nix_t[i] + dt / 0.3f);
        for (int b = 0; b < 4; b++) {
            float target = (dg[i] >> b) & 1 ? 1.0f : 0.0f;
            G.bits[i][b] += (target - G.bits[i][b]) * fminf(1, dt * 12);
        }
    }
    float on[NWORD];
    words_target(G.words_lang, &G.now, on);
    for (int i = 0; i < NWORD; i++) G.words[i] += (on[i] - G.words[i]) * fminf(1, dt * 2.5f);
    rocks_update(dt);
}

/* The track's title over the clock, then gone. */
static void title(vp_dvg *d)
{
    if (G.title_time <= 0 || !SET.title) return;
    float k = smooth01(G.title_time / 1.5f) * smooth01((7 - G.title_time) / 0.5f);
    vp_text_font(VP_FONT_ASTEROIDS);
    color(d, 200, 215, 255, k * 0.7f);
    vp_text_fit(d, G.title, CX, W * 0.95f, 85, W * 0.84f, VP_TEXT_CENTER);
}

/* SAVER_SHOTS=<folder>: a picture every 2 s, for development. */
static void shots(vp_dvg *d)
{
    static const char *dir;
    static int checked, n;
    static float next = 2;
    if (!checked) { dir = getenv("SAVER_SHOTS"); checked = 1; }
    if (!dir || G.time < next) return;
    next = G.time + 2;
    char path[512];
    snprintf(path, sizeof path, "%s/shot_%03d.png", dir, n++);
    vp_dvg_save_png(d, path, 1024, 768);    /* this frame's lines: call it last */
}

static void draw(vp_game *g, vp_dvg *d)
{
    (void)g;
    float t = smooth01(G.blend), dim = menu_is_open() ? 0.22f : 1;   /* behind the menu: faint */
    float rk = smooth01(G.report_k);                    /* the weather report over it all */
    dim *= 1 - rk;
    if (rk > 0.01f) weather_view(d, rk * (menu_is_open() ? 0.22f : 1), wx_style_now(G.report_turn));
    if (G.from != G.face) { PICK_SLOT = 1; face_draw(d, G.from, (1 - t) * dim); PICK_SLOT = 0; }
    face_draw(d, G.face, t * dim);
    if (G.name_time > 0 && !menu_is_open()) {        /* the face's name, a while */
        float k = smooth01(G.name_time / 1) * smooth01((3.5f - G.name_time) / 0.6f);
        color(d, 200, 210, 230, k * 0.6f);
        text(d, L_FACE[SET.lang][G.face], CX, 170, 70, VP_TEXT_CENTER);
    }
    if (!menu_is_open()) title(d);
    menu_draw(d);
    shots(d);
}

int main(int argc, char **argv)
{
    settings_load(argc > 0 ? argv[0] : NULL);
    vp_game_desc desc = {0};
    desc.title = "Clock";
    desc.init = init;
    desc.update = update;
    desc.draw = draw;
    int rc = vp_run(&desc, argc, argv);
    input_release();
    /* out at once: a normal exit would wait for a weather fetch still
     * running (it flushes every stream, the weather pipe too) */
    fflush(stdout);
    fflush(stderr);
    _Exit(rc);
}
