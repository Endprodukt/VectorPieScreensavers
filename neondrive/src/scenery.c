/* scenery.c — the far scenes on the horizon, and the sun.
 *
 * The horizon is a ring R_SCN metres round the camera, turned by the
 * road's heading: as the road bends, the scenes go by. On it stands one
 * scene at a time: a city, mountains, pyramids, monuments, mesas. A scene
 * rises from behind the horizon, grows slowly as if coming nearer, and
 * before it is ever reached sinks away again while the next one rises: the
 * drive never arrives. A new track moves on to the next one, in a while.
 *
 * The shapes are flat, solid polygons facing the camera (they hide the sun
 * behind them, and one another), their details lines just in front. The
 * sun stands further out, low over the road ahead, following its bends
 * slowly. Everything below the ground is hidden by the ground itself (an
 * invisible polygon, main.c): a sinking scene sinks behind the horizon.
 */
#include "saver.h"

#include <math.h>
#include <string.h>

#define R_SCN  3500.0f          /* the scenes' ring */
#define R_HOR  5000.0f          /* the horizon line */
#define R_SUN  6000.0f
#define PW     (2 * PI_F * R_SCN)   /* the ring, round once */
#define FADE   10.0f            /* s to rise, s to sink */
#define MAXSH  64

enum { SC_CITY, SC_MOUNTAINS, SC_PYRAMIDS, SC_MONUMENTS, SC_MESAS, SC_COUNT };
enum { SH_TOWER, SH_PEAK, SH_PYRAMID, SH_OBELISK, SH_ARCH, SH_DOME, SH_LATTICE, SH_MESA };

typedef struct { int type; float u, w, h; uint32_t id; } shape;
typedef struct {
    int   active, kind;
    float age, life;
    int   n;
    shape sh[MAXSH];
} scene;

static scene S[2];              /* S[0] the one now, S[1] the one sinking away */
static uint32_t made;
static float sun_dir;           /* where the sun is, all turns added up (like G.heading) */

static vp3d_scene *SC;
static vp_dvg *D;

/* ---- making a scene ---------------------------------------------------- */

static uint32_t rng;
static float rnd(void)
{
    rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
    return (float)(rng & 0xffffff) / 16777216.0f;
}

static void add(scene *sc, int type, float u, float w, float h)
{
    if (sc->n >= MAXSH) return;
    shape *s = &sc->sh[sc->n++];
    s->type = type; s->u = u; s->w = w; s->h = h;
    s->id = (uint32_t)(rnd() * 1e6f);
}

static void make(scene *sc, int kind, float age)
{
    memset(sc, 0, sizeof *sc);
    sc->active = 1;
    sc->kind = kind;
    sc->age = age;
    rng = (G.seed * 7919u + (++made) * 104729u) | 1;
    sc->life = 150 + rnd() * 60;                                /* minutes, not seconds */
    switch (kind) {
    case SC_CITY:                                               /* towers in clusters */
        for (float u = 0; u < PW - 500;) {
            float w = 220 + rnd() * 330;
            add(sc, SH_TOWER, u + w / 2, w, 380 + rnd() * 720);
            u += w + (rnd() < 0.25f ? 400 + rnd() * 1000 : 15 + rnd() * 50);
        }
        break;
    case SC_MOUNTAINS:
        for (int i = 0; i < 16; i++)
            add(sc, SH_PEAK, i * PW / 16 + rnd() * 500, 1300 + rnd() * 1500, 450 + rnd() * 650);
        break;
    case SC_PYRAMIDS:
        for (int i = 0; i < 9; i++) {
            float w = 700 + rnd() * 900;
            add(sc, SH_PYRAMID, i * PW / 9 + rnd() * 700, w, w * 0.5f);
        }
        break;
    case SC_MONUMENTS:
        for (int i = 0; i < 10; i++) {
            float u = i * PW / 10 + rnd() * 600;
            switch ((int)(rnd() * 4)) {
            case 0:  add(sc, SH_OBELISK, u, 110 + rnd() * 50, 800 + rnd() * 300); break;
            case 1:  add(sc, SH_ARCH, u, 600 + rnd() * 250, 520 + rnd() * 160); break;
            case 2:  add(sc, SH_DOME, u, 560 + rnd() * 250, 600 + rnd() * 200); break;
            default: add(sc, SH_LATTICE, u, 400 + rnd() * 150, 1000 + rnd() * 250); break;
            }
        }
        break;
    default:                                                    /* SC_MESAS */
        for (int i = 0; i < 10; i++)
            add(sc, SH_MESA, i * PW / 10 + rnd() * 600, 900 + rnd() * 1200, 220 + rnd() * 330);
        break;
    }
}

static void start_next(void)
{
    int kind = S[0].active ? S[0].kind : -1, next;
    rng = (G.seed + made * 31u + 5) | 1;                       /* (xorshift: never 0) */
    next = kind < 0 ? (int)(rnd() * SC_COUNT) : (kind + 1 + (int)(rnd() * (SC_COUNT - 1))) % SC_COUNT;
    S[1] = S[0];
    make(&S[0], next, 0);
}

void scenery_update(float dt)
{
    if (!S[0].active) {
        make(&S[0], (int)(hash01(G.seed) * SC_COUNT), FADE);
        sun_dir = G.heading;
    }
    for (int i = 0; i < 2; i++) S[i].age += dt;
    if (S[1].active && S[1].age >= S[1].life) S[1].active = 0;
    if (!S[1].active && S[0].age > S[0].life - FADE) start_next();
    sun_dir += (G.heading - sun_dir) * fminf(1, dt / 12);       /* the sun follows the road, slowly */
}

void scenery_next(void)
{
    scene *sc = &S[0];
    if (sc->age > 90 && sc->age < sc->life - FADE) sc->life = sc->age + FADE;
}

/* ---- where things are -------------------------------------------------- */

static float wrap(float a)
{
    return remainderf(a, 2 * PI_F);                            /* -pi .. pi, without a loop */
}

/* On a ring r round the camera, at azimuth a (this frame's world): x along
 * the ring (to the right), y up from the ground. */
static vp_vec3 ring(float a, float r, float x, float y)
{
    float s = sinf(a), c = cosf(a);
    return vp_v3(G.eye.x + s * r + c * x, y, G.eye.z + c * r - s * x);
}

/* can the camera see something at azimuth a, half as wide as `half` radians? */
static int seen(float a, float half)
{
    return fabsf(wrap(a - G.cam_yaw)) < G.view.fov_deg * PI_F / 360 + half + 0.1f;
}

typedef struct { float base, k; } placing;      /* the bottom (sunk), the size */

static placing place(const scene *sc)
{
    float rise = smooth01(sc->age / FADE) * (1 - smooth01((sc->age - (sc->life - FADE)) / FADE));
    placing p;
    p.k = 0.85f + 0.2f * clampf(sc->age / sc->life, 0, 1);     /* nearer, never there */
    p.base = -(1 - rise) * 1500 * p.k;
    return p;
}

/* ---- the shapes: a solid outline and lines on it ----------------------- */

static float A;                 /* the shape being drawn: its azimuth */

static vp_vec3 P(float x, float y) { return ring(A, R_SCN, x, y); }
static vp_vec3 Q(float x, float y) { return ring(A, R_SCN - 3, x, y); }   /* just in front */

static void outline(const float *xy, int n, unsigned edges)
{
    vp_vec3 pts[16];
    for (int i = 0; i < n && i < 16; i++) pts[i] = P(xy[2 * i], xy[2 * i + 1]);
    vp3d_scene_polygon(SC, pts, n, edges);
}

static void detail(float x0, float y0, float x1, float y1)
{
    vp3d_scene_line(SC, Q(x0, y0), Q(x1, y1));
}

#define NO_BOTTOM (VP3D_ALL_EDGES & ~1u)    /* edge 0 (the first two points) left out */

static void tower(const shape *s, float b, float k)
{
    float hw = s->w * k / 2, h = s->h * k;
    float set = hash01(s->id) < 0.4f ? 0.72f : 1.0f;            /* a setback near the top */
    color_f(D, 40, 200, 255, 1);
    float body[] = { -hw, b, hw, b, hw, b + h * set, -hw, b + h * set };
    outline(body, 4, NO_BOTTOM);
    if (set < 1) {
        float top[] = { -hw * 0.6f, b + h * set, hw * 0.6f, b + h * set, hw * 0.6f, b + h, -hw * 0.6f, b + h };
        outline(top, 4, NO_BOTTOM);
    }
    if (hash01(s->id + 1) > 0.7f) {                             /* an antenna, its light steady */
        detail(0, b + h, 0, b + h * 1.15f);
        color_f(D, 255, 40, 60, 1);
        detail(-12, b + h * 1.15f, 12, b + h * 1.15f);
    }
    for (int i = 0; i < 10; i++) {                              /* a few lit windows */
        uint32_t id = s->id * 13u + (uint32_t)i;
        if (hash01(id + 99) > 0.5f) continue;
        float wx = (hash01(id + 3) - 0.5f) * 2 * (hw - 40);
        float wy = b + 60 + hash01(id + 4) * (h * set - 120);
        if (wy < 0) continue;
        if (hash01(id) < 0.5f) color_f(D, 255, 220, 90, 1); else color_f(D, 255, 80, 200, 1);
        detail(wx - 20, wy, wx + 20, wy);
    }
}

static void peak(const shape *s, float b, float k)
{
    float hw = s->w * k / 2, h = s->h * k;
    color_f(D, 150, 70, 255, 1);
    float tri[] = { -hw, b, hw, b, 0, b + h };
    outline(tri, 3, VP3D_ALL_EDGES & ~1u);
    color_f(D, 110, 50, 220, 0.8f);
    float side = hash01(s->id) < 0.5f ? -1 : 1;
    detail(0, b + h, side * hw * 0.25f, b + h * 0.4f);         /* a spur */
    detail(-hw * 0.2f, b + h * 0.8f, hw * 0.12f, b + h * 0.74f);   /* the snow line */
}

static void pyramid(const shape *s, float b, float k)
{
    float hw = s->w * k / 2, h = s->h * k, ex = hw * 0.3f;      /* the edge between its faces */
    color_f(D, 255, 150, 50, 1);
    float tri[] = { -hw, b, hw, b, 0, b + h };
    outline(tri, 3, NO_BOTTOM);
    detail(0, b + h, ex, b);
    color_f(D, 255, 110, 40, 0.7f);
    for (int i = 1; i <= 3; i++) {                              /* its courses, on the lit face */
        float t = i / 4.0f;
        detail(ex * (1 - t), b + h * t, hw - hw * t, b + h * t);
    }
}

static void obelisk(const shape *s, float b, float k)
{
    float hw = s->w * k / 2, h = s->h * k;
    color_f(D, 255, 170, 230, 1);
    float o[] = { -hw, b, hw, b, hw * 0.7f, b + h * 0.88f, 0, b + h, -hw * 0.7f, b + h * 0.88f };
    outline(o, 5, NO_BOTTOM);
    detail(-hw * 0.7f, b + h * 0.88f, hw * 0.7f, b + h * 0.88f);
}

static void arch(const shape *s, float b, float k)
{
    float hw = s->w * k / 2, h = s->h * k, ow = hw * 0.32f, oh = h * 0.42f;
    color_f(D, 255, 190, 110, 1);
    float o[] = { -hw, b, hw, b, hw, b + h, -hw, b + h };
    outline(o, 4, NO_BOTTOM);
    detail(-hw, b + h * 0.8f, hw, b + h * 0.8f);
    detail(-ow, b, -ow, b + oh);                                /* the opening, round at the top */
    detail(ow, b, ow, b + oh);
    for (int i = 0; i < 8; i++) {
        float a0 = PI_F * i / 8, a1 = PI_F * (i + 1) / 8;
        detail(-cosf(a0) * ow, b + oh + sinf(a0) * ow, -cosf(a1) * ow, b + oh + sinf(a1) * ow);
    }
}

static void dome(const shape *s, float b, float k)
{
    float hw = s->w * k / 2, h = s->h * k, dw = hw * 0.8f, drum = h * 0.5f;
    color_f(D, 120, 220, 255, 1);
    float o[2 * 13];
    int n = 0;
    o[n++] = -dw; o[n++] = b;
    o[n++] = dw;  o[n++] = b;
    for (int i = 0; i <= 10; i++) {                             /* the drum's right side, the dome */
        float a = PI_F * i / 10;
        o[n++] = cosf(a) * dw;
        o[n++] = b + drum + sinf(a) * h * 0.4f;
    }
    outline(o, n / 2, NO_BOTTOM);
    detail(0, b + h * 0.9f, 0, b + h);                          /* the spire */
    detail(-dw, b + drum, dw, b + drum);
    for (int i = 1; i <= 3; i++) {                              /* columns round the drum */
        float x = dw * (i / 2.0f - 1);
        detail(x, b, x, b + drum);
    }
}

static void lattice(const shape *s, float b, float k)
{
    float hw = s->w * k / 2, h = s->h * k;
    color_f(D, 255, 90, 200, 1);
    detail(-hw, b, -hw * 0.08f, b + h * 0.9f);
    detail(-hw * 0.08f, b + h * 0.9f, 0, b + h);
    detail(0, b + h, hw * 0.08f, b + h * 0.9f);
    detail(hw * 0.08f, b + h * 0.9f, hw, b);
    const float t[] = { 0.0f, 0.22f, 0.45f, 0.68f };
    for (int i = 0; i < 3; i++) {                               /* braced, narrowing */
        float w0 = hw * (1 - 0.92f * t[i] / 0.9f), w1 = hw * (1 - 0.92f * t[i + 1] / 0.9f);
        detail(-w0, b + h * t[i], w1, b + h * t[i + 1]);
        detail(w0, b + h * t[i], -w1, b + h * t[i + 1]);
        detail(-w1, b + h * t[i + 1], w1, b + h * t[i + 1]);
    }
}

static void mesa(const shape *s, float b, float k)
{
    float hw = s->w * k / 2, h = s->h * k, tw = s->w * k * 0.3f;
    color_f(D, 255, 110, 70, 1);
    float o[] = { -hw, b, hw, b, tw, b + h, -tw, b + h };
    outline(o, 4, NO_BOTTOM);
    color_f(D, 255, 80, 90, 0.6f);
    for (int i = 1; i <= 2; i++) {                              /* its layers of rock */
        float t = i / 3.0f, ww = hw + (tw - hw) * t;
        detail(-ww, b + h * t, ww, b + h * t);
    }
}

static void scene_draw(const scene *sc)
{
    placing p = place(sc);
    if (p.base < -1450 * p.k) return;                           /* all sunk */
    for (int j = 0; j < sc->n; j++) {
        const shape *s = &sc->sh[j];
        A = s->u / R_SCN - G.heading;
        if (!seen(A, s->w * p.k / 2 / R_SCN)) continue;
        switch (s->type) {
        case SH_TOWER:   tower(s, p.base, p.k); break;
        case SH_PEAK:    peak(s, p.base, p.k); break;
        case SH_PYRAMID: pyramid(s, p.base, p.k); break;
        case SH_OBELISK: obelisk(s, p.base, p.k); break;
        case SH_ARCH:    arch(s, p.base, p.k); break;
        case SH_DOME:    dome(s, p.base, p.k); break;
        case SH_LATTICE: lattice(s, p.base, p.k); break;
        default:         mesa(s, p.base, p.k); break;
        }
    }
}

/* ---- the sky ----------------------------------------------------------- */

/* The sun: low over the road, striped, yellow at the top to magenta at the
 * horizon, wider gaps lower down. It does not move with the music. */
static void sun(void)
{
    float a = sun_dir - G.heading, r = R_SUN * 0.16f, cy = R_SUN * 0.015f;
    if (!seen(a, 0.25f)) return;
    const float step = r * 0.055f;
    int band = 0, line = 0;
    for (float y = cy + r - step * 0.5f; y > 8; y -= step) {
        float p = (cy + r - y) / (cy + r);                     /* 0 the top, 1 the horizon */
        if (p > 0.45f) {
            int period = 4 - (int)((p - 0.45f) / 0.55f * 3);
            if (period < 2) period = 2;
            if (band++ % period == period - 1) continue;
        }
        float t = (y - cy) / r, half = r * sqrtf(fmaxf(0, 1 - t * t));
        if (p < 0.5f) color_f(D, 255, 230 - 240 * p, 60, 0.9f);
        else          color_f(D, 255, 110 - 160 * (p - 0.5f), 60 + 180 * (p - 0.5f), 0.9f);
        float side = line++ & 1 ? -1.0f : 1.0f;               /* back and forth: a short way between */
        vp3d_scene_line(SC, ring(a, R_SUN, -half * side, y), ring(a, R_SUN, half * side, y));
    }
    color_f(D, 255, 40, 180, 0.4f);                             /* a glow round it */
    for (int i = 0; i < 24; i++) {
        float a0 = PI_F * i / 24, a1 = PI_F * (i + 1) / 24;
        vp3d_scene_line(SC, ring(a, R_SUN, cosf(a0) * r * 1.12f, cy + sinf(a0) * r * 1.12f),
                            ring(a, R_SUN, cosf(a1) * r * 1.12f, cy + sinf(a1) * r * 1.12f));
    }
}

void sky_draw(vp3d_scene *sc, vp_dvg *d)
{
    SC = sc;
    D = d;
    sun();
    color_f(d, 255, 80, 220, 1);                                /* the horizon, glowing */
    float half = G.view.fov_deg * PI_F / 360 + 0.15f;
    for (int i = 0; i < 16; i++) {
        float a0 = G.cam_yaw - half + 2 * half * i / 16, a1 = G.cam_yaw - half + 2 * half * (i + 1) / 16;
        vp3d_scene_line(sc, ring(a0, R_HOR, 0, 0), ring(a1, R_HOR, 0, 0));
    }
    if (S[1].active) scene_draw(&S[1]);
    scene_draw(&S[0]);
}
