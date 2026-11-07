/* Clock — a VectorPie screensaver: the time, on one face after another.
 *
 * Ten faces: ANALOG (a watch dial with a sweeping second hand), DIGITAL
 * (seven segments, the date under it), KLAPPZAHLEN (a flip clock, its cards
 * folding over), BINAER (the digits in bits), WORTUHR (the time in German
 * words), NIXIE (glowing tubes), ORBIT (hours, minutes and seconds circling
 * like planets), TUNNEL (a web of lanes down a tube, a claw running round its
 * rim: in the manner of the old vector tube shooters), ROCKS (the time among
 * drifting rocks, a little ship circling it: in the manner of the old vector
 * rock shooters) and WETTER (the weather now and the next days). Under each
 * of the others a line with the date and the weather.
 *
 * The weather comes from Open-Meteo over the cabinet's network, for the
 * place in clock.ini (weather.h); without one the clock shows none.
 *
 * Switching: each new track brings the next face, so the cabinet's NEXT
 * TRACK control (which keeps the screensaver running, unlike every other
 * control) switches it. On a PC the arrow keys do too. One face blends into
 * the next.
 *
 * To the music, smoothly: the faces' rings and marks glow a little brighter
 * when it is louder. Nothing flashes or blinks (the colons stand still). A
 * new track shows its title.
 *
 * On the cabinet: installed in SETTINGS -> SCREENSAVERS (tools/deploy.sh or
 * the .vps). On a PC: saver_clock <a .ogg/.wav or a folder of them>. Esc
 * quits. SAVER_FACE=<0..9> starts on that face, SAVER_CYCLE=<s> switches
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

#include "weather.h"

#define W      VP_DVG_COORD_MAX
#define CX     (W / 2.0f)
#define PI     3.14159265f
#define NFACE  10
#define FACE_WEATHER 9
#define BLEND  1.4f                     /* s from one face to the next */

static const char *FACE_NAME[NFACE] = { "ANALOG", "DIGITAL", "KLAPPZAHLEN", "BINAER", "WORTUHR", "NIXIE", "ORBIT", "TUNNEL", "ROCKS", "WETTER" };

typedef struct { int h, m, s, wday, mday, mon, year; float frac; } now_t;

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
    weather_t wx;                       /* the latest weather */
    vp_music_feed m;
    int have, started;
    unsigned last_track;
    float title_time;
    char title[VP_MUSIC_TEXT];
} G;

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
static float smooth01(float t) { t = clampf(t, 0, 1); return t * t * (3 - 2 * t); }

static void color(vp_dvg *d, float r, float g, float b, float k)
{
    k = clampf(k, 0, 1);
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

static now_t read_now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t t = (time_t)tv.tv_sec;
    struct tm *lt = localtime(&t);
    now_t n = { lt->tm_hour, lt->tm_min, lt->tm_sec, lt->tm_wday, lt->tm_mday, lt->tm_mon + 1,
                lt->tm_year + 1900, (float)tv.tv_usec / 1e6f };
    return n;
}

static void digits6(const now_t *n, int out[6])
{
    out[0] = n->h / 10; out[1] = n->h % 10;
    out[2] = n->m / 10; out[3] = n->m % 10;
    out[4] = n->s / 10; out[5] = n->s % 10;
}

static const char *WEEKDAY[7] = { "SONNTAG", "MONTAG", "DIENSTAG", "MITTWOCH", "DONNERSTAG", "FREITAG", "SAMSTAG" };

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
static void face_analog(vp_dvg *d, float k)
{
    const now_t *n = &G.now;
    float cx = CX, cy = 2030, R = 1520;
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
    color(d, 235, 240, 255, k);
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
    /* the date window at three */
    char buf[8];
    snprintf(buf, sizeof buf, "%02d", n->mday);
    float wx = cx + R * 0.42f, wy = cy;
    color(d, 120, 200, 255, k * 0.7f);
    line(d, wx - 110, wy - 80, wx + 110, wy - 80); vp_lineto(d, (int)(wx + 110), (int)(wy + 80));
    vp_lineto(d, (int)(wx - 110), (int)(wy + 80)); vp_lineto(d, (int)(wx - 110), (int)(wy - 80));
    color(d, 235, 240, 255, k);
    text(d, buf, wx, wy - 45, 90, VP_TEXT_CENTER);
    /* the hands, sweeping */
    float sec = n->s + n->frac, min = n->m + sec / 60, hour = (n->h % 12) + min / 60;
    struct { float a, len, wid, r, g, b; } hand[2] = {
        { hour / 12 * 2 * PI, R * 0.55f, 70, 235, 240, 255 },
        { min / 60 * 2 * PI, R * 0.84f, 46, 235, 240, 255 },
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
    line(d, cx - c * R * 0.18f, cy - s * R * 0.18f, cx + c * R * 0.92f, cy + s * R * 0.92f);
    circle(d, cx, cy, 34, 12);
    circle(d, cx - c * R * 0.12f, cy - s * R * 0.12f, 40, 12);
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
    float x = CX - (4 * w + 3 * sp + 260 + 60 + 2 * w * 0.38f + 90) / 2;
    const float R = 60, G2 = 235, B = 255;
    for (int i = 0; i < 4; i++) {
        seg7(d, dg[i], x, y, w, h, k, R, G2, B);
        x += w + sp;
        if (i == 1) {                                /* the colon, standing still */
            color(d, R, G2, B, k);
            for (int j = 0; j < 2; j++) {
                float cy = y + h * (j ? 0.68f : 0.32f), cx = x + 40 + (cy - y) * 0.1f;
                circle(d, cx, cy, 38, 10);
                circle(d, cx, cy, 18, 8);
            }
            x += 260;
        }
    }
    seg7(d, dg[4], x - 40, y, w * 0.38f, h * 0.38f, k * 0.85f, R, G2, B);
    seg7(d, dg[5], x - 40 + w * 0.38f + 90, y, w * 0.38f, h * 0.38f, k * 0.85f, R, G2, B);
    char buf[48];
    snprintf(buf, sizeof buf, "%s  %02d.%02d.%04d", WEEKDAY[n->wday], n->mday, n->mon, n->year);
    color(d, 60, 235, 255, k * 0.75f * accent());
    vp_text_font(VP_FONT_ASTEROIDS);
    vp_text_fit(d, buf, CX, 1300, 150, W * 0.8f, VP_TEXT_CENTER);
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
    color(d, 245, 245, 250, k);
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
    float total = 6 * cw + 3 * gap + 2 * pair, x = CX - total / 2, y = 1750;
    for (int i = 0; i < 6; i++) {
        float sc = i >= 4 ? 0.62f : 1;              /* the seconds smaller */
        flip_card(d, i, x, y, cw * sc, ch * sc, k);
        x += cw * sc + (i & 1 ? pair : gap);
    }
    char buf[48];
    snprintf(buf, sizeof buf, "%s %02d.%02d.", WEEKDAY[G.now.wday], G.now.mday, G.now.mon);
    color(d, 200, 205, 215, k * 0.6f * accent());
    text(d, buf, CX, 1250, 120, VP_TEXT_CENTER);
}

/* bits: each digit a column, from the top 8 4 2 1 */
static void face_binary(vp_dvg *d, float k)
{
    const int NB[6] = { 2, 4, 3, 4, 3, 4 };
    const float step = 420, r = 140, pair = 260;
    float total = 5 * step + 2 * pair, x0 = CX - total / 2 + 120, y0 = 1350;
    color(d, 80, 255, 140, k * 0.4f * accent());
    const char *VAL[4] = { "1", "2", "4", "8" };
    for (int b = 0; b < 4; b++) text(d, VAL[b], x0 - 380, y0 + b * step - 45, 90, VP_TEXT_CENTER);
    float x = x0;
    for (int c = 0; c < 6; c++) {
        for (int b = 0; b < NB[c]; b++) {
            float y = y0 + b * step, on = G.bits[c][b];
            color(d, 80, 255, 140, k * (0.25f + 0.2f * accent()));
            circle(d, x, y, r, 24);
            if (on > 0.02f) {                       /* lit: filled with rings */
                color(d, 80, 255, 140, k * on);
                circle(d, x, y, r * 0.75f, 20);
                circle(d, x, y, r * 0.5f, 16);
                circle(d, x, y, r * 0.25f, 10);
            }
        }
        x += step + (c & 1 ? pair : 0);
    }
    /* the time in figures, small, under it */
    char buf[16];
    snprintf(buf, sizeof buf, "%02d:%02d:%02d", G.now.h, G.now.m, G.now.s);
    color(d, 80, 255, 140, k * 0.55f);
    text(d, buf, CX, 760, 130, VP_TEXT_CENTER);
}

/* the word clock: German, to five minutes, the minutes between as dots */
enum { ES, IST, M_FUENF, M_ZEHN, ZWANZIG, VIERTEL, VOR, NACH, HALB,
       H_EIN, H_S, ZWEI, DREI, VIER, H_FUENF, SECHS, SIEBEN, ACHT, NEUN, H_ZEHN, ELF, ZWOELF, UHR, NWORD };
static const struct { const char *s; int row, glue; } WORD[NWORD] = {
    { "ES", 0, 0 }, { "IST", 0, 0 }, { "FUENF", 0, 0 }, { "ZEHN", 0, 0 },
    { "ZWANZIG", 1, 0 }, { "VIERTEL", 1, 0 },
    { "VOR", 2, 0 }, { "NACH", 2, 0 }, { "HALB", 2, 0 },
    { "EIN", 3, 0 }, { "S", 3, 1 }, { "ZWEI", 3, 0 }, { "DREI", 3, 0 },
    { "VIER", 4, 0 }, { "FUENF", 4, 0 }, { "SECHS", 4, 0 },
    { "SIEBEN", 5, 0 }, { "ACHT", 5, 0 }, { "NEUN", 5, 0 },
    { "ZEHN", 6, 0 }, { "ELF", 6, 0 }, { "ZWOELF", 6, 0 },
    { "UHR", 7, 0 },
};

static void words_target(const now_t *n, float on[NWORD])
{
    for (int i = 0; i < NWORD; i++) on[i] = 0;
    on[ES] = on[IST] = 1;
    int five = n->m / 5, h = n->h % 12;
    if (five >= 5) h = (h + 1) % 12;                /* from 25 past: towards the next hour */
    switch (five) {
    case 0: on[UHR] = 1; break;
    case 1: on[M_FUENF] = on[NACH] = 1; break;
    case 2: on[M_ZEHN] = on[NACH] = 1; break;
    case 3: on[VIERTEL] = on[NACH] = 1; break;
    case 4: on[ZWANZIG] = on[NACH] = 1; break;
    case 5: on[M_FUENF] = on[VOR] = on[HALB] = 1; break;
    case 6: on[HALB] = 1; break;
    case 7: on[M_FUENF] = on[NACH] = on[HALB] = 1; break;
    case 8: on[ZWANZIG] = on[VOR] = 1; break;
    case 9: on[VIERTEL] = on[VOR] = 1; break;
    case 10: on[M_ZEHN] = on[VOR] = 1; break;
    case 11: on[M_FUENF] = on[VOR] = 1; break;
    }
    const int HOUR[12] = { ZWOELF, H_EIN, ZWEI, DREI, VIER, H_FUENF, SECHS, SIEBEN, ACHT, NEUN, H_ZEHN, ELF };
    on[HOUR[h]] = 1;
    if (HOUR[h] == H_EIN && five != 0) on[H_S] = 1;  /* EIN UHR, but FUENF NACH EINS */
}

static void face_words(vp_dvg *d, float k)
{
    const float h = 230, row_h = 400, top = 3500, space = 130;
    vp_text_font(VP_FONT_ASTEROIDS);
    for (int row = 0; row < 8; row++) {
        float width = 0;
        for (int i = 0; i < NWORD; i++)
            if (WORD[i].row == row) width += vp_text_width(WORD[i].s, h) + (WORD[i].glue || width == 0 ? 0 : space);
        float x = CX - width / 2, y = top - row * row_h;
        int first = 1;
        for (int i = 0; i < NWORD; i++) {
            if (WORD[i].row != row) continue;
            if (!first && !WORD[i].glue) x += space;
            first = 0;
            float on = G.words[i];
            color(d, 255, 190, 90, k * (0.1f + 0.9f * on));
            vp_text(d, WORD[i].s, x, y, h, VP_TEXT_LEFT);
            x += vp_text_width(WORD[i].s, h);
        }
    }
    /* the minutes past the five: dots */
    int extra = G.now.m % 5;
    for (int i = 0; i < 4; i++) {
        float x = CX + (i - 1.5f) * 260, y = top - 8 * row_h + 260;
        color(d, 255, 190, 90, k * (i < extra ? 1 : 0.12f));
        circle(d, x, y, 45, 12);
        if (i < extra) circle(d, x, y, 20, 8);
    }
}

/* nixie tubes */
static void face_nixie(vp_dvg *d, float k)
{
    const float tw = 470, th = 1150, gap = 70, pair = 230;
    float total = 6 * tw + 3 * gap + 2 * pair, x = CX - total / 2, y = 1500;
    for (int i = 0; i < 6; i++) {
        /* the glass: round at the top, the sockets' pins under it */
        float r = tw / 2;
        color(d, 180, 120, 80, k * 0.4f * accent());
        vp_moveto(d, (int)x, (int)y);
        vp_lineto(d, (int)x, (int)(y + th - r));
        arc(d, x + r, y + th - r, r, r, 180, 0, 16);
        vp_lineto(d, (int)(x + tw), (int)y);
        vp_lineto(d, (int)x, (int)y);
        for (int p = 0; p < 4; p++) line(d, x + tw * (0.2f + p * 0.2f), y - 10, x + tw * (0.2f + p * 0.2f), y - 90);
        /* the mesh in front, faint */
        color(d, 180, 120, 80, k * 0.15f);
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
        if (i == 1 || i == 3) {                                       /* the separators */
            float sx = x + pair / 2;
            color(d, 255, 130, 40, k * 0.8f);
            circle(d, sx, y + th * 0.33f, 30, 10);
            circle(d, sx, y + th * 0.6f, 30, 10);
            x += pair;
        } else if (i < 5) x += gap;
    }
    char buf[48];
    snprintf(buf, sizeof buf, "%02d.%02d.%04d", G.now.mday, G.now.mon, G.now.year);
    color(d, 255, 130, 40, k * 0.45f * accent());
    text(d, buf, CX, 1000, 120, VP_TEXT_CENTER);
}

/* hours, minutes, seconds as planets on their rings */
static void face_orbit(vp_dvg *d, float k)
{
    const now_t *n = &G.now;
    float cx = CX, cy = 2030;
    float sec = n->s + n->frac, min = n->m + sec / 60, hour = (n->h % 12) + min / 60;
    struct { float r, a, pr, cr, cg, cb; int ticks; } ring[3] = {
        { 650, hour / 12, 95, 255, 120, 220, 12 },
        { 1080, min / 60, 70, 120, 160, 255, 12 },
        { 1480, sec / 60, 45, 120, 230, 255, 60 },
    };
    for (int i = 0; i < 3; i++) {
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
    snprintf(buf, sizeof buf, "%02d:%02d", n->h, n->m);
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
    float sec = n->s + n->frac, min = n->m + sec / 60, hour = (n->h % 12) + min / 60;
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
    for (int i = 0; i < 7; i++) {
        float px = sx + (tx * CL[i][0] + nx * CL[i][1]) * 120, py = sy + (ty * CL[i][0] + ny * CL[i][1]) * 120;
        if (i == 0) vp_moveto(d, (int)px, (int)py);
        else        vp_lineto(d, (int)px, (int)py);
    }
    /* the time, far down the tube */
    char buf[8];
    snprintf(buf, sizeof buf, "%02d:%02d", n->h, n->m);
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
        ROCK[i].x = rock_rnd() * W;
        ROCK[i].y = rock_rnd() * W;
        float a = rock_rnd() * 2 * PI, v = 60 + 90 * rock_rnd();
        ROCK[i].vx = cosf(a) * v;
        ROCK[i].vy = sinf(a) * v;
        ROCK[i].a = rock_rnd() * 2 * PI;
        ROCK[i].spin = (rock_rnd() - 0.5f) * 0.5f;
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
        ROCK[i].x = fmodf(ROCK[i].x + ROCK[i].vx * dt + W + 600, W + 600) - 300;   /* round the edges */
        ROCK[i].y = fmodf(ROCK[i].y + ROCK[i].vy * dt + W + 600, W + 600) - 300;
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
        color(d, 200, 200, 210, k * 0.45f * accent());
        for (int j = 0; j <= 10; j++) {
            float a = ROCK[i].a + j * 2 * PI / 10, rr = ROCK[i].size * ROCK[i].rad[j % 10];
            int x = (int)(ROCK[i].x + cosf(a) * rr), y = (int)(ROCK[i].y + sinf(a) * rr);
            if (x < 0 || x > W || y < 0 || y > W) { j = 11; break; }
            if (j == 0) vp_moveto(d, x, y);
            else        vp_lineto(d, x, y);
        }
    }
    /* the time */
    const float h = 900, w = h * 0.6f, y = 1600;
    color(d, 240, 240, 250, k);
    glyph(d, n->h / 10, CX - 140 - 2 * w - 170, y, w, h);
    glyph(d, n->h % 10, CX - 140 - w, y, w, h);
    circle(d, CX, y + h * 0.3f, 22, 8);
    circle(d, CX, y + h * 0.7f, 22, 8);
    glyph(d, n->m / 10, CX + 140, y, w, h);
    glyph(d, n->m % 10, CX + 140 + w + 170, y, w, h);
    /* the pieces of the last minute */
    for (int i = 0; i < NFRAG; i++) {
        if (FRAG[i].life <= 0) continue;
        color(d, 240, 240, 250, k * smooth01(FRAG[i].life));
        line(d, FRAG[i].x0, FRAG[i].y0, FRAG[i].x1, FRAG[i].y1);
    }
    /* the ship, gliding round the time, nose ahead (the seconds: once a minute) */
    float sec = n->s + n->frac, a = PI / 2 - sec / 60 * 2 * PI;
    float sx = CX + cosf(a) * 1750, sy = y + h / 2 + sinf(a) * 1250;
    float hx = sinf(a) * 1750, hy = -cosf(a) * 1250, hl = sqrtf(hx * hx + hy * hy);
    hx /= hl; hy /= hl;                                          /* its heading: along the path */
    float px = -hy, py = hx, s = 70;
    color(d, 240, 240, 250, k);
    vp_moveto(d, (int)(sx + hx * s * 1.6f), (int)(sy + hy * s * 1.6f));
    vp_lineto(d, (int)(sx - hx * s + px * s), (int)(sy - hy * s + py * s));
    vp_lineto(d, (int)(sx - hx * s * 0.4f), (int)(sy - hy * s * 0.4f));
    vp_lineto(d, (int)(sx - hx * s - px * s), (int)(sy - hy * s - py * s));
    vp_lineto(d, (int)(sx + hx * s * 1.6f), (int)(sy + hy * s * 1.6f));
    /* the seconds, small, under it all */
    char buf[8];
    snprintf(buf, sizeof buf, "%02d", n->s);
    color(d, 240, 240, 250, k * 0.6f);
    text(d, buf, CX, 1150, 110, VP_TEXT_CENTER);
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

static const char *WEEKDAY_SHORT[7] = { "SO", "MO", "DI", "MI", "DO", "FR", "SA" };

static void face_weather(vp_dvg *d, float k)
{
    const weather_t *w = &G.wx;
    const now_t *n = &G.now;
    char buf[96];
    if (!w->ok) {
        color(d, 200, 215, 240, k * 0.7f);
        text(d, "KEIN WETTER", CX, 2300, 160, VP_TEXT_CENTER);
        color(d, 200, 215, 240, k * 0.45f);
        text(d, "ORT IN CLOCK.INI, NETZ NOETIG", CX, 2000, 90, VP_TEXT_CENTER);
        snprintf(buf, sizeof buf, "%s %02d.%02d.%04d   %02d:%02d", WEEKDAY[n->wday], n->mday, n->mon, n->year, n->h, n->m);
        text(d, buf, CX, 1500, 110, VP_TEXT_CENTER);
        return;
    }
    /* the place, the time */
    color(d, 200, 215, 240, k * 0.8f);
    snprintf(buf, sizeof buf, "%s   %02d:%02d", w->place, n->h, n->m);
    vp_text_font(VP_FONT_ASTEROIDS);
    vp_text_fit(d, buf, CX, 3550, 130, W * 0.86f, VP_TEXT_CENTER);
    /* now: the picture, the temperature, the words */
    weather_icon(d, w->code, w->is_day, CX - 900, 2600, 600, k);
    color(d, 245, 245, 250, k);
    float th = 800, tw = temperature(d, w->temp, 0, 0, th, 0);
    temperature(d, w->temp, CX + 450 - tw / 2, 2250, th, 1);
    color(d, 200, 215, 240, k * 0.85f);
    text(d, weather_words(w->code), CX + 450, 2000, 120, VP_TEXT_CENTER);
    snprintf(buf, sizeof buf, "WIND %d KM/H   FEUCHTE %d%%", (int)lroundf(w->wind), (int)lroundf(w->humidity));
    color(d, 200, 215, 240, k * 0.6f * accent());
    text(d, buf, CX, 1700, 90, VP_TEXT_CENTER);
    /* the next days */
    for (int i = 0; i < w->days && i < 4; i++) {
        float x = CX + (i - 1.5f) * 900, y = 1050;
        color(d, 200, 215, 240, k * 0.75f);
        text(d, i == 0 ? "HEUTE" : WEEKDAY_SHORT[(n->wday + i) % 7], x, y + 330, 90, VP_TEXT_CENTER);
        weather_icon(d, w->day_code[i], 1, x, y + 120, 170, k * 0.9f);
        snprintf(buf, sizeof buf, "%d / %d", (int)lroundf(w->day_max[i]), (int)lroundf(w->day_min[i]));
        color(d, 245, 245, 250, k * 0.8f);
        text(d, buf, x, y - 200, 90, VP_TEXT_CENTER);
    }
}

/* under the other faces: the date and the weather, one line */
static void info_line(vp_dvg *d, float k)
{
    const now_t *n = &G.now;
    char buf[160];
    if (G.wx.ok)
        snprintf(buf, sizeof buf, "%s %02d.%02d.   %s  %d GRAD  %s", WEEKDAY_SHORT[n->wday], n->mday, n->mon,
                 G.wx.place, (int)lroundf(G.wx.temp), weather_words(G.wx.code));
    else
        snprintf(buf, sizeof buf, "%s %02d.%02d.%04d", WEEKDAY[n->wday], n->mday, n->mon, n->year);
    color(d, 200, 215, 240, k * 0.55f);
    vp_text_font(VP_FONT_ASTEROIDS);
    vp_text_fit(d, buf, CX, 360, 90, W * 0.86f, VP_TEXT_CENTER);
}

static void face_draw(vp_dvg *d, int f, float k)
{
    if (k <= 0.01f) return;
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
    case FACE_WEATHER: face_weather(d, k); break;
    }
    if (f != FACE_WEATHER) info_line(d, k);
}

/* ---- the game --------------------------------------------------------------- */

static void face_switch(int f)
{
    G.from = G.face;
    G.face = (f % NFACE + NFACE) % NFACE;
    G.blend = 0;
    G.name_time = 3.5f;
}

static int init(vp_game *g)
{
    (void)g;
    glyphs_make();
    const char *f = getenv("SAVER_FACE"), *c = getenv("SAVER_CYCLE");
    G.face = G.from = f ? ((atoi(f) % NFACE) + NFACE) % NFACE : 0;
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
    float on[NWORD];
    words_target(&G.now, on);
    for (int i = 0; i < NWORD; i++) G.words[i] = on[i];
    return VP_OK;
}

static void update(vp_game *g, float dt)
{
    G.time += dt;
    G.now = read_now();
    weather_get(&G.wx);
    G.have = vp_saver_music(&G.m);
    if (G.have && G.m.track != G.last_track) {
        if (G.started) face_switch(G.face + 1);      /* a new track: the next face */
        G.started = 1;
        G.last_track = G.m.track;
        snprintf(G.title, sizeof G.title, "%s", G.m.title);
        G.title_time = G.title[0] ? 7 : 0;
    }
    vp_input *in = vp_game_input(g);                 /* on a PC: the arrow keys */
    if (in && vp_input_pressed(in, "key_Right")) face_switch(G.face + 1);
    if (in && vp_input_pressed(in, "key_Left"))  face_switch(G.face - 1);
    if (G.cycle > 0 && (G.cycle_t += dt) >= G.cycle) { G.cycle_t = 0; face_switch(G.face + 1); }

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
    words_target(&G.now, on);
    for (int i = 0; i < NWORD; i++) G.words[i] += (on[i] - G.words[i]) * fminf(1, dt * 2.5f);
    rocks_update(dt);
}

/* The track's title over the clock, then gone. */
static void title(vp_dvg *d)
{
    if (G.title_time <= 0) return;
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
    float t = smooth01(G.blend);
    if (G.from != G.face) face_draw(d, G.from, 1 - t);
    face_draw(d, G.face, t);
    if (G.name_time > 0) {                           /* the face's name, a while */
        float k = smooth01(G.name_time / 1) * smooth01((3.5f - G.name_time) / 0.6f);
        color(d, 200, 210, 230, k * 0.6f);
        text(d, FACE_NAME[G.face], CX, 170, 70, VP_TEXT_CENTER);
    }
    title(d);
    shots(d);
}

int main(int argc, char **argv)
{
    weather_start(argc > 0 ? argv[0] : NULL);
    vp_game_desc desc = {0};
    desc.title = "Clock";
    desc.init = init;
    desc.update = update;
    desc.draw = draw;
    return vp_run(&desc, argc, argv);
}
