/* Constellations — a VectorPie screensaver: a slow tour of the real night
 * sky. The view glides from one constellation to the next, each near the
 * last; there its figure draws itself in, its German name (and the Latin one)
 * appears under it, its bright stars are named; then on to the next.
 *
 * The stars of the figures and the bright ones beside them are real (sky.h:
 * their places, brightness and color). Behind them many faint stars, thicker
 * along the Milky Way, where it really runs (their places there are made up).
 * All stars twinkle, softly, each in its own time; now and then a shooting
 * star.
 *
 * To the music, smoothly: when it is louder the tour goes a little faster,
 * the stars twinkle livelier and shooting stars come more often. Nothing
 * flashes or pulses. A new track shows its title.
 *
 * On the cabinet: installed in SETTINGS -> SCREENSAVERS (tools/deploy.sh or
 * the .vps). Any control wakes the cabinet, START 2 skips to the next track.
 * On a PC: saver_constellations <a .ogg/.wav or a folder of them>. Esc quits.
 * SAVER_SHOTS=<folder> saves a picture every 2 s; SAVER_TEMPO=<factor> runs
 * the tour that many times faster (both for development).
 */
#include <vectorpie/vp.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sky.h"

#define W         VP_DVG_COORD_MAX
#define CX        (W / 2.0f)
#define CY        (W * 0.56f)               /* the sky's middle: the names go below */
#define PI        3.14159265f
#define DEG       (PI / 180)
#define FAINT_N   1500                      /* faint stars all round */
#define MAX_STARS 2000
#define LINE_GAP  45.0f                     /* figure lines stop short of their stars */
#define FAINT_FIG 0.16f                     /* the other figures, faintly */
#define DRAW_S    4.0f                      /* a figure draws itself in, s */
#define HOLD_S    11.0f                     /* and stays */
#define LEAVE_S   2.5f                      /* and fades as the view leaves */

typedef struct { float x, y, z; } v3;

/* every star: its direction, brightness, tint, twinkle */
static struct {
    v3 v;
    float mag, rate, phase;
    char tint;
    int con, idx;                           /* its figure and place there, or -1 */
} S[MAX_STARS];
static int NS;
static v3 CON_MID[SKY_NCON];                /* each figure's middle */
static float CON_SIZE[SKY_NCON];            /* and its radius, radians */
static int CON_STAR[SKY_NCON][24];          /* index in S of each of its stars */

static struct {
    float time, loud, tempo, twinkle;       /* twinkle: the twinkling's own clock */
    int from, to;                           /* the figures the view goes between */
    float glide, glide_s;                   /* how far along the glide, its length */
    float fig, hold;                        /* the figure drawn in 0..1, then held (s) */
    float leave_k;                          /* the last figure, fading */
    int leaving;
    v3 mid;                                 /* the view: its middle */
    float half;                             /* and half its width, radians */
    struct { int on; float t, x, y, dx, dy; } meteor;
    vp_music_feed m;
    int have;
    unsigned last_track;
    float title_time;
    char title[VP_MUSIC_TEXT];
    uint32_t rnd;
} G;

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
static float smooth01(float t) { t = clampf(t, 0, 1); return t * t * (3 - 2 * t); }

static float hash01(uint32_t i)
{
    i *= 2654435761u;
    i ^= i >> 15;
    i *= 2246822519u;
    i ^= i >> 13;
    return (float)(i & 0xffffff) / 16777215.0f;
}

static float rnd01(void)
{
    G.rnd = G.rnd * 1664525u + 1013904223u;
    return (float)(G.rnd >> 8) / 16777215.0f;
}

static v3 v3_make(float x, float y, float z) { v3 v = { x, y, z }; return v; }
static float dot(v3 a, v3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static v3 cross(v3 a, v3 b) { return v3_make(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
static v3 scale(v3 a, float k) { return v3_make(a.x * k, a.y * k, a.z * k); }
static v3 add(v3 a, v3 b) { return v3_make(a.x + b.x, a.y + b.y, a.z + b.z); }
static v3 norm(v3 a) { float l = sqrtf(dot(a, a)); return l > 0 ? scale(a, 1 / l) : v3_make(1, 0, 0); }

/* a direction on the sky from right ascension (hours) and declination (degrees) */
static v3 radec(float ra, float dec)
{
    float a = ra * 15 * DEG, d = dec * DEG;
    return v3_make(cosf(d) * cosf(a), cosf(d) * sinf(a), sinf(d));
}

static void color(vp_dvg *d, float r, float g, float b, float k)
{
    k = clampf(k, 0, 1);
    vp_color(d, (uint8_t)clampf(r * k, 0, 255), (uint8_t)clampf(g * k, 0, 255), (uint8_t)clampf(b * k, 0, 255));
}

/* ---- the sky -------------------------------------------------------------- */

static void add_star(v3 v, float mag, char tint, int con, int idx)
{
    if (NS >= MAX_STARS) return;
    S[NS].v = v;
    S[NS].mag = mag;
    S[NS].tint = tint;
    S[NS].con = con;
    S[NS].idx = idx;
    S[NS].rate = 1.2f + 2.4f * hash01((uint32_t)NS * 7 + 5);     /* a wave every 1.7 to 5 s */
    S[NS].phase = 2 * PI * hash01((uint32_t)NS * 7 + 6);
    NS++;
}

static void sky_make(void)
{
    for (int c = 0; c < SKY_NCON; c++) {
        const sky_con *k = &SKY_CON[c];
        v3 sum = v3_make(0, 0, 0);
        for (int i = 0; i < k->n; i++) {
            v3 v = radec(k->s[i].ra, k->s[i].dec);
            sum = add(sum, v);
            CON_STAR[c][i] = NS;
            if (k->s[i].shared) {               /* drawn with the other figure: here a place only */
                CON_STAR[c][i] = -1 - c;        /* (found below) */
                continue;
            }
            add_star(v, k->s[i].mag, k->s[i].tint, c, i);
        }
        CON_MID[c] = norm(sum);
        float far = 0;
        for (int i = 0; i < k->n; i++)
            far = fmaxf(far, acosf(clampf(dot(CON_MID[c], radec(k->s[i].ra, k->s[i].dec)), -1, 1)));
        CON_SIZE[c] = far;
    }
    /* the shared stars: the same star in the other figure */
    for (int c = 0; c < SKY_NCON; c++)
        for (int i = 0; i < SKY_CON[c].n; i++) {
            if (CON_STAR[c][i] >= 0) continue;
            v3 v = radec(SKY_CON[c].s[i].ra, SKY_CON[c].s[i].dec);
            for (int j = 0; j < NS; j++)
                if (dot(S[j].v, v) > 0.99999f) { CON_STAR[c][i] = j; break; }
        }
    for (int i = 0; i < SKY_NLONE; i++)
        add_star(radec(SKY_LONE[i].ra, SKY_LONE[i].dec), SKY_LONE[i].mag, SKY_LONE[i].tint, -1, -1);

    /* faint stars: two in three along the Milky Way, where it really runs
     * (the galaxy's north pole and middle), thicker towards its middle */
    v3 gnp = radec(12.8573f, 27.128f), gc = radec(17.7611f, -28.936f);
    v3 gx = norm(add(gc, scale(gnp, -dot(gc, gnp)))), gy = cross(gnp, gx);
    uint32_t h = 9001;
    for (int i = 0; i < FAINT_N; i++, h += 3) {
        v3 v;
        if (i % 3 != 2) {
            float l = 2 * PI * hash01(h), u = hash01(h + 1) * 0.98f + 0.01f;
            if (hash01(h + 2) < 0.4f) l = (hash01(h + 2) - 0.2f) * 2.5f * PI / 2;   /* the middle */
            float b = 7 * DEG * sqrtf(-2 * logf(u)) * cosf(2 * PI * hash01(h + 5));
            v = add(add(scale(gx, cosf(b) * cosf(l)), scale(gy, cosf(b) * sinf(l))), scale(gnp, sinf(b)));
        } else {
            float z = hash01(h) * 2 - 1, a = 2 * PI * hash01(h + 1), r = sqrtf(1 - z * z);
            v = v3_make(r * cosf(a), r * sinf(a), z);
        }
        float mag = 4.4f + 2.2f * sqrtf(hash01(h + 7));
        add_star(v, mag, hash01(h + 8) < 0.3f ? 'b' : hash01(h + 8) > 0.85f ? 'y' : 0, -1, -1);
    }
}

/* ---- the view -------------------------------------------------------------- */

static v3 VE, VN;                           /* the view's east (to the left) and north (up) */
static float ASPECT_X = 0.75f, ASPECT_Y = 1; /* the board's square stretched to the 4:3 tube (3:4 vertical) */
static float VS;                            /* board units per stereographic unit */
static float VCOS;                          /* stars farther from the middle are not drawn */

static void view_set(void)
{
    v3 pole = v3_make(0, 0, 1);
    VE = norm(cross(pole, G.mid));
    VN = cross(G.mid, VE);
    VS = (W * 0.5f) / (2 * tanf(G.half / 2));
    VCOS = cosf(fminf(G.half * 1.5f, 1.4f));
}

/* where v is on the screen, stereographic (shapes stay true); 0: not in view */
static int project(v3 v, float *x, float *y)
{
    float z = dot(v, G.mid);
    if (z < VCOS) return 0;
    float k = 2 / (1 + z);
    *x = CX - VS * k * dot(v, VE) * ASPECT_X;    /* the tube is 4:3: across a quarter less, so shapes stay true */
    *y = CY + VS * k * dot(v, VN) * ASPECT_Y;
    return 1;
}

/* a line, cut to the screen */
static void seg(vp_dvg *d, float x0, float y0, float x1, float y1)
{
    float t0 = 0, t1 = 1, dx = x1 - x0, dy = y1 - y0;
    float p[4] = { -dx, dx, -dy, dy }, q[4] = { x0, W - x0, y0, W - y0 };
    for (int i = 0; i < 4; i++) {
        if (p[i] == 0) { if (q[i] < 0) return; continue; }
        float r = q[i] / p[i];
        if (p[i] < 0) { if (r > t1) return; if (r > t0) t0 = r; }
        else          { if (r < t0) return; if (r < t1) t1 = r; }
    }
    vp_moveto(d, (int)(x0 + dx * t0), (int)(y0 + dy * t0));
    vp_lineto(d, (int)(x0 + dx * t1), (int)(y0 + dy * t1));
}

/* ---- the tour --------------------------------------------------------------- */

static float con_half(int c) { return clampf(CON_SIZE[c] * 1.2f + 4 * DEG, 10 * DEG, 36 * DEG); }

static void tour_next(void)
{
    G.leaving = G.to;
    G.leave_k = 1;
    G.from = G.to;
    G.to = (G.to + 1) % SKY_NCON;
    G.glide = 0;
    float angle = acosf(clampf(dot(CON_MID[G.from], CON_MID[G.to]), -1, 1));
    G.glide_s = clampf(4 + angle / DEG / 6, 5, 11);
    G.fig = 0;
    G.hold = 0;
}

static void tour_update(float dt)
{
    float pace = (1 + 0.5f * G.loud) * G.tempo;
    if (G.leave_k > 0) G.leave_k = fmaxf(0, G.leave_k - dt * G.tempo / LEAVE_S);
    if (G.glide < 1) {
        G.glide = fminf(1, G.glide + dt * G.tempo / G.glide_s);
        float t = smooth01(G.glide);
        /* along the great circle between the two, a little farther out on the way */
        v3 a = CON_MID[G.from], b = CON_MID[G.to];
        float om = acosf(clampf(dot(a, b), -1, 1));
        v3 m = om < 1e-4f ? b : add(scale(a, sinf((1 - t) * om) / sinf(om)), scale(b, sinf(t * om) / sinf(om)));
        G.mid = norm(m);
        float h0 = con_half(G.from), h1 = con_half(G.to);
        G.half = h0 + (h1 - h0) * t + sinf(PI * t) * fminf(om * 0.25f, 10 * DEG);
    } else if (G.fig < 1) {
        G.fig = fminf(1, G.fig + dt * pace / DRAW_S);
    } else if ((G.hold += dt * pace) >= HOLD_S) {
        tour_next();
    }
    /* held: the view creeps in a little */
    if (G.glide >= 1) G.half = con_half(G.to) * (1 - 0.06f * smooth01((G.fig + G.hold / HOLD_S) / 2));
}

/* ---- drawing ------------------------------------------------------------- */

static void tint_rgb(char tint, float *r, float *g, float *b)
{
    switch (tint) {
    case 'r': *r = 255; *g = 150; *b = 90; break;
    case 'y': *r = 255; *g = 225; *b = 160; break;
    case 'b': *r = 175; *g = 200; *b = 255; break;
    default:  *r = 235; *g = 235; *b = 255; break;
    }
}

typedef struct { float x, y, k; int i; long key; } vis;
static vis VIS[MAX_STARS];

static int vis_cmp(const void *a, const void *b)
{
    long ka = ((const vis *)a)->key, kb = ((const vis *)b)->key;
    return ka < kb ? -1 : ka > kb;
}

static void stars_draw(vp_dvg *d)
{
    /* those in view, in bands across, back and forth: a short way for the beam
     * (the order changes as the view moves; each star is drawn the same way) */
    int n = 0;
    const int BAND = 300;
    for (int i = 0; i < NS; i++) {
        float x, y;
        if (!project(S[i].v, &x, &y) || x < -20 || x > W + 20 || y < -20 || y > W + 20) continue;
        float base = clampf(1.0f - 0.11f * S[i].mag, 0.22f, 1);
        float w = S[i].rate * G.twinkle + S[i].phase;
        float tw = 0.5f + 0.3f * sinf(w) + 0.2f * sinf(w * 1.73f + 2.1f);       /* 0..1, smooth */
        float depth = S[i].mag < 1.5f ? 0.3f : 0.55f;                            /* the bright twinkle less */
        int band = (int)(y / BAND);
        VIS[n].x = x;
        VIS[n].y = y;
        VIS[n].k = base * (1 - depth + depth * tw * tw);
        VIS[n].i = i;
        VIS[n].key = (long)band * 8192 + (band & 1 ? (long)(W - x) : (long)x);
        n++;
    }
    qsort(VIS, (size_t)n, sizeof VIS[0], vis_cmp);
    for (int j = 0; j < n; j++) {
        float r, g, b, x = VIS[j].x, y = VIS[j].y;
        tint_rgb(S[VIS[j].i].tint, &r, &g, &b);
        color(d, r, g, b, VIS[j].k);
        float mag = S[VIS[j].i].mag;
        if (mag < 2.6f) {                                  /* the bright: a little cross */
            float a = mag < 0.6f ? 16 : mag < 1.6f ? 11 : 7;
            seg(d, x - a * ASPECT_X, y, x + a * ASPECT_X, y);
            seg(d, x, y - a * ASPECT_Y, x, y + a * ASPECT_Y);
        } else {
            seg(d, x - 3, y, x + 3, y);
        }
    }
}

/* a figure's lines: the part `drawn` (0..1, along its path) bright, the rest faint */
static void figure_draw(vp_dvg *d, int c, float drawn, float k)
{
    const sky_con *f = &SKY_CON[c];
    float total = 0;
    for (int p = 0; p + 1 < f->npath; p++)
        if (f->path[p] >= 0 && f->path[p + 1] >= 0) total++;
    float upto = drawn * total, done = 0;
    for (int p = 0; p + 1 < f->npath; p++) {
        int a = f->path[p], b = f->path[p + 1];
        if (a < 0 || b < 0) continue;
        int sa = CON_STAR[c][a], sb = CON_STAR[c][b];
        float x0, y0, x1, y1;
        float part = clampf(upto - done, 0, 1);
        done++;
        if (sa < 0 || sb < 0 || !project(S[sa].v, &x0, &y0) || !project(S[sb].v, &x1, &y1)) continue;
        float dx = x1 - x0, dy = y1 - y0, len = sqrtf(dx * dx + dy * dy);
        if (len < LINE_GAP * 2.5f) continue;
        float ux = dx / len, uy = dy / len;
        x0 += ux * LINE_GAP; y0 += uy * LINE_GAP;           /* short of the stars */
        x1 -= ux * LINE_GAP; y1 -= uy * LINE_GAP;
        float xm = x0 + (x1 - x0) * part, ym = y0 + (y1 - y0) * part;
        if (part > 0) {
            color(d, 90, 150, 255, fmaxf(FAINT_FIG, k));
            seg(d, x0, y0, xm, ym);
        }
        if (part < 1) {
            color(d, 90, 150, 255, FAINT_FIG);
            seg(d, xm, ym, x1, y1);
        }
    }
}

static void names_draw(vp_dvg *d, int c, float k)
{
    if (k <= 0.01f) return;
    vp_text_font(VP_FONT_ASTEROIDS);
    const sky_con *f = &SKY_CON[c];
    /* its bright stars */
    color(d, 200, 215, 255, k * 0.75f);
    for (int i = 0; i < f->n; i++) {
        int s = CON_STAR[c][i];
        float x, y;
        if (!f->s[i].name || s < 0 || !project(S[s].v, &x, &y)) continue;
        if (x < 0 || x > W - 200 || y < 40 || y > W - 60) continue;
        vp_text(d, f->s[i].name, x + 45, y - 70, 50, VP_TEXT_LEFT);
    }
    /* its names, under the sky */
    color(d, 255, 215, 140, k);
    vp_text_fit(d, f->name, CX, 330, 125, W * 0.8f, VP_TEXT_CENTER);
    if (f->latin[0] && strcmp(f->latin, f->name) != 0) {
        color(d, 170, 190, 255, k * 0.8f);
        vp_text_fit(d, f->latin, CX, 170, 70, W * 0.6f, VP_TEXT_CENTER);
    }
}

/* now and then a shooting star: a short streak sliding down, in and out smoothly */
static void meteor(vp_dvg *d, float dt)
{
    if (!G.meteor.on) {
        if (rnd01() < dt * G.tempo * (1 + 1.5f * G.loud) / 30) {
            G.meteor.on = 1;
            G.meteor.t = 0;
            G.meteor.x = W * (0.15f + 0.7f * rnd01());
            G.meteor.y = W * (0.6f + 0.3f * rnd01());
            float a = (rnd01() < 0.5f ? -1 : 1) * (0.35f + 0.5f * rnd01());
            G.meteor.dx = sinf(a);
            G.meteor.dy = -cosf(a);
        }
        return;
    }
    const float LIFE = 1.1f, RUN = 1300, TAIL = 380;
    G.meteor.t += dt / LIFE;
    if (G.meteor.t >= 1) { G.meteor.on = 0; return; }
    float t = G.meteor.t, head = RUN * t, k = sinf(PI * t);
    float hx = G.meteor.x + G.meteor.dx * head, hy = G.meteor.y + G.meteor.dy * head;
    float tail = TAIL * (0.4f + 0.6f * k);
    color(d, 230, 235, 255, k * 0.8f);
    seg(d, hx - G.meteor.dx * tail, hy - G.meteor.dy * tail, hx, hy);
}

/* ---- the game --------------------------------------------------------------- */

static int init(vp_game *g)
{
    (void)g;
    sky_make();
    const char *tempo = getenv("SAVER_TEMPO");
    G.tempo = tempo ? clampf((float)atof(tempo), 0.1f, 50) : 1;
    G.rnd = (uint32_t)vp_ticks_ms() * 2654435761u + 1;
    G.to = (int)(rnd01() * SKY_NCON) % SKY_NCON;            /* somewhere else each time */
    G.from = G.to;
    G.leaving = -1;
    G.glide = 1;
    if (getenv("VP_VERTICAL") && atoi(getenv("VP_VERTICAL"))) { ASPECT_X = 1; ASPECT_Y = 0.75f; }
    G.mid = CON_MID[G.to];
    G.half = con_half(G.to);
    return VP_OK;
}

static float last_dt;

static void update(vp_game *g, float dt)
{
    (void)g;
    last_dt = dt;
    G.time += dt;
    G.have = vp_saver_music(&G.m);
    if (G.have && G.m.track != G.last_track) {
        G.last_track = G.m.track;
        snprintf(G.title, sizeof G.title, "%s", G.m.title);
        G.title_time = G.title[0] ? 7 : 0;
    }
    G.title_time = fmaxf(0, G.title_time - dt);
    G.loud += ((G.have ? G.m.level : 0) - G.loud) * fminf(1, dt * 0.4f);   /* slowed right down */
    G.twinkle += dt * (1 + 0.6f * G.loud);
    tour_update(dt);
}

/* The track's title over the sky, then gone. */
static void title(vp_dvg *d)
{
    if (G.title_time <= 0) return;
    float k = smooth01(G.title_time / 1.5f) * smooth01((7 - G.title_time) / 0.5f);
    vp_text_font(VP_FONT_ASTEROIDS);
    color(d, 200, 215, 255, k * 0.8f);
    vp_text_fit(d, G.title, CX, W * 0.94f, 90, W * 0.84f, VP_TEXT_CENTER);
}

/* SAVER_SHOTS=<folder>: a picture every 2 s, for development. */
static void shots(vp_dvg *d)
{
    static const char *dir;
    static int checked, n;
    static float next = 3;
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
    view_set();
    stars_draw(d);
    for (int c = 0; c < SKY_NCON; c++) {
        if (c == G.to)           figure_draw(d, c, G.glide >= 1 ? smooth01(G.fig) : 0, 1);
        else if (c == G.leaving) figure_draw(d, c, 1, G.leave_k);
        else                     figure_draw(d, c, 0, 0);
    }
    if (G.leaving >= 0 && G.leaving != G.to) names_draw(d, G.leaving, smooth01(G.leave_k));
    if (G.glide >= 1) names_draw(d, G.to, smooth01(G.fig * 1.6f - 0.3f));
    meteor(d, last_dt);
    title(d);
    shots(d);
}

int main(int argc, char **argv)
{
    vp_game_desc desc = {0};
    desc.title = "Constellations";
    desc.init = init;
    desc.update = update;
    desc.draw = draw;
    return vp_run(&desc, argc, argv);
}
