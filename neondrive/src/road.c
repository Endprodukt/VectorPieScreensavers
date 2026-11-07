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

/* The order of the lines matters as much as their number: the beam moves,
 * dark, from the end of one to the start of the next (the SDK draws in the
 * order asked), and a frame's whole way, lit and dark, is what makes the
 * board slow. So each line starts near where the last ended: the lines
 * along the road go back and forth (back: from far to near), the ones
 * across it too. */

/* A line along the road, x across (on the left, pushed out by the oncoming
 * carriageway when `shift`), faded with distance, through every `stride`th
 * of the distances; back: drawn from far to near. */
static void along(vp3d_scene *sc, vp_dvg *d, float x, int shift, int stride, float r, float g, float b, float k,
                  int back)
{
    vp_vec3 pts[64];
    float kk[64];
    int n = 0;
    for (int j = 0;; j += stride) {
        int i = j < nds - 1 ? j : nds - 1;              /* the last one always */
        float xx = shift ? x - ON_SHIFT * road_oncoming(ds[i]) : x;
        pts[n] = road_point(ds[i], xx, 0);
        kk[n++] = k * fade(ds[i]);
        if (i == nds - 1) break;
    }
    for (int m = 1; m < n; m++) {
        int a = back ? n - m : m - 1, e = back ? n - m - 1 : m;
        color_f(d, r, g, b, kk[a > e ? a : e]);         /* as faded as its far end */
        vp3d_scene_line(sc, pts[a], pts[e]);
    }
}

/* The oncoming carriageway: a line at `at` (-1 its left edge, 1 its right
 * edge), where there is one. */
static void oncoming_line(vp3d_scene *sc, vp_dvg *d, float at, float r, float g, float b, int back)
{
    vp_vec3 prev = vp_v3(0, 0, 0);
    float fprev = 0;
    for (int m = 0; m < nds; m++) {
        int i = back ? nds - 1 - m : m;
        float f = road_oncoming(ds[i]);
        float mid = -ROAD_HALF - (MEDIAN + ON_HALF) * f;
        vp_vec3 p = road_point(ds[i], mid + at * ON_HALF * f, 0);
        if (m > 0 && f > 0.001f && fprev > 0.001f) {
            color_f(d, r, g, b, fade(ds[back ? i + 1 : i]));
            vp3d_scene_line(sc, prev, p);
        }
        prev = p;
        fprev = f;
    }
}

void road_draw(vp3d_scene *sc, vp_dvg *d)
{
    /* the grid along the roads, fading outwards and with distance: out on the
     * right, back and forth, then in on the left (straight enough with half
     * the points) */
    int back = 0;
    for (int k = 0; k < GRID_N; k++, back ^= 1) {
        float x = ROAD_HALF + GRID_IN + k * GRID_X;
        along(sc, d, x, 0, 2, 220, 30, 190, 0.85f - k * 0.05f, back);
    }
    for (int k = GRID_N - 1; k >= 0; k--, back ^= 1) {
        float x = ROAD_HALF + GRID_IN + k * GRID_X;
        along(sc, d, -x, 1, 2, 220, 30, 190, 0.85f - k * 0.05f, back);
    }
    /* across, rushing in; every other one far away: on the left from far to
     * near, back and forth, then on the right from near to far */
    const float in = ROAD_HALF + GRID_IN, out = in + (GRID_N - 1) * GRID_X;
    long n0 = (long)ceilf((G.s - 10) / GRID_Z), n1 = (long)floorf((G.s + 300) / GRID_Z);
    for (int side = 0; side < 2; side++)
        for (long m = 0; m <= n1 - n0; m++) {
            long n = side == 0 ? n1 - m : n0 + m;
            float dz = n * GRID_Z - G.s;
            if (dz > 120 && (n & 1)) continue;
            float sh = side == 0 ? ON_SHIFT * road_oncoming(dz) : 0, sign = side == 0 ? -1.0f : 1.0f;
            vp_vec3 a = road_point(dz, sign * in - sh, 0), b = road_point(dz, sign * out - sh, 0);
            color_f(d, 255, 40, 210, fade(dz) * 0.85f);
            if (back) vp3d_scene_line(sc, b, a);
            else      vp3d_scene_line(sc, a, b);
            back ^= 1;
        }

    /* our road: its edges, doubled, and the dashes between its lanes, from
     * the right edge to the left one, near to far and back */
    along(sc, d, ROAD_HALF, 0, 1, 0, 240, 255, 1, 0);
    along(sc, d, ROAD_HALF - 0.3f, 0, 1, 0, 160, 255, 0.6f, 1);
    const float dash = 3, every = 12;
    long e0 = (long)ceilf((G.s - 10) / every), e1 = (long)floorf((G.s + 260) / every);
    for (int l = LANES - 1, up = 1; l >= 1; l--, up ^= 1) {
        float x = -ROAD_HALF + l * LANE_W;
        for (long m = 0; m <= e1 - e0; m++) {
            float a = (up ? e0 + m : e1 - m) * every - G.s;
            color_f(d, 255, 225, 70, fade(a));
            if (up) vp3d_scene_line(sc, road_point(a, x, 0), road_point(a + dash, x, 0));
            else    vp3d_scene_line(sc, road_point(a + dash, x, 0), road_point(a, x, 0));
        }
    }
    int up = (LANES - 1) & 1;                           /* where the dashes ended: far (1) or near */
    along(sc, d, -ROAD_HALF + 0.3f, 0, 1, 0, 160, 255, 0.6f, up);
    along(sc, d, -ROAD_HALF, 0, 1, 0, 240, 255, 1, !up);

    /* the oncoming carriageway: out and back */
    oncoming_line(sc, d, 1, 0, 240, 255, up);
    for (long m = 0; m <= e1 - e0; m++) {
        float a = (up ? e0 + m : e1 - m) * every - G.s;
        if (road_oncoming(a) < 0.9f) continue;
        color_f(d, 230, 230, 255, fade(a) * 0.8f);
        if (up) vp3d_scene_line(sc, road_point(a, ON_CENTER, 0), road_point(a + 3, ON_CENTER, 0));
        else    vp3d_scene_line(sc, road_point(a + 3, ON_CENTER, 0), road_point(a, ON_CENTER, 0));
    }
    oncoming_line(sc, d, -1, 0, 240, 255, up);
}
