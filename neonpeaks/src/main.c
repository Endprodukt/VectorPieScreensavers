/* Neon Peaks — a VectorPie screensaver: a flight down a neon valley towards
 * a striped sun, between mountains the music raises.
 *
 * The valley floor is a glowing grid; on both sides wireframe mountains,
 * solid (they hide what is behind them). Far away a range of peaks stands
 * before the sun, never reached.
 *
 * The music builds the land: each new row of ground, far ahead at the
 * horizon, takes its shape from the spectrum heard as it is made (the bass
 * the mountains by the valley, the treble the far ones), then comes towards
 * the camera. So the mountains are the music, a little while ago. Also,
 * smoothly: the flight is faster when it is louder. The neon colors fade
 * slowly into one another, a new track starting the next fade. Nothing flashes or
 * blinks; new rows rise out of the flat at the horizon, never pop.
 *
 * On the cabinet: installed in SETTINGS -> SCREENSAVERS (tools/deploy.sh or
 * the .vps). Any control wakes the cabinet, START 2 skips to the next track.
 * On a PC: saver_neonpeaks <a .ogg/.wav or a folder of them>. Esc quits.
 * SAVER_SHOTS=<folder> saves a picture every 2 s (for development).
 */
#include <vectorpie/vp.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define W        VP_DVG_COORD_MAX
#define CX       (W / 2.0f)
#define PI       3.14159265f
#define ROWS     24                     /* rows of ground ahead */
#define RING     32                     /* rows kept (ROWS ahead, some behind) */
#define COLS     21                     /* points across */
#define DZ       10.0f                  /* m between rows */
#define VALLEY   16.0f                  /* m: the flat floor's half width */
#define PEAK     55.0f                  /* m: the highest mountains */
#define EYE_Y    5.0f
#define SUN_DIST 2600.0f
#define SUN_R    640.0f
#define SUN_Y    430.0f
#define RANGE_N  28                     /* points of the far range */

/* the columns' x: close together in the valley, wider out on the slopes */
static float XS[COLS];

static struct {
    float    h[RING][COLS];             /* the ground's height, per row (by its number % RING) */
    long     made;                      /* rows made: row numbers 0 .. made-1 */
    float    z;                         /* the camera, m along the valley */
    float    x;                         /* its drift from the middle */
    float    speed, time, loud;
    float    bands[VP_MUSIC_BANDS];     /* the spectrum, smoothed */
    vp_music_feed m;
    int      have;
    unsigned last_track;
    float    title_time;
    char     title[VP_MUSIC_TEXT];
    int      pal_from, pal_to;
    float    pal_t;                     /* 0..1 of the blend */
    float    pal_hold;                  /* s the current one has been held */
    float    range[RANGE_N];            /* the far range's peaks, m */
    uint32_t seed;
} G;

static vp3d_scene *scene;
static vp3d_view view;

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
static float smooth01(float t) { t = clampf(t, 0, 1); return t * t * (3 - 2 * t); }

static float hash01(uint32_t i)
{
    i ^= G.seed;
    i *= 2654435761u;
    i ^= i >> 15;
    i *= 2246822519u;
    i ^= i >> 13;
    return (float)(i & 0xffffff) / 16777215.0f;
}

/* value noise, 0..1, smooth, in two dimensions */
static float noise2(float x, float y)
{
    int ix = (int)floorf(x), iy = (int)floorf(y);
    float fx = smooth01(x - ix), fy = smooth01(y - iy);
#define HN(a, b) hash01((uint32_t)(a) * 73856093u ^ (uint32_t)(b) * 19349663u)
    float a = HN(ix, iy), b = HN(ix + 1, iy), c = HN(ix, iy + 1), d = HN(ix + 1, iy + 1);
#undef HN
    return (a + (b - a) * fx) + ((c + (d - c) * fx) - (a + (b - a) * fx)) * fy;
}

/* ---- the colors: floor, low slopes, peaks, sun top, sun bottom, far range ---- */

/* All neon (2026-11-07: the user liked the first, magenta and cyan, best;
 * green and orange sets are out). They fade into one another all the time:
 * held a while, then blended slowly; a new track starts the next blend. */
typedef struct { float r, g, b; } rgb;
#define NPAL      5
#define PAL_HOLD  18.0f                 /* s each is held */
#define PAL_BLEND 8.0f                  /* s from one to the next */
static const rgb PAL[NPAL][6] = {
    { { 255, 0, 200 }, { 200, 0, 255 },  { 0, 230, 255 },  { 255, 230, 0 },  { 255, 0, 120 }, { 120, 0, 200 } },
    { { 0, 200, 255 }, { 60, 90, 255 },  { 255, 0, 200 },  { 255, 200, 0 },  { 255, 0, 150 }, { 0, 90, 200 } },
    { { 255, 40, 140 }, { 180, 0, 255 }, { 40, 140, 255 },  { 255, 210, 40 }, { 220, 0, 255 }, { 110, 0, 170 } },
    { { 150, 0, 255 }, { 255, 0, 160 },  { 0, 255, 240 },  { 255, 240, 60 }, { 255, 0, 180 }, { 90, 0, 160 } },
    { { 0, 120, 255 }, { 0, 230, 255 },  { 255, 60, 220 },  { 255, 220, 30 }, { 255, 40, 160 }, { 60, 0, 200 } },
};

static rgb pal(int i)
{
    float t = smooth01(G.pal_t);
    rgb a = PAL[G.pal_from][i], b = PAL[G.pal_to][i];
    rgb c = { a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t };
    return c;
}

static rgb mix(rgb a, rgb b, float t)
{
    rgb c = { a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t };
    return c;
}

static void color(vp_dvg *d, rgb c, float k)
{
    k = clampf(k, 0, 1);
    vp_color(d, (uint8_t)(c.r * k), (uint8_t)(c.g * k), (uint8_t)(c.b * k));
}

/* ---- the ground ----------------------------------------------------------- */

/* Row n, made now: its heights from the noise and the spectrum. */
static void make_row(long n)
{
    float *h = G.h[n % RING];
    float zz = n * DZ;
    for (int j = 0; j < COLS; j++) {
        float x = XS[j], ax = fabsf(x);
        float slope = smooth01((ax - VALLEY) / 45);               /* 0 in the valley, 1 well out */
        if (slope <= 0) { h[j] = 0; continue; }
        float n1 = noise2(x * 0.035f, zz * 0.022f), n2 = noise2(x * 0.09f + 40, zz * 0.06f);
        float shape = 0.25f + 0.75f * (n1 * 0.75f + n2 * 0.25f);
        /* the music: bass by the valley, treble far out */
        float music = 1;
        if (G.have) {
            float f = clampf((ax - VALLEY) / (XS[COLS - 1] - VALLEY), 0, 1) * (VP_MUSIC_BANDS - 1);
            int b = (int)f;
            float band = G.bands[b] + (G.bands[b + 1 < VP_MUSIC_BANDS ? b + 1 : b] - G.bands[b]) * (f - b);
            music = 0.35f + 1.3f * sqrtf(clampf(band, 0, 1));
        }
        h[j] = PEAK * slope * shape * music;
    }
}

/* how high a point of row n stands now: new rows rise out of the flat at the horizon */
static float height(long n, int j)
{
    float ahead = n * DZ - G.z;
    return G.h[n % RING][j] * smooth01((ROWS * DZ - ahead) / (4 * DZ));
}

/* the color of a piece of ground as high as h, k bright */
static void ground_color(vp_dvg *d, float h, float k)
{
    color(d, h < 0.5f ? pal(0) : mix(pal(1), pal(2), clampf(h / PEAK, 0, 1)), k);
}

static void ground_draw(vp_dvg *d)
{
    static float hh[ROWS + 1][COLS], zz[ROWS + 1], kk[ROWS + 1];
    long first = (long)floorf(G.z / DZ) - 1;
    if (first < 0) first = 0;
    for (int r = 0; r <= ROWS; r++) {
        long n = first + r;
        float ahead = n * DZ - G.z;
        zz[r] = n * DZ;
        /* dimmer with distance (our own fog: the view's would dim the sun too),
         * and fading in at the horizon */
        kk[r] = smooth01((ROWS * DZ - ahead) / (3 * DZ)) * (1 - 0.65f * clampf((ahead - 30) / (ROWS * DZ - 30), 0, 1));
        for (int j = 0; j < COLS; j++) hh[r][j] = height(n, j);
    }
    /* the ground as invisible screens: the mountains hide what is behind them */
    for (int r = 0; r < ROWS; r++)
        for (int j = 0; j + 1 < COLS; j++) {
            vp_vec3 p[4] = { vp_v3(XS[j], hh[r][j], zz[r]), vp_v3(XS[j + 1], hh[r][j + 1], zz[r]),
                             vp_v3(XS[j + 1], hh[r + 1][j + 1], zz[r + 1]), vp_v3(XS[j], hh[r + 1][j], zz[r + 1]) };
            vp_vec3 t[3] = { p[0], p[2], p[3] };        /* in two triangles: flat, so */
            vp3d_scene_polygon(scene, p, 3, 0);         /* that they never hide their own edges */
            vp3d_scene_polygon(scene, t, 3, 0);
        }
    /* its lines, in long strokes back and forth (each piece starting where
     * the last ended: few jumps of the beam). A row keeps its way, by its own
     * number: one drawn the other way round lies a little apart on the
     * monitor, and rows flipping as they come would twitch */
    for (int r = 0; r <= ROWS; r++)
        for (int i = 0; i + 1 < COLS; i++) {
            int odd = (int)((first + r) & 1);
            int j = odd ? COLS - 2 - i : i;
            int a = odd ? j + 1 : j, b = odd ? j : j + 1;
            ground_color(d, fmaxf(hh[r][a], hh[r][b]), kk[r]);
            vp3d_scene_line(scene, vp_v3(XS[a], hh[r][a], zz[r]), vp_v3(XS[b], hh[r][b], zz[r]));
        }
    for (int j = 0; j < COLS; j++)
        for (int i = 0; i < ROWS; i++) {
            int r = j & 1 ? ROWS - 1 - i : i;
            int a = j & 1 ? r + 1 : r, b = j & 1 ? r : r + 1;
            ground_color(d, fmaxf(hh[a][j], hh[b][j]), kk[r]);
            vp3d_scene_line(scene, vp_v3(XS[j], hh[a][j], zz[a]), vp_v3(XS[j], hh[b][j], zz[b]));
        }
}

/* ---- the sky: the sun, the far range, the horizon ---------------------------- */

static void sun_draw(vp_dvg *d)
{
    float cz = G.z + SUN_DIST, cx = G.x;
    /* its outline */
    color(d, pal(3), 1);
    enum { SEG = 40 };
    for (int i = 0; i < SEG; i++) {
        float a0 = 2 * PI * i / SEG, a1 = 2 * PI * (i + 1) / SEG;
        vp3d_scene_line(scene, vp_v3(cx + SUN_R * sinf(a0), SUN_Y + SUN_R * cosf(a0), cz),
                        vp_v3(cx + SUN_R * sinf(a1), SUN_Y + SUN_R * cosf(a1), cz));
    }
    /* filled with lines across, top to bottom, sliding slowly down; in the
     * lower half the gaps between them widen (the stripes) */
    const int LINES = 26;
    float drift = fmodf(G.time * 0.12f, 1);
    for (int i = 0; i < LINES; i++) {
        float t = (i + drift) / LINES;                     /* 0 top .. 1 bottom */
        if (t > 0.5f) {
            float s = (t - 0.5f) * 2;                      /* 0..1 down the lower half */
            float band = fmodf((t - 0.5f) * LINES / 2.5f, 1);
            if (band < 0.25f + 0.55f * s) continue;        /* the gaps */
        }
        float y = SUN_R * (1 - 2 * t);
        float w = sqrtf(fmaxf(0, SUN_R * SUN_R - y * y));
        color(d, mix(pal(3), pal(4), t), 1);
        vp3d_scene_line(scene, vp_v3(cx - w, SUN_Y + y, cz), vp_v3(cx + w, SUN_Y + y, cz));
    }
}

static void range_draw(vp_dvg *d)
{
    /* far peaks before the sun, moving with the camera: never reached */
    const float dist = 1700, span = 2600;
    float z = G.z + dist;
    color(d, pal(5), 1);
    for (int i = 0; i + 1 < RANGE_N; i++) {
        float x0 = G.x - span / 2 + span * i / (RANGE_N - 1), x1 = G.x - span / 2 + span * (i + 1) / (RANGE_N - 1);
        vp_vec3 p[4] = { vp_v3(x0, -20, z), vp_v3(x1, -20, z), vp_v3(x1, G.range[i + 1], z), vp_v3(x0, G.range[i], z) };
        vp3d_scene_polygon(scene, p, 4, 4u);               /* its ridge only */
    }
}

/* ---- the music ------------------------------------------------------------- */

static void music(float dt)
{
    G.have = vp_saver_music(&G.m);
    if (G.have && G.m.track != G.last_track) {     /* a new track: its title, new colors */
        G.last_track = G.m.track;
        snprintf(G.title, sizeof G.title, "%s", G.m.title);
        G.title_time = G.title[0] ? 7 : 0;
        if (G.time > 1 && G.pal_t >= 1) G.pal_hold = PAL_HOLD;   /* the next colors now */
    }
    G.title_time = fmaxf(0, G.title_time - dt);
    if (G.pal_t < 1) G.pal_t = fminf(1, G.pal_t + dt / PAL_BLEND);
    else if ((G.pal_hold += dt) >= PAL_HOLD) {
        G.pal_from = G.pal_to;
        G.pal_to = (G.pal_to + 1 + (int)(hash01((uint32_t)(G.time * 10) + 3u) * (NPAL - 1))) % NPAL;
        G.pal_t = 0;
        G.pal_hold = 0;
    }
    G.loud += ((G.have ? G.m.level : 0) - G.loud) * fminf(1, dt * 0.4f);
    for (int i = 0; i < VP_MUSIC_BANDS; i++) {     /* the spectrum, smoothed: no spikes in the land */
        float b = G.have ? G.m.bands[i] : 0;
        G.bands[i] += (b - G.bands[i]) * fminf(1, dt * 4);
    }
}

/* ---- the game --------------------------------------------------------------- */

static int init(vp_game *g)
{
    (void)g;
    G.seed = (uint32_t)vp_ticks_ms() * 2654435761u;     /* other land each run */
    for (int j = 0; j < COLS; j++) {                    /* x: -170 .. 170, close in the middle */
        float t = (float)j / (COLS - 1) * 2 - 1;
        XS[j] = 170 * t * (0.35f + 0.65f * t * t);
    }
    for (int i = 0; i < RANGE_N; i++)
        G.range[i] = 60 + 260 * noise2(i * 0.45f, 9.5f) * (0.5f + 0.5f * hash01((uint32_t)i + 777u));
    G.pal_from = G.pal_to = 0;                         /* the classic first */
    G.pal_t = 1;
    G.speed = 22;
    if (vp3d_scene_create(&scene) != VP_OK) return VP_ERR_NOMEM;
    vp3d_scene_set_contact(scene, 0.2f);
    vp3d_view_init(&view, 72);
    view.near_z = 0.5f;
    view.far_z = 5000;
    return VP_OK;
}

static void update(vp_game *g, float dt)
{
    (void)g;
    G.time += dt;
    music(dt);
    float target = 20 + 22 * G.loud;                    /* m/s */
    G.speed += (target - G.speed) * fminf(1, dt * 0.5f);
    G.z += G.speed * dt;
    G.x = 4 * sinf(G.time * 0.11f) + 2 * sinf(G.time * 0.047f + 1);   /* a slow drift across */
    long need = (long)floorf(G.z / DZ) + ROWS + 1;
    while (G.made < need) make_row(G.made++);
}

/* The track's title over the sky, then gone. */
static void title(vp_dvg *d)
{
    if (G.title_time <= 0) return;
    float k = smooth01(G.title_time / 1.5f) * smooth01((7 - G.title_time) / 0.5f);
    vp_text_font(VP_FONT_ASTEROIDS);
    color(d, pal(3), k);
    vp_text_fit(d, G.title, CX, W * 0.93f, 110, W * 0.84f, VP_TEXT_CENTER);
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
    vp_vec3 eye = vp_v3(G.x, EYE_Y, G.z);
    vp3d_look_at(&view, eye, vp_v3(G.x * 0.6f, EYE_Y + 1.2f, G.z + 60), vp_v3(0, 1, 0));
    vp3d_scene_begin(scene, d, &view);
    /* the ground beyond the land: invisible, hiding the sun's lower part */
    const float r = 6000;
    vp_vec3 ground[4] = { vp_v3(G.x - r, 0, G.z), vp_v3(G.x + r, 0, G.z),
                          vp_v3(G.x + r, 0, G.z + r), vp_v3(G.x - r, 0, G.z + r) };
    vp3d_scene_polygon(scene, ground, 4, 0);
    sun_draw(d);
    range_draw(d);
    ground_draw(d);
    vp3d_scene_end(scene);
    title(d);
    shots(d);
}

int main(int argc, char **argv)
{
    vp_game_desc desc = {0};
    desc.title = "Neon Peaks";
    desc.init = init;
    desc.update = update;
    desc.draw = draw;
    return vp_run(&desc, argc, argv);
}
