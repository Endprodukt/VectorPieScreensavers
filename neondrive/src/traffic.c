/* traffic.c — the others on the road, both ways, and how the car gets by.
 *
 * Our road has three lanes, all going our way. Cars, sports cars, trucks,
 * motorbikes, vans, pickups and buses drive on them, all slower than the
 * car, so it keeps catching up with them: each comes in from the far end,
 * grows, and is passed. The louder the music, the more of them. They keep
 * a lane each and do not run into one another (one catching up with a
 * slower one in its lane falls in behind).
 *
 * Where the oncoming carriageway runs beside ours, traffic comes the other
 * way on its two lanes: only where that carriageway reaches all the way to
 * the car, so that every one of them goes by.
 *
 * The car looks ahead in its lane: someone near, it moves to the lane
 * beside with the most room (if there is room to get in), and while no
 * lane is free it slows down behind. Now and then it changes lanes just so.
 *
 * Nothing ever drives through anything: the car's front never passes the
 * back of one in its way (traffic_limit), it does not move across into
 * one beside it, and one coming up behind it stays BEHIND metres back (so
 * the camera, behind the car, is never inside it).
 *
 * The camera can ask for a lane (traffic_hold_lane): the car moves there
 * as soon as there is room, and stays, slowing behind anyone in it,
 * until the camera lets it go.
 *
 * Positions: `at` is a vehicle's back, in metres along the road (like G.s).
 * The car's back is at G.s, its front CAR_LEN ahead.
 */
#include "saver.h"

#include <math.h>

#define NV        12
#define CAR_LEN   4.5f
#define CAR_W     1.96f
#define SPAWN_AT  260.0f        /* new ones come in this far ahead, and up to 60 m more */
#define SPACING   35.0f         /* at least this far apart along the road (our way) */
#define LOOK      55.0f         /* how far ahead the car looks in its lane */
#define ROOM      30.0f         /* room it needs ahead in a lane to move into it */
#define ONCOMING  3             /* at most, at once */
#define BEHIND    12.0f         /* how far one behind the car stays back */

/* speed range (m/s), how often */
static const struct { float v0, v1, often; } TYPE[T_COUNT] = {
    { 18, 25, 5 },              /* T_SEDAN */
    { 22, 28, 3 },              /* T_SPORTS */
    { 15, 20, 2 },              /* T_TRUCK */
    { 20, 27, 2 },              /* T_BIKE */
    { 17, 22, 2 },              /* T_VAN */
    { 18, 24, 2 },              /* T_PICKUP */
    { 15, 18, 1 },              /* T_BUS */
};

static const float PAINT[][3] = {
    { 0, 240, 255 }, { 255, 230, 60 }, { 120, 255, 140 }, { 255, 140, 40 },
    { 230, 230, 255 }, { 190, 90, 255 }, { 255, 80, 160 },
};
#define NPAINT ((int)(sizeof PAINT / sizeof PAINT[0]))

typedef struct {
    int      active, type, lane, oncoming;
    float    at, speed, base;       /* speed now, and its own (oncoming: towards us) */
    uint32_t id;
} vehicle;

static vehicle V[NV];
static struct { int lane, ready, held; float wander; } P;   /* the car's plans; held: a lane the camera wants, else -1 */
static uint32_t serial;

static float lane_x(int lane) { return -ROAD_HALF + (lane + 0.5f) * LANE_W; }

/* an oncoming lane's middle, as far as that carriageway is there */
static float on_lane_x(int lane, float d)
{
    float f = road_oncoming(d);
    return -ROAD_HALF - (MEDIAN + ON_HALF) * f + (lane - 0.5f) * LANE_W * f;
}

static float len(const vehicle *v) { return model_vehicle_length(v->type); }

/* Would the car, at x across, touch this one sideways (with a margin)? */
static int beside(const vehicle *v, float x)
{
    return fabsf(lane_x(v->lane) - x) < (model_vehicle_width(v->type) + CAR_W) / 2 + 0.3f;
}

/* ---- the traffic ------------------------------------------------------- */

static int pick_type(uint32_t id)
{
    float total = 0, r;
    for (int t = 0; t < T_COUNT; t++) total += TYPE[t].often;
    r = hash01(id) * total;
    int t = 0;
    while (t < T_COUNT - 1 && r >= TYPE[t].often) r -= TYPE[t++].often;
    return t;
}

static vehicle *free_slot(void)
{
    for (int i = 0; i < NV; i++) if (!V[i].active) return &V[i];
    return 0;
}

static void spawn(float at)
{
    for (int i = 0; i < NV; i++)
        if (V[i].active && !V[i].oncoming && fabsf(V[i].at - at) < SPACING) return;
    vehicle *v = free_slot();
    if (!v) return;
    uint32_t id = G.seed * 17u + (++serial) * 7919u;
    v->active = 1;
    v->oncoming = 0;
    v->type = pick_type(id);
    v->lane = (int)(hash01(id + 1) * LANES) % LANES;
    v->at = at;
    v->base = v->speed = TYPE[v->type].v0 + hash01(id + 2) * (TYPE[v->type].v1 - TYPE[v->type].v0);
    v->id = id;
}

/* Oncoming, only where its carriageway runs all the way to the car. */
static void spawn_oncoming(float at)
{
    int n = 0;
    for (int i = 0; i < NV; i++) {
        if (!V[i].active || !V[i].oncoming) continue;
        if (fabsf(V[i].at - at) < 40) return;
        n++;
    }
    if (n >= ONCOMING) return;
    for (float d = -30; d < at - G.s + 20; d += 10)
        if (road_oncoming(d) < 0.99f) return;
    vehicle *v = free_slot();
    if (!v) return;
    uint32_t id = G.seed * 29u + (++serial) * 6007u;
    v->active = 1;
    v->oncoming = 1;
    v->type = pick_type(id);
    v->lane = (int)(hash01(id + 1) * ON_LANES) % ON_LANES;
    v->at = at;
    v->base = v->speed = TYPE[v->type].v0 + hash01(id + 2) * (TYPE[v->type].v1 - TYPE[v->type].v0);
    v->id = id;
}

/* How far ahead of the car's front the nearest one in this lane is (its
 * back); one beside the car, or just behind it, counts as 0. */
static float gap(int lane)
{
    float g = 1e9f;
    for (int i = 0; i < NV; i++) {
        const vehicle *v = &V[i];
        if (!v->active || v->oncoming || v->lane != lane) continue;
        if (v->at + len(v) < G.s - BEHIND - 2) continue; /* well behind: gone by */
        g = fminf(g, fmaxf(v->at - (G.s + CAR_LEN), 0));
    }
    return g;
}

/* The nearest one ahead in the lane the car is going to, or in its way
 * where it is now (between two lanes): whose speed to keep. */
static const vehicle *ahead_of_car(float *dist)
{
    const vehicle *best = 0;
    *dist = 1e9f;
    for (int i = 0; i < NV; i++) {
        const vehicle *v = &V[i];
        if (!v->active || v->oncoming) continue;
        float half = model_vehicle_width(v->type) / 2 + 1.1f;
        if (v->lane != P.lane && fabsf(lane_x(v->lane) - G.car_x) > half) continue;
        float dz = v->at - (G.s + CAR_LEN);
        if (dz > -1 && dz < *dist) { *dist = dz; best = v; }
    }
    return best;
}

static void steer(float dt)
{
    if (!P.ready) { P.lane = LANES / 2; G.car_x = lane_x(P.lane); P.wander = 8; P.held = -1; P.ready = 1; }
    float here = gap(P.lane);
    int best = P.lane;
    if (P.held >= 0) {                          /* the camera wants a lane: there, step by step, and stay */
        int l = P.lane + (P.held > P.lane) - (P.held < P.lane);
        if (l != P.lane && gap(l) > ROOM) best = l;
    } else if (here < LOOK) {                          /* someone ahead: a freer lane beside? */
        float most = here + 8;
        for (int l = P.lane - 1; l <= P.lane + 1; l += 2) {
            if (l < 0 || l >= LANES) continue;
            float g = gap(l);
            if (g > ROOM && g > most) { most = g; best = l; }
        }
    } else if ((P.wander -= dt) <= 0) {         /* the road is free: a lane change, just so */
        int l = P.lane + (hash01(serial * 3u + (uint32_t)G.time) < 0.5f ? -1 : 1);
        if (l >= 0 && l < LANES && gap(l) > LOOK * 1.5f) best = l;
        P.wander = 7 + hash01((uint32_t)(G.time * 10)) * 9;
    }
    P.lane = best;

    float want = clampf((lane_x(P.lane) - G.car_x) * 1.6f, -3.2f, 3.2f);
    G.car_vx += (want - G.car_vx) * fminf(1, dt * 4);
    float x = G.car_x + G.car_vx * dt;
    for (int i = 0; i < NV; i++) {              /* never sideways into one beside it */
        const vehicle *v = &V[i];
        if (!v->active || v->oncoming || beside(v, G.car_x) || !beside(v, x)) continue;
        if (v->at < G.s + CAR_LEN + 2 && v->at + len(v) > G.s - BEHIND - 2) {
            x = G.car_x;
            G.car_vx = 0;
            P.lane = (int)clampf(floorf((G.car_x + ROAD_HALF) / LANE_W), 0, LANES - 1);
            break;
        }
    }
    G.car_x = x;
}

void traffic_update(float dt)
{
    int count = 0;
    for (int i = 0; i < NV; i++) {
        vehicle *v = &V[i];
        if (!v->active) continue;
        if (v->oncoming) {
            v->at -= v->speed * dt;
            if (v->at < G.s - 40) v->active = 0;
            continue;
        }
        v->speed = v->base;                     /* one catching up falls in behind */
        for (int j = 0; j < NV; j++) {
            const vehicle *w = &V[j];
            if (j == i || !w->active || w->oncoming || w->lane != v->lane) continue;
            float dz = w->at - (v->at + len(v));
            if (dz > -1 && dz < 15) v->speed = fminf(v->speed, w->base);
        }
        float behind_car = G.s - (v->at + len(v));  /* behind the car, in its way: stay back */
        int behind = beside(v, G.car_x) && behind_car > -1;
        if (behind && behind_car < BEHIND + 15) v->speed = fminf(v->speed, G.speed);
        v->at += v->speed * dt;
        if (behind && v->at + len(v) > G.s - BEHIND) v->at = G.s - BEHIND - len(v);
        if (v->at + len(v) < G.s - 40 || v->at > G.s + SPAWN_AT + 140) v->active = 0;
        else count++;
    }

    /* the louder, the busier */
    static int filled;
    int wanted = 3 + (int)((G.have ? G.m.level : 0.2f) * 6);
    if (!filled) {                              /* the first: some already on the road */
        for (float at = G.s + 40; at < G.s + SPAWN_AT; at += 50) spawn(at + hash01(serial) * 10);
        filled = 1;
    } else if (count < wanted && hash01(serial + (uint32_t)(G.time * 60)) < dt * 2) {
        spawn(G.s + SPAWN_AT + hash01(serial * 5u) * 60);
    }
    if (hash01(serial * 11u + (uint32_t)(G.time * 60)) < dt * 0.8f)
        spawn_oncoming(G.s + SPAWN_AT + 40 + hash01(serial * 13u) * 40);
    steer(dt);
}

void traffic_hold_lane(int lane) { P.held = lane; }

int traffic_in_lane(int lane) { return P.lane == lane && fabsf(G.car_x - lane_x(lane)) < 0.15f; }

float traffic_room(int lane) { return gap(lane); }

float traffic_limit(void)
{
    float limit = 1e9f;
    for (int i = 0; i < NV; i++) {
        const vehicle *v = &V[i];
        if (!v->active || v->oncoming || !beside(v, G.car_x)) continue;
        if (v->at + len(v) < G.s) continue;                     /* behind it */
        limit = fminf(limit, v->at - CAR_LEN - 2);
    }
    return limit;
}

float traffic_speed_cap(void)
{
    float dist;
    const vehicle *v = ahead_of_car(&dist);
    if (!v || dist > 40) return 1e9f;
    return v->speed + fmaxf(0, dist - 10) * 0.8f;  /* settle 10 m behind it */
}

/* ---- drawing them ------------------------------------------------------ */

void traffic_draw(vp3d_scene *sc, vp_dvg *d)
{
    for (int i = 0; i < NV; i++) {
        const vehicle *v = &V[i];
        if (!v->active) continue;
        float dz = v->at - G.s;
        if (dz < -35 || dz > SPAWN_AT + 120) continue;
        float x = v->oncoming ? on_lane_x(v->lane, dz) : lane_x(v->lane);
        vp_vec3 p = road_point(dz, x, 0);
        vp_xf place = vp_xf_mul2(vp_xf_translate(p.x, p.y, p.z),
                                 vp_xf_rot_y(road_heading(dz) + (v->oncoming ? PI_F : 0)));
        const float *c = PAINT[v->id % NPAINT];
        color_f(d, c[0], c[1], c[2], fade(dz));
        vp3d_scene_mesh(sc, model_vehicle(v->type), &place, VP3D_SOLID);
        if (dz < 80) {                                          /* the wheels turning, near enough to see */
            color_f(d, 200, 200, 255, fade(dz));
            model_spokes(sc, v->type, &place, (v->oncoming ? -v->at : v->at) * WHEEL_TURN);
        }
    }
}
