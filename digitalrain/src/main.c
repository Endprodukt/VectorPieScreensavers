/* Digital Rain — a VectorPie screensaver: green code raining down the
 * tube, and now and then the name of one of the cabinet's games in it.
 *
 * Columns of glyphs fall: each drop a bright head and a trail fading behind
 * it; now and then a glyph quietly changes. The glyphs are our own, a few
 * strokes each, in the manner of half-width katakana and digits, some
 * mirrored.
 *
 * Every 20 to 40 s one name shows, from the cabinet's game list (its
 * classics in advance's gamelist.ini, its own games' game.ini) or VECTORPIE,
 * one of three ways:
 *   SLIDE   it glides across the screen, big; the rain makes way for it
 *   FALL    it falls down a column as a drop of its letters
 *   DECODE  a row of glyphs locks into its letters one by one, holds, and
 *           comes apart again
 *
 * To the music, smoothly: the rain falls faster when it is louder. Nothing
 * pulses or blinks.
 *
 * On the cabinet: installed in SETTINGS -> SCREENSAVERS (tools/deploy.sh or
 * the .vps). Any control wakes the cabinet, START 2 skips to the next track.
 * On a PC: saver_digitalrain <a .ogg/.wav or a folder of them>. Esc quits.
 * SAVER_GAMELIST=<file> reads that game list instead of the cabinet's (to
 * try it on a PC); SAVER_SHOTS=<folder> saves a picture every 2 s.
 */
#include <vectorpie/vp.h>

#include <ctype.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W          VP_DVG_COORD_MAX
#define COLS       22
#define ROWS       20
#define MARGIN     90.0f
#define CELL_W     ((W - 2 * MARGIN) / COLS)
#define CELL_H     ((W - 2 * MARGIN) / ROWS)
#define GLYPH_W    (CELL_W * 0.62f)
#define GLYPH_H    (CELL_H * 0.70f)
#define DROPS      2                    /* at most, per column */
#define NAMES      160
#define NAME_LEN   40
#define GAMELIST   "/usr/local/share/advance/gamelist.ini"
#define GAMES_DIR  "/persistent/games"

/* ---- the glyphs: strokes in a cell 0..1 across, 0..1 up ---------------- */

typedef struct { int n; float s[5][4]; } glyph;

static const glyph GLYPHS[] = {
    { 2, { { 0, 1, 1, 1 }, { 1, 1, 1, 0 } } },
    { 2, { { .4f, .8f, 0, .1f }, { .6f, .8f, 1, .1f } } },
    { 3, { { .2f, .9f, .8f, .75f }, { .2f, .55f, .8f, .4f }, { .1f, .2f, .9f, 0 } } },
    { 3, { { .1f, .85f, .3f, .75f }, { .1f, .55f, .3f, .45f }, { .1f, 0, 1, .8f } } },
    { 3, { { .2f, 1, .2f, .4f }, { .8f, 1, .8f, .3f }, { .8f, .3f, .4f, 0 } } },
    { 3, { { 0, 1, 1, 1 }, { 0, .6f, 1, .6f }, { 1, 1, .3f, 0 } } },
    { 3, { { .1f, 1, .9f, 1 }, { .5f, 1, .5f, 0 }, { 0, 0, 1, 0 } } },
    { 4, { { .1f, 1, .9f, 1 }, { 0, .6f, 1, .6f }, { .5f, 1, .5f, .1f }, { .5f, .1f, 1, 0 } } },
    { 3, { { .1f, .9f, .25f, .7f }, { .45f, .95f, .55f, .75f }, { .95f, .9f, .3f, 0 } } },
    { 3, { { 0, .7f, 1, .7f }, { .5f, 1, .5f, .4f }, { .5f, .4f, .2f, 0 } } },
    { 4, { { .2f, 0, .8f, 0 }, { .8f, 0, .8f, 1 }, { .8f, 1, .2f, 1 }, { .2f, 1, .2f, 0 } } },
    { 2, { { .5f, 0, .5f, 1 }, { .5f, 1, .25f, .8f } } },
    { 3, { { .1f, 1, .9f, 1 }, { 0, .65f, 1, .65f }, { 1, .65f, .4f, 0 } } },
    { 3, { { .4f, 1, 0, .5f }, { .4f, 1, 1, 1 }, { 1, 1, .3f, 0 } } },
    { 3, { { 0, 1, 1, 1 }, { 1, 1, 1, 0 }, { 1, 0, 0, 0 } } },
    { 3, { { .2f, 1, .2f, 0 }, { .2f, .5f, .9f, .6f }, { .2f, 0, 1, 0 } } },
};
#define NGLYPHS ((int)(sizeof GLYPHS / sizeof GLYPHS[0]))

/* ---- the rain ------------------------------------------------------------ */

typedef struct { int active; float head, speed; int len; } drop;   /* head: rows from the top */

static struct {
    drop    drops[DROPS];
    uint8_t glyph[ROWS];                /* each cell's glyph; bit 7: mirrored */
    float   next;                       /* s to the next drop */
} C[COLS];

/* ---- the names ------------------------------------------------------------ */

enum { SLIDE, FALL, DECODE, MODES };

static char names[NAMES][NAME_LEN];
static int  nnames;

static struct {
    int   mode, active;
    char  text[NAME_LEN];
    float t, next;                      /* s into it; s to the next one */
    float x, y, height;                 /* SLIDE */
    int   col, row0;                    /* FALL: its column, its first row; DECODE: its row, first column */
    float head, speed;                  /* FALL */
    float lock[NAME_LEN];               /* DECODE: when each letter locks */
} N;

static struct {
    vp_music_feed m;
    int   have;
    float time, loud;
    uint32_t seed, rng;
} G;

static float rnd(void)
{
    G.rng ^= G.rng << 13; G.rng ^= G.rng >> 17; G.rng ^= G.rng << 5;
    return (float)(G.rng & 0xffffff) / 16777216.0f;
}

static float smooth01(float t)
{
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    return t * t * (3 - 2 * t);
}

static void color_k(vp_dvg *d, float r, float g, float b, float k)
{
    k = k < 0 ? 0 : k > 1 ? 1 : k;
    vp_color(d, (uint8_t)(r * k), (uint8_t)(g * k), (uint8_t)(b * k));
}

/* ---- reading the game list ----------------------------------------------- */

/* A name as shown: capitals, without a trailing "(...)", trimmed. */
static void add_name(const char *s)
{
    char buf[NAME_LEN];
    int n = 0;
    while (*s == ' ' || *s == '\t') s++;
    for (; *s && *s != '(' && *s != '\r' && *s != '\n' && n < NAME_LEN - 1; s++)
        buf[n++] = (char)toupper((unsigned char)*s);
    while (n > 0 && buf[n - 1] == ' ') n--;
    buf[n] = 0;
    if (n < 2 || nnames >= NAMES) return;
    for (int i = 0; i < nnames; i++) if (!strcmp(names[i], buf)) return;
    memcpy(names[nnames++], buf, (size_t)n + 1);
}

/* advance's gamelist.ini: maker|name|..., '#' lines left out */
static void read_gamelist(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#') continue;
        char *a = strchr(line, '|');
        if (!a) continue;
        char *b = strchr(a + 1, '|');
        if (b) *b = 0;
        add_name(a + 1);
    }
    fclose(f);
}

/* the cabinet's own games: <dir>/<game>/game.ini, name = ... */
static void read_games(const char *dir)
{
    DIR *dd = opendir(dir);
    if (!dd) return;
    struct dirent *e;
    while ((e = readdir(dd))) {
        if (e->d_name[0] == '.') continue;
        char path[512], line[256];
        snprintf(path, sizeof path, "%s/%s/game.ini", dir, e->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        while (fgets(line, sizeof line, f)) {
            if (strncmp(line, "name", 4)) continue;
            char *eq = strchr(line, '=');
            if (!eq) continue;
            char *semi = strchr(eq, ';');
            if (semi) *semi = 0;
            add_name(eq + 1);
            break;
        }
        fclose(f);
    }
    closedir(dd);
}

static const char *pick_name(void)
{
    if (nnames == 0 || rnd() < 0.3f) return "VECTORPIE";   /* now and then, ourselves */
    return names[(int)(rnd() * nnames) % nnames];
}

/* ---- the rain, moving ------------------------------------------------------ */

static void new_drop(int c)
{
    for (int i = 0; i < DROPS; i++) {
        drop *p = &C[c].drops[i];
        if (p->active) continue;
        p->active = 1;
        p->head = -1;
        p->speed = 5 + rnd() * 8;
        p->len = 6 + (int)(rnd() * 12);
        return;
    }
}

static int fall_column_busy(int c) { return N.active && N.mode == FALL && N.col == c; }

static void rain(float dt)
{
    float pace = 1 + 0.6f * G.loud;                    /* louder: faster, smoothly */
    for (int c = 0; c < COLS; c++) {
        for (int i = 0; i < DROPS; i++) {
            drop *p = &C[c].drops[i];
            if (!p->active) continue;
            p->head += p->speed * pace * dt;
            if (p->head - p->len > ROWS) p->active = 0;
        }
        if ((C[c].next -= dt * pace) <= 0) {
            C[c].next = 0.8f + rnd() * 3.5f;
            if (!fall_column_busy(c)) new_drop(c);
        }
    }
    /* now and then a glyph changes, quietly */
    int changes = (int)(dt * 25 + rnd());
    for (int k = 0; k < changes; k++) {
        int c = (int)(rnd() * COLS) % COLS, r = (int)(rnd() * ROWS) % ROWS;
        C[c].glyph[r] = (uint8_t)((int)(rnd() * NGLYPHS) % NGLYPHS | (rnd() < 0.3f ? 0x80 : 0));
    }
}

/* ---- the names, moving ------------------------------------------------------ */

static void start_name(void)
{
    const char *s = pick_name();
    memset(&N, 0, sizeof N);
    snprintf(N.text, sizeof N.text, "%s", s);
    int len = (int)strlen(N.text);
    N.active = 1;
    N.mode = (int)(rnd() * MODES) % MODES;
    if (len > ROWS - 2 && N.mode == FALL) N.mode = SLIDE;     /* too long to fall, to lock in a row */
    if (len > COLS - 2 && N.mode == DECODE) N.mode = SLIDE;
    switch (N.mode) {
    case SLIDE:
        vp_text_font(VP_FONT_ASTEROIDS);
        N.height = CELL_H * 1.5f;
        N.x = W + 50;
        N.y = W * (0.3f + rnd() * 0.4f);
        break;
    case FALL:
        N.col = (int)(rnd() * COLS) % COLS;
        N.row0 = (int)(rnd() * (ROWS - len));
        N.head = -1;
        N.speed = 4.5f;
        break;
    default:                                            /* DECODE */
        N.row0 = 3 + (int)(rnd() * (ROWS - 6));
        N.col = (COLS - len) / 2;
        for (int i = 0; i < len; i++) N.lock[i] = 0.2f + rnd() * 1.6f;
        break;
    }
}

static void names_update(float dt)
{
    if (!N.active) {
        if ((N.next -= dt) <= 0) start_name();
        return;
    }
    N.t += dt;
    int len = (int)strlen(N.text), done = 0;
    switch (N.mode) {
    case SLIDE:
        vp_text_font(VP_FONT_ASTEROIDS);
        N.x -= 700 * dt;
        done = N.x + vp_text_width(N.text, N.height) < -50;
        break;
    case FALL:
        N.head += N.speed * dt;
        done = N.head - (N.row0 + len) > 8;             /* well past its last letter, then gone */
        break;
    default:
        done = N.t > 8.5f;                              /* lock in, hold, come apart */
        break;
    }
    if (done) {
        N.active = 0;
        N.next = 20 + rnd() * 20;
    }
}

/* ---- drawing ----------------------------------------------------------------- */

static float cell_x(int c) { return MARGIN + (c + 0.5f) * CELL_W; }
static float cell_y(int r) { return W - MARGIN - (r + 0.5f) * CELL_H; }   /* the middle of a cell */

static void draw_glyph(vp_dvg *d, uint8_t g, float cx, float cy)
{
    const glyph *gl = &GLYPHS[g & 0x7f];
    float m = (g & 0x80) ? -1 : 1, x0 = cx - GLYPH_W / 2 * m, y0 = cy - GLYPH_H / 2;
    for (int i = 0; i < gl->n; i++) {
        const float *s = gl->s[i];
        vp_moveto(d, (int)(x0 + s[0] * GLYPH_W * m), (int)(y0 + s[1] * GLYPH_H));
        vp_lineto(d, (int)(x0 + s[2] * GLYPH_W * m), (int)(y0 + s[3] * GLYPH_H));
    }
}

static void draw_letter(vp_dvg *d, char ch, float cx, float cy)
{
    char s[2] = { ch, 0 };
    vp_text_font(VP_FONT_ASTEROIDS);
    vp_text(d, s, cx, cy - GLYPH_H / 2, GLYPH_H, VP_TEXT_CENTER);
}

/* Is this cell under the gliding name (the rain makes way for it)? */
static int under_slide(float x, float y)
{
    if (!N.active || N.mode != SLIDE) return 0;
    vp_text_font(VP_FONT_ASTEROIDS);
    float w = vp_text_width(N.text, N.height);
    return x > N.x - CELL_W && x < N.x + w + CELL_W && y > N.y - CELL_H && y < N.y + N.height + CELL_H;
}

/* DECODE: how locked a letter is now, 0 .. 1 */
static float locked(int i)
{
    float in = smooth01((N.t - N.lock[i]) / 0.25f);
    float out = 1 - smooth01((N.t - 6.0f - N.lock[i] * 0.6f) / 0.25f);
    return in * out;
}

static void draw(vp_game *g, vp_dvg *d)
{
    (void)g;
    int len = (int)strlen(N.text);
    for (int c = 0; c < COLS; c++) {
        float x = cell_x(c);
        for (int i = 0; i < DROPS; i++) {
            const drop *p = &C[c].drops[i];
            if (!p->active) continue;
            int head = (int)floorf(p->head);
            for (int k = 0; k < p->len; k++) {
                int r = head - k;
                if (r < 0 || r >= ROWS) continue;
                float y = cell_y(r);
                if (under_slide(x, y)) continue;
                if (N.active && N.mode == DECODE && r == N.row0 && c >= N.col && c < N.col + len && locked(c - N.col) > 0.5f)
                    continue;                           /* a letter is there now */
                float f = 1 - (float)k / p->len;
                if (k == 0) color_k(d, 200, 255, 210, 1);
                else color_k(d, 0, 255, 80, 0.15f + 0.75f * f * f);
                draw_glyph(d, C[c].glyph[r], x, y);
            }
        }
    }
    if (!N.active) return;
    switch (N.mode) {
    case SLIDE:
        vp_text_font(VP_FONT_ASTEROIDS);
        color_k(d, 0, 140, 50, 1);                      /* a faint echo behind it */
        vp_text(d, N.text, N.x + N.height * 0.6f, N.y, N.height, VP_TEXT_LEFT);
        color_k(d, 180, 255, 190, 1);
        vp_text(d, N.text, N.x, N.y, N.height, VP_TEXT_LEFT);
        break;
    case FALL: {
        int head = (int)floorf(N.head);
        for (int i = 0; i < len; i++) {
            int r = N.row0 + i;
            if (r > head) break;                        /* not reached yet */
            float age = N.head - r;                     /* rows since the head passed */
            if (N.text[i] == ' ') continue;
            if (r == head) color_k(d, 220, 255, 230, 1);
            else color_k(d, 60, 255, 120, 1 - smooth01((age - len - 2) / 6));
            draw_letter(d, N.text[i], cell_x(N.col), cell_y(r));
        }
        break;
    }
    default:
        for (int i = 0; i < len; i++) {
            float k = locked(i);
            if (k <= 0.01f || N.text[i] == ' ') continue;
            color_k(d, 160, 255, 180, k);
            draw_letter(d, N.text[i], cell_x(N.col + i), cell_y(N.row0));
        }
        break;
    }
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

static void draw_all(vp_game *g, vp_dvg *d)
{
    draw(g, d);
    shots(d);
}

static void update(vp_game *g, float dt)
{
    (void)g;
    G.time += dt;
    G.have = vp_saver_music(&G.m);
    G.loud += ((G.have ? G.m.level : 0) - G.loud) * fminf(1, dt * 0.3f);
    rain(dt);
    names_update(dt);
}

static int init(vp_game *g)
{
    (void)g;
    G.seed = (uint32_t)vp_ticks_ms() * 2654435761u;
    G.rng = G.seed | 1;
    const char *list = getenv("SAVER_GAMELIST");
    read_gamelist(list ? list : GAMELIST);
    if (!list) {
        read_gamelist("/usr/local/share/advance/gamelist.additions");
        read_games(GAMES_DIR);
    }
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r < ROWS; r++) C[c].glyph[r] = (uint8_t)((int)(rnd() * NGLYPHS) % NGLYPHS | (rnd() < 0.3f ? 0x80 : 0));
        C[c].next = rnd() * 3;
    }
    N.next = 8;                                         /* the first name soon */
    fprintf(stderr, "Digital Rain: %d names\n", nnames);
    return VP_OK;
}

int main(int argc, char **argv)
{
    vp_game_desc desc = {0};
    desc.title = "Digital Rain";
    desc.init = init;
    desc.update = update;
    desc.draw = draw_all;
    return vp_run(&desc, argc, argv);
}
