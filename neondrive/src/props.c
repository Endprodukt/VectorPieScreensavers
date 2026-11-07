/* props.c — what stands by the road, stretch by stretch.
 *
 *   beach    palms on both sides, a lamp now and then, a VECTORPIE sign
 *   city     blocks of towers with lit windows, lamps, billboards
 *   desert   cacti, rocks, VECTORPIE signs
 *   power    pylons with their wires, guard rails
 *   tunnel   neon gates over the road, one after another
 *   highway  sign gantries over the road (every other one: VECTORPIE),
 *            lamps, guard rails
 *
 * Everything is placed by its distance along the road, from the stretch's
 * start, so it is always the same thing at the same place and comes in
 * from the far end. On the left, things stand beyond the oncoming
 * carriageway where there is one.
 */
#include "saver.h"

#include <math.h>

#define NEAR  -14.0f            /* behind the camera: gone */
#define FAR   280.0f

static vp3d_scene *S;
static vp_dvg *D;

static float left_x(float d, float off) { return -(ROAD_HALF + off) - ON_SHIFT * road_oncoming(d); }
static float right_x(float off) { return ROAD_HALF + off; }

static vp_xf placing(float d, float x, float turn)
{
    vp_vec3 at = road_point(d, x, 0);
    return vp_xf_mul2(vp_xf_translate(at.x, at.y, at.z), vp_xf_rot_y(road_heading(d) + turn));
}

/* A prop at d, x across; on the left turned round, to face the road. */
static void put(int kind, float d, float x, vp3d_mode mode)
{
    vp_xf p = placing(d, x, x < 0 ? PI_F : 0);
    vp3d_scene_mesh(S, model_prop(kind), &p, mode);
}

static void palm(float d, float x, uint32_t h)
{
    float k = fade(d);
    color_f(D, 255, 70, 190, k);
    put(P_PALM, d, x, VP3D_WIREFRAME);
    color_f(D, 0, 255, 170, k);
    vp_xf p = placing(d, x, (x < 0 ? PI_F : 0) + (hash01(h) - 0.5f) * 0.6f);
    vp3d_scene_mesh(S, model_prop(P_FRONDS), &p, VP3D_WIREFRAME);
}

/* a VECTORPIE sign by the road, on the right */
static void sign(float d)
{
    color_f(D, 255, 220, 80, fade(d));
    put(P_SIGN, d, right_x(2.5f), VP3D_SOLID);
}

static void lamp(float d, float x)
{
    color_f(D, 130, 150, 255, fade(d));
    put(P_LAMP, d, x, VP3D_SOLID);
}

/* a tower: a box `w` along the road, `deep` across, `h` high, its front
 * `off` from the road's edge; a few lit windows on its front */
static void tower(float d, int right, float off, float w, float deep, float h, uint32_t id)
{
    float face = right ? right_x(off) : left_x(d, off), mid = face + (right ? deep : -deep) / 2;
    vp_xf p = vp_xf_mul2(placing(d, mid, 0), vp_xf_scale(deep, h, w));
    color_f(D, 40, 200, 255, fade(d));
    vp3d_scene_mesh(S, model_prop(P_BOX), &p, VP3D_SOLID);
    if (d > 160) return;
    float fx = face + (right ? -0.05f : 0.05f);
    for (int i = 0; i < 8; i++) {
        uint32_t wid = id * 31u + (uint32_t)i;
        if (hash01(wid + 9) > 0.55f) continue;
        float wz = (hash01(wid + 1) - 0.5f) * (w - 2), wy = 2 + hash01(wid + 2) * (h - 4);
        if (hash01(wid) < 0.5f) color_f(D, 255, 220, 90, fade(d)); else color_f(D, 255, 80, 200, fade(d));
        vp3d_scene_line(S, road_point(d + wz - 0.7f, fx, wy), road_point(d + wz + 0.7f, fx, wy));
    }
}

/* a guard rail on posts, from a to b ahead (clipped to what is seen) */
static void rail(float a, float b, int right)
{
    /* its posts stand on the road, every 8 m from its start: the car goes
     * by them (they do not move with it) */
    const float every = 8;
    float first_d = fmaxf(a, -30.0f);                     /* (behind even the furthest camera) */
    first_d = a + ceilf((first_d - a) / every) * every;
    b = fminf(b, 160);
    vp_vec3 prev = vp_v3(0, 0, 0);
    int first = 1;
    for (float d = first_d; d <= b; d += every) {
        float x = right ? right_x(1.0f) : left_x(d, 1.0f);
        vp_vec3 p = road_point(d, x, 0.75f);
        color_f(D, 200, 200, 230, fade(d) * 0.8f);
        if (!first) vp3d_scene_line(S, prev, p);
        if (d < 70) vp3d_scene_line(S, road_point(d, x, 0), p);
        prev = p;
        first = 0;
    }
}

static void pylons(float start, float end)
{
    vp_vec3 prev[4];
    int have = 0;
    for (float at = start + 20; at < end; at += 70) {
        float d = at - G.s;
        if (d < NEAR - 70 || d > FAR) { have = 0; continue; }
        vp_xf p = placing(d, right_x(14), 0);
        vp_vec3 tips[4] = { vp_xf_point(&p, vp_v3(-7, 19, 0)), vp_xf_point(&p, vp_v3(7, 19, 0)),
                            vp_xf_point(&p, vp_v3(-5, 15, 0)), vp_xf_point(&p, vp_v3(5, 15, 0)) };
        color_f(D, 170, 120, 255, fade(d));
        if (d > NEAR) vp3d_scene_mesh(S, model_prop(P_PYLON), &p, VP3D_WIREFRAME);
        color_f(D, 150, 150, 200, fade(d) * 0.6f);
        if (have)                                               /* the wires, to the last one */
            for (int i = 0; i < 4; i++) vp3d_scene_line(S, prev[i], tips[i]);
        for (int i = 0; i < 4; i++) prev[i] = tips[i];
        have = 1;
    }
}

static void zone(long n)
{
    float start = n * ZONE_LEN, end = start + ZONE_LEN;
    int kind = zone_kind(n);
    uint32_t zs = G.seed * 101u + (uint32_t)n * 7919u;

    /* things every `every` metres from the start, d their distance ahead */
#define EACH(every) for (float at = start + (every) / 2, d = at - G.s; at < end; at += (every), d = at - G.s) \
                        if (d > NEAR && d < FAR)
    switch (kind) {
    case Z_BEACH: {
        EACH(16) {                                              /* sides by their place: they never swap */
            uint32_t h = zs + (uint32_t)(at * 3);
            float off = 3.5f + hash01(h) * 3;
            palm(d, ((long)((at - start) / 16) & 1) ? right_x(off) : left_x(d, off), h);
        }
        EACH(64) lamp(d, right_x(1.5f));
        EACH(240) sign(d + 20);
        break;
    }
    case Z_CITY: {
        EACH(30) {
            if (d > 220) break;                                 /* the far ones: too many lines */
            uint32_t h = zs + (uint32_t)(at * 5);
            for (int side = 0; side < 2; side++) {
                uint32_t id = h + (uint32_t)side * 77u;
                tower(d, side, 14 + hash01(id) * 10, 10 + hash01(id + 1) * 8, 10 + hash01(id + 2) * 6,
                      12 + hash01(id + 3) * 33, id);
            }
        }
        EACH(40) { lamp(d, right_x(1.2f)); lamp(d, left_x(d, 1.2f)); }
        EACH(130) {
            uint32_t h = zs + (uint32_t)at;
            int k = (int)((at - start) / 130) & 1;              /* by its place, like the palms */
            float x = k ? right_x(6) : left_x(d, 6);
            color_f(D, k ? 255 : 0, k ? 80 : 240, 220, fade(d));
            put(P_BILLBOARD1 + (int)(hash01(h) * 3) % 3, d, x, VP3D_SOLID);
        }
        break;
    }
    case Z_DESERT: {
        EACH(30) {
            uint32_t h = zs + (uint32_t)(at * 7);
            float off = 4 + hash01(h) * 10;
            color_f(D, 90, 255, 120, fade(d));
            put(P_CACTUS, d, hash01(h + 1) < 0.5f ? right_x(off) : left_x(d, off), VP3D_SOLID);
        }
        EACH(60) {
            uint32_t h = zs + (uint32_t)(at * 11);
            float off = 6 + hash01(h) * 14, sc = 0.7f + hash01(h + 2) * 0.9f;
            float x = hash01(h + 1) < 0.5f ? right_x(off) : left_x(d, off);
            vp_xf p = vp_xf_mul2(placing(d, x, hash01(h + 3) * 6), vp_xf_scale(sc, sc, sc));
            color_f(D, 255, 120, 70, fade(d));
            vp3d_scene_mesh(S, model_prop(P_ROCK), &p, VP3D_SOLID);
        }
        EACH(120) sign(d);
        break;
    }
    case Z_POWER:
        pylons(start, end);
        EACH(240) sign(d + 40);
        rail(start - G.s, end - G.s, 1);
        rail(start - G.s, end - G.s, 0);
        break;
    case Z_TUNNEL: {
        EACH(9) {
            if (d > 190) break;
            int hue = (int)((at - start) / 9) % 3;
            float k = fade(d);
            if (hue == 0) color_f(D, 255, 40, 200, k);
            else if (hue == 1) color_f(D, 0, 240, 255, k);
            else color_f(D, 160, 80, 255, k);
            put(P_ARCH, d, 0, VP3D_WIREFRAME);
        }
        break;
    }
    default:                                                    /* Z_HIGHWAY */
        EACH(160) {                                             /* every other one: VECTORPIE */
            color_f(D, 200, 220, 255, fade(d));
            put(((long)((at - start) / 160) & 1) ? P_GANTRY_VP : P_GANTRY, d, 0, VP3D_SOLID);
        }
        EACH(40) { lamp(d, right_x(1.2f)); lamp(d, left_x(d, 1.2f)); }
        EACH(260) sign(d + 70);
        rail(start - G.s, end - G.s, 1);
        rail(start - G.s, end - G.s, 0);
        break;
    }
#undef EACH
}

void props_draw(vp3d_scene *sc, vp_dvg *d)
{
    S = sc;
    D = d;
    long n0 = (long)floorf((G.s + NEAR) / ZONE_LEN), n1 = (long)floorf((G.s + FAR) / ZONE_LEN);
    for (long n = n0; n <= n1; n++) zone(n);
}
