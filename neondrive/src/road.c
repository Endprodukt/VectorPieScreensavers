/* road.c — the road: its bends, its stretches, the grid and the lines on it.
 *
 * The road is endless and made up as it goes. Its curvature at a distance s
 * is smooth noise, with straights where the noise is small. Each frame it
 * is laid out from the car (at the origin, heading +z): a table of points
 * every STEP metres, from ROAD_BACK behind to ROAD_AHEAD in front, by
 * adding the curvature up into a heading and the heading into a position.
 *
 * It comes in stretches of ZONE_LEN metres, each of a kind (beach, city,
 * desert, power lines, neon tunnel, highway: props.c dresses them), and in
 * some of them an oncoming carriageway of two lanes runs to the left, past
 * a median: it branches off at the start of the stretch and comes back at
 * its end. The grid on the ground lies beside the roads, never on them.
 */
#include "saver.h"

#include <math.h>

#define STEP      2.0f
#define NT        231                   /* (ROAD_BACK + ROAD_AHEAD) / STEP + 1 */
#define I0        20                    /* ROAD_BACK / STEP: the car's entry */
#define CURV_MAX  0.0045f               /* 1/m: a radius of 220 m at the sharpest */
#define CURV_LEN  650.0f                /* metres per noise step: a bend lasts a while */
#define TAPER     90.0f                 /* metres the oncoming carriageway takes to branch off */
#define GRID_X    5.0f                  /* the grid's lines along the road, apart */
#define GRID_N    9
#define GRID_Z    6.0f                  /* and across it */
#define GRID_IN   2.5f                  /* from the road's edge to the grid */

static vp_vec3 tpos[NT];
static float   thead[NT];

static float ds[64];                    /* distances the lines along the road go through */
static int   nds;

/* ---- the road's course ------------------------------------------------- */

static float noise(float x)
{
    int i = (int)floorf(x);
    float f = x - (float)i;
    float a = hash01(G.seed + (uint32_t)i) * 2 - 1, b = hash01(G.seed + (uint32_t)i + 1) * 2 - 1;
    return a + (b - a) * smooth01(f);
}

float road_curv(float s)
{
    float n = noise(s / CURV_LEN), a = fabsf(n);
    if (a < 0.3f) return 0;                     /* a straight */
    float c = (a - 0.3f) / 0.7f * CURV_MAX;
    return n < 0 ? -c : c;
}

static vp_vec3 dir(float h) { return vp_v3(sinf(h), 0, cosf(h)); }

void road_build(void)
{
    tpos[I0] = vp_v3(0, 0, 0);
    thead[I0] = 0;
    for (int i = I0; i < NT - 1; i++) {
        float d = (i - I0) * STEP;
        float h1 = thead[i] + road_curv(G.s + d + STEP / 2) * STEP;
        tpos[i + 1] = vp_v3_add(tpos[i], vp_v3_scale(dir((thead[i] + h1) / 2), STEP));
        thead[i + 1] = h1;
    }
    for (int i = I0; i > 0; i--) {
        float d = (i - I0) * STEP;
        float h0 = thead[i] - road_curv(G.s + d - STEP / 2) * STEP;
        tpos[i - 1] = vp_v3_sub(tpos[i], vp_v3_scale(dir((thead[i] + h0) / 2), STEP));
        thead[i - 1] = h0;
    }
    if (!nds) {                                 /* close together near, far apart far away */
        for (float d = -12; d <= ROAD_AHEAD - 20 && nds < 64;) {
            ds[nds++] = d;
            d += d < 30 ? 3 : d < 90 ? 6 : d < 200 ? 12 : 25;
        }
    }
}

float road_heading(float d)
{
    float f = (d + ROAD_BACK) / STEP;
    if (f <= 0) return thead[0];
    if (f >= NT - 1) return thead[NT - 1];
    int i = (int)f;
    return thead[i] + (thead[i + 1] - thead[i]) * (f - (float)i);
}

vp_vec3 road_point(float d, float x, float y)
{
    float f = (d + ROAD_BACK) / STEP, h;
    vp_vec3 p;
    if (f >= NT - 1) {                          /* beyond the table: on along its last heading */
        h = thead[NT - 1];
        p = vp_v3_add(tpos[NT - 1], vp_v3_scale(dir(h), (f - (NT - 1)) * STEP));
    } else {
        if (f < 0) f = 0;
        int i = (int)f;
        float t = f - (float)i;
        h = thead[i] + (thead[i + 1] - thead[i]) * t;
        p = vp_v3_add(tpos[i], vp_v3_scale(vp_v3_sub(tpos[i + 1], tpos[i]), t));
    }
    return vp_v3(p.x + cosf(h) * x, p.y + y, p.z - sinf(h) * x);
}

/* ---- its stretches ----------------------------------------------------- */

static int pick(long n)
{
    static const float often[Z_COUNT] = { 3, 3, 2, 2, 1, 2 };
    float total = 0;
    for (int k = 0; k < Z_COUNT; k++) total += often[k];
    float r = hash01(G.seed * 13u + (uint32_t)n * 2654435u) * total;
    int k = 0;
    while (k < Z_COUNT - 1 && r >= often[k]) r -= often[k++];
    return k;
}

int zone_kind(long n)
{
    int k = pick(n);
    return k == pick(n - 1) ? (k + 1) % Z_COUNT : k;    /* never the same twice running */
}

static int divided(long n)
{
    return zone_kind(n) != Z_TUNNEL && hash01(G.seed * 7u + (uint32_t)n * 977u + 3) < 0.45f;
}

float road_oncoming(float d)
{
    float s = G.s + d;
    long n = (long)floorf(s / ZONE_LEN);
    if (!divided(n)) return 0;
    float start = n * ZONE_LEN, end = start + ZONE_LEN;
    float in = divided(n - 1) ? 1 : smooth01((s - start) / TAPER);
    float out = divided(n + 1) ? 1 : smooth01((end - s) / TAPER);
    return in * out;
}

/* ---- the lines --------------------------------------------------------- */

/* A line along the road, x across (on the left, pushed out by the oncoming
 * carriageway when `shift`), faded with distance, through every `stride`th
 * of the distances. */
static void along(vp3d_scene *sc, vp_dvg *d, float x, int shift, int stride, float r, float g, float b, float k)
{
    vp_vec3 prev = vp_v3(0, 0, 0);
    for (int j = 0;; j += stride) {
        int i = j < nds - 1 ? j : nds - 1;              /* the last one always */
        float xx = shift ? x - ON_SHIFT * road_oncoming(ds[i]) : x;
        vp_vec3 p = road_point(ds[i], xx, 0);
        if (i > 0) {
            color_f(d, r, g, b, k * fade(ds[i]));
            vp3d_scene_line(sc, prev, p);
        }
        prev = p;
        if (i == nds - 1) break;
    }
}

/* The oncoming carriageway: a line at `at` (-1 its left edge, 1 its right
 * edge), where there is one. */
static void oncoming_line(vp3d_scene *sc, vp_dvg *d, float at, float r, float g, float b)
{
    vp_vec3 prev = vp_v3(0, 0, 0);
    float fprev = 0;
    for (int i = 0; i < nds; i++) {
        float f = road_oncoming(ds[i]);
        float mid = -ROAD_HALF - (MEDIAN + ON_HALF) * f;
        vp_vec3 p = road_point(ds[i], mid + at * ON_HALF * f, 0);
        if (i > 0 && f > 0.001f && fprev > 0.001f) {
            color_f(d, r, g, b, fade(ds[i]));
            vp3d_scene_line(sc, prev, p);
        }
        prev = p;
        fprev = f;
    }
}

void road_draw(vp3d_scene *sc, vp_dvg *d)
{
    /* the grid along the roads, fading outwards and with distance */
    for (int k = 0; k < GRID_N; k++) {
        float x = ROAD_HALF + GRID_IN + k * GRID_X, f = 0.85f - k * 0.05f;
        along(sc, d, x, 0, 2, 220, 30, 190, f);           /* straight enough with half the points */
        along(sc, d, -x, 1, 2, 220, 30, 190, f);
    }
    /* across, rushing in; every other one far away */
    long n0 = (long)ceilf((G.s - 10) / GRID_Z);
    for (long n = n0;; n++) {
        float dz = n * GRID_Z - G.s;
        if (dz > 300) break;
        if (dz > 120 && (n & 1)) continue;
        float in = ROAD_HALF + GRID_IN, out = in + (GRID_N - 1) * GRID_X, sh = ON_SHIFT * road_oncoming(dz);
        color_f(d, 255, 40, 210, fade(dz) * 0.85f);
        vp3d_scene_line(sc, road_point(dz, in, 0), road_point(dz, out, 0));
        vp3d_scene_line(sc, road_point(dz, -in - sh, 0), road_point(dz, -out - sh, 0));
    }

    /* our road: its edges, doubled, and the dashes between its lanes */
    along(sc, d, ROAD_HALF, 0, 1, 0, 240, 255, 1);
    along(sc, d, -ROAD_HALF, 0, 1, 0, 240, 255, 1);
    along(sc, d, ROAD_HALF - 0.3f, 0, 1, 0, 160, 255, 0.6f);
    along(sc, d, -ROAD_HALF + 0.3f, 0, 1, 0, 160, 255, 0.6f);
    const float dash = 3, every = 12;
    for (int l = 1; l < LANES; l++) {
        float x = -ROAD_HALF + l * LANE_W;
        for (long n = (long)ceilf((G.s - 10) / every);; n++) {
            float a = n * every - G.s;
            if (a > 260) break;
            color_f(d, 255, 225, 70, fade(a));
            vp3d_scene_line(sc, road_point(a, x, 0), road_point(a + dash, x, 0));
        }
    }

    /* the oncoming carriageway */
    oncoming_line(sc, d, -1, 0, 240, 255);
    oncoming_line(sc, d, 1, 0, 240, 255);
    for (long n = (long)ceilf((G.s - 10) / 12);; n++) {
        float a = n * 12 - G.s;
        if (a > 260) break;
        if (road_oncoming(a) < 0.9f) continue;
        color_f(d, 230, 230, 255, fade(a) * 0.8f);
        vp3d_scene_line(sc, road_point(a, ON_CENTER, 0), road_point(a + 3, ON_CENTER, 0));
    }
}
