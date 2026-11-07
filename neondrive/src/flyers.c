/* flyers.c — what flies: flocks of birds, and now and then a plane towing a
 * VECTORPIE banner.
 *
 * Both hang on the road: a flock or a plane is at a distance along it (and
 * across, and up), so the car comes nearer and passes under them.
 *   Birds fly in a V up the road, slower than the car, their wings beating
 *   slowly, now and then gliding.
 *   The plane crosses the road from left to right well ahead, the banner on
 *   a line behind it, its side to the road: the car drives under it.
 */
#include "saver.h"

#include <math.h>

#define FLOCKS     2
#define BIRDS      9
#define BIRD_SPAN  3.6f
#define PLANE_LEN  8.0f
#define BANNER_L   30.0f
#define BANNER_H   4.0f
#define TOW        10.0f

typedef struct { int active, n; float at, x, y, speed, phase; } flock;
typedef struct { int active; float at, x, y, speed; } plane;

static flock F[FLOCKS];
static plane PL;
static float next_flock = 4, next_plane = 12;
static vp3d_mesh *plane_mesh;
static float word_seg[40][4];
static int nword;
static uint32_t serial;

static vp3d_mesh *make_plane(void)
{
    /* along +x (its nose), wings across z: a small high-wing propeller plane */
    vp3d_mesh *m;
    vp3d_mesh_create(&m);
    const float bx[][6] = {
        { -4.0f, 3.2f, -0.6f, 0.7f, -0.6f, 0.6f },             /* the fuselage */
        { 3.2f, 4.0f, -0.4f, 0.5f, -0.4f, 0.4f },              /* the nose */
        { -0.6f, 1.2f, 0.7f, 0.85f, -5.5f, 5.5f },             /* the wing */
        { -4.0f, -3.0f, 0.7f, 2.2f, -0.08f, 0.08f },           /* the fin */
        { -4.0f, -3.1f, 0.0f, 0.12f, -1.8f, 1.8f },            /* the tailplane */
    };
    for (int i = 0; i < 5; i++) {
        float x0 = bx[i][0], x1 = bx[i][1], y0 = bx[i][2], y1 = bx[i][3], z0 = bx[i][4], z1 = bx[i][5];
        int b0 = vp3d_mesh_vertex(m, x0, y0, z0), b1 = vp3d_mesh_vertex(m, x1, y0, z0);
        int b2 = vp3d_mesh_vertex(m, x1, y0, z1), b3 = vp3d_mesh_vertex(m, x0, y0, z1);
        int t0 = vp3d_mesh_vertex(m, x0, y1, z0), t1 = vp3d_mesh_vertex(m, x1, y1, z0);
        int t2 = vp3d_mesh_vertex(m, x1, y1, z1), t3 = vp3d_mesh_vertex(m, x0, y1, z1);
        int f[6][4] = { { b0, b1, b2, b3 }, { t0, t3, t2, t1 }, { b3, b2, t2, t3 },
                        { b0, t0, t1, b1 }, { b1, t1, t2, b2 }, { b0, b3, t3, t0 } };
        for (int k = 0; k < 6; k++) vp3d_mesh_face(m, f[k], 4);
    }
    int p0 = vp3d_mesh_vertex(m, 4.05f, -1.0f, 0), p1 = vp3d_mesh_vertex(m, 4.05f, 1.1f, 0);   /* the propeller */
    vp3d_mesh_line(m, p0, p1);
    vp3d_mesh_finish(m);
    return m;
}

static float rnd(void) { return hash01(G.seed * 5u + (++serial) * 2971u); }

/* ---- birds ------------------------------------------------------------- */

static void birds_update(float dt)
{
    for (int i = 0; i < FLOCKS; i++) {
        flock *f = &F[i];
        if (!f->active) continue;
        f->at += f->speed * dt;
        if (f->at - G.s < -40) f->active = 0;
    }
    if ((next_flock -= dt) > 0) return;
    next_flock = 20 + rnd() * 30;
    for (int i = 0; i < FLOCKS; i++) {
        flock *f = &F[i];
        if (f->active) continue;
        f->active = 1;
        f->n = 5 + (int)(rnd() * (BIRDS - 4));
        f->at = G.s + 160 + rnd() * 60;
        f->x = (rnd() - 0.5f) * 60;
        f->y = 12 + rnd() * 12;
        f->speed = 10 + rnd() * 5;
        f->phase = rnd() * 10;
        return;
    }
}

static void birds_draw(vp3d_scene *sc, vp_dvg *d)
{
    for (int i = 0; i < FLOCKS; i++) {
        const flock *f = &F[i];
        if (!f->active) continue;
        float dz = f->at - G.s;
        color_f(d, 230, 230, 255, fade(dz) * 0.9f);
        for (int b = 0; b < f->n; b++) {
            int rank = (b + 1) / 2, side = (b & 1) ? 1 : -1;  /* a V: the leader, then pairs behind */
            float bd = dz - rank * 2.6f, bx = f->x + side * rank * 2.2f;
            float t = G.time * 1.6f + f->phase + b * 0.7f;
            float beat = fmodf(t, 6.0f) < 4.0f ? sinf(t * 3.1f) : 0.25f;   /* beating, then gliding */
            float tip = 0.15f + 0.45f * beat, half = BIRD_SPAN / 2;
            vp_vec3 l = road_point(bd - 0.3f, bx - half, f->y + tip);
            vp_vec3 m = road_point(bd, bx, f->y);
            vp_vec3 r = road_point(bd - 0.3f, bx + half, f->y + tip);
            vp3d_scene_line(sc, l, m);
            vp3d_scene_line(sc, m, r);
        }
    }
}

/* ---- the plane ----------------------------------------------------------- */

static void plane_update(float dt)
{
    if (PL.active) {
        PL.x += PL.speed * dt;
        if (PL.at - G.s < -60) PL.active = 0;
        return;
    }
    if ((next_plane -= dt) > 0) return;
    next_plane = 70 + rnd() * 60;
    PL.active = 1;
    PL.at = G.s + 480 + rnd() * 60;     /* the car gets there in about 15 s */
    PL.speed = 26 + rnd() * 6;
    PL.x = -PL.speed * 15 * (0.4f + rnd() * 0.2f) - 40;   /* over the road about then */
    PL.y = 28 + rnd() * 14;
}

static void plane_draw(vp3d_scene *sc, vp_dvg *d)
{
    if (!PL.active) return;
    float dz = PL.at - G.s, k = fade(dz * 0.6f);
    float h = road_heading(dz);
    vp_vec3 at = road_point(dz, PL.x, PL.y);
    /* the plane's own frame: +x across the road (where it flies), +z along */
    vp_xf place = vp_xf_mul2(vp_xf_translate(at.x, at.y, at.z), vp_xf_rot_y(h));
    color_f(d, 230, 230, 255, k);
    vp3d_scene_mesh(sc, plane_mesh, &place, VP3D_SOLID);

    /* the tow line, the banner, its word to the road */
    float bx0 = -4.0f - TOW, bx1 = bx0 - BANNER_L;
    color_f(d, 160, 160, 200, k * 0.7f);
    vp3d_scene_line(sc, vp_xf_point(&place, vp_v3(-4.0f, 0.2f, 0)), vp_xf_point(&place, vp_v3(bx0, 0, 0)));
    vp_vec3 cloth[4] = { vp_xf_point(&place, vp_v3(bx1, -BANNER_H, 0)), vp_xf_point(&place, vp_v3(bx0, -BANNER_H, 0)),
                         vp_xf_point(&place, vp_v3(bx0, 0, 0)), vp_xf_point(&place, vp_v3(bx1, 0, 0)) };
    color_f(d, 255, 80, 200, k);
    vp3d_scene_polygon(sc, cloth, 4, VP3D_ALL_EDGES);
    float wh = 2.6f, wx = bx1 + (BANNER_L - WORD_W * wh) / 2, wy = -BANNER_H + (BANNER_H - wh) / 2;
    color_f(d, 0, 240, 255, k);
    for (int i = 0; i < nword; i++) {
        /* on the side the road sees (-z), a little in front of the cloth */
        vp_vec3 a = vp_xf_point(&place, vp_v3(wx + word_seg[i][0] * wh, wy + word_seg[i][1] * wh, -0.1f));
        vp_vec3 b = vp_xf_point(&place, vp_v3(wx + word_seg[i][2] * wh, wy + word_seg[i][3] * wh, -0.1f));
        vp3d_scene_line(sc, a, b);
    }
}

/* ---- both ---------------------------------------------------------------- */

void flyers_update(float dt)
{
    if (!plane_mesh) {
        plane_mesh = make_plane();
        nword = vectorpie_strokes(word_seg, 40);
    }
    birds_update(dt);
    plane_update(dt);
}

void flyers_draw(vp3d_scene *sc, vp_dvg *d)
{
    birds_draw(sc, d);
    plane_draw(sc, d);
}
