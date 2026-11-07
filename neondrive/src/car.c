/* car.c — the car, and the camera following it.
 *
 * The car is a low wedge with a light bar across its tail, louvres over the
 * rear window and a wing (models.c). It goes from lane to lane past the
 * traffic (traffic.c), turning a little into a lane change and leaning out
 * of it.
 *
 * The camera follows it from behind, from one of a few places (low, high,
 * to one side, far back), and every half minute or so glides slowly over
 * to another. One of them swings round behind the car out to its right and
 * slowly back, while it is held: first the car moves over to the right
 * lane (traffic_hold_lane), so that the camera swings out over the
 * roadside, where nothing drives. It swings only with the lane free well
 * ahead (so the car need not slow down), and gives up after SWING_WAIT s
 * if it cannot get there.
 */
#include "saver.h"

#include <math.h>

static struct { float yaw, roll; } C;

/* swing: how far (radians) it goes round the car to the right and back */
typedef struct { float back, height, side, ahead, look_h, fov, swing; } shot;

static const shot SHOTS[] = {
    {  7.5f, 2.6f,  0.0f, 25, 1.0f, 62, 0 },      /* close behind */
    { 12.0f, 6.0f,  0.0f, 30, 0.0f, 58, 0 },      /* high */
    {  5.8f, 1.15f, 0.0f, 30, 1.1f, 66, 0 },      /* low, at its bumper */
    {  7.0f, 2.2f,  3.4f, 20, 1.0f, 62, 0 },      /* to its right */
    { 22.0f, 9.0f, -5.0f, 40, 0.0f, 50, 0 },      /* far back and up, from the left */
    {  6.0f, 1.4f, -3.0f, 18, 1.0f, 64, 0 },      /* low, to its left */
    {  8.0f, 2.0f,  0.0f, 14, 0.9f, 60, 1.0f },   /* behind, swinging out to the right */
};
#define CAR_MID 2.25f                          /* the car's middle, ahead of its back */
#define NSHOTS ((int)(sizeof SHOTS / sizeof SHOTS[0]))
#define GLIDE  5.0f                            /* s from one to the next */
#define SWING_WAIT 12.0f                       /* s the swing waits for the right lane */
#define OVER_TRAFFIC 6.0f                      /* m up, gliding across the lanes */
#define SWING_ROOM 180.0f                      /* m free ahead in the right lane, to swing: no slowing down */
#define SWING_TIME 14.0f                       /* s the swing takes */

/* swinging: the car is in the right lane, the swing goes on (else it waits) */
static struct { int from, to, ready, swinging; float t, hold, length, wait; } K;

void car_update(float dt)
{
    float k = fminf(1, dt * 4);
    C.yaw += (G.car_vx * 0.035f - C.yaw) * k;
    C.roll += (-G.car_vx * 0.012f - G.curve * 4 - C.roll) * k;
}

void camera_update(float dt)
{
    if (!K.ready) {
        vp3d_view_init(&G.view, 62);
        K.from = K.to = 0;
        K.t = 1;
        K.hold = K.length = 20;
        K.ready = 1;
    }
    K.t = fminf(1, K.t + dt / GLIDE);
    if (SHOTS[K.to].swing > 0 && !K.swinging) {  /* the swing: first to the right lane */
        K.wait += dt;
        if (traffic_in_lane(RIGHT_LANE) && traffic_room(RIGHT_LANE) > SWING_ROOM) {   /* there, and free: swing */
            K.swinging = 1;
            K.hold = K.length = SWING_TIME;
        }
        else if (K.wait > SWING_WAIT) {         /* no room there: another shot instead */
            K.to = 0;
            traffic_hold_lane(-1);
        }
    } else if ((K.hold -= dt) <= 0) {           /* on to another place */
        K.from = K.to;
        /* any of the others: one step on from this one, by 1 .. NSHOTS-1 (never the same) */
        K.to = (K.from + 1 + (int)(hash01((uint32_t)(G.time * 1000) + G.seed) * (NSHOTS - 1)) % (NSHOTS - 1)) % NSHOTS;
        K.t = 0;
        K.hold = K.length = 18 + hash01((uint32_t)(G.time * 77)) * 14;
        K.swinging = 0;
        K.wait = 0;
        traffic_hold_lane(SHOTS[K.to].swing > 0 ? RIGHT_LANE : -1);
    }
    const shot *a = &SHOTS[K.from], *b = &SHOTS[K.to];
    float t = smooth01(K.t);
#define MIX(f) (a->f + (b->f - a->f) * t)
    /* round the car's middle by `round` radians: out to the right and back,
     * smoothly, over the time the shot is held */
    float through = K.swinging ? clampf(1 - K.hold / K.length, 0, 1) : 0;
    float round = MIX(swing) * sinf(PI_F * through) * sinf(PI_F * through);
    float r = MIX(back) + CAR_MID;
    float side = MIX(side), x = G.car_x * 0.7f + side + sinf(round) * r;
    /* a shot to one side stays off the road (over the roadside, or the
     * median on the left), whatever lane the car is in: the traffic it
     * passes never drives through the camera */
    float off = smooth01(fabsf(side) / 2.5f);
    if (side > 0) x += (fmaxf(x, ROAD_HALF + 0.8f) - x) * off;
    if (side < 0) x += (fminf(x, -ROAD_HALF - 0.8f) - x) * off;
    /* gliding across the lanes from one side to the other, it goes up and
     * over the traffic (higher than any truck), not through it */
    float over = smooth01(fabsf(b->side - a->side) / 3) * sinf(PI_F * K.t);
    float height = fmaxf(MIX(height), MIX(height) + (OVER_TRAFFIC - MIX(height)) * over);
    vp_vec3 eye = road_point(CAR_MID - cosf(round) * r, x, height);
    vp_vec3 target = road_point(MIX(ahead), G.car_x * 0.5f, MIX(look_h));
    G.view.fov_deg = MIX(fov);
#undef MIX
    vp3d_look_at(&G.view, eye, target, vp_v3(0, 1, 0));
    G.eye = eye;
    G.cam_yaw = atan2f(target.x - eye.x, target.z - eye.z);
}

void car_draw(vp3d_scene *sc, vp_dvg *d)
{
    vp_vec3 p = road_point(0, G.car_x, 0);
    vp_xf place = vp_xf_mul2(vp_xf_translate(p.x, p.y, p.z),
                             vp_xf_mul2(vp_xf_rot_y(road_heading(0) + C.yaw), vp_xf_rot_z(C.roll)));
    color_f(d, 255, 60, 200, 1);
    vp3d_scene_mesh(sc, model_car(), &place, VP3D_SOLID);
    color_f(d, 220, 220, 255, 1);
    model_spokes(sc, -1, &place, G.s * WHEEL_TURN);
}
