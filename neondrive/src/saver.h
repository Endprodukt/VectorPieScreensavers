/* saver.h — Neon Drive: an endless drive into an 80s sunset, to the music.
 *
 * A 3D world (the SDK's vp_3d, drawn through one scene so that solid things
 * hide what is behind them), in metres. The car is always at the origin of
 * this frame's world, the road's heading there along +z; the road ahead
 * and behind is laid out from it each frame (road.c). +x right, +y up, +z
 * forward (left-handed, as the SDK).
 *
 * The parts:
 *   road.c     the road: its bends, its stretches (beach, city, desert ...,
 *              with or without the oncoming carriageway), the grid, the lines
 *   models.c   the meshes: the car, the traffic, what stands by the road
 *   props.c    what stands by the road, stretch by stretch
 *   traffic.c  the others on the road (both ways), and how the car gets by
 *   flyers.c   birds, and a plane towing a VECTORPIE banner
 *   car.c      the car and the camera following it
 *   scenery.c  the far scenes on the horizon and the sun
 *   main.c     the music, the timing, the title, the scene
 */
#ifndef SAVER_H
#define SAVER_H

#include <vectorpie/vp.h>

#include <stdint.h>

#define W          VP_DVG_COORD_MAX
#define CX         (W / 2)
#define PI_F       3.14159265f

#define LANES      3                    /* ours, all one way */
#define LANE_W     3.6f
#define ROAD_HALF  (LANES * LANE_W / 2)
#define ON_LANES   2                    /* the oncoming carriageway, where there is one */
#define ON_HALF    (ON_LANES * LANE_W / 2)
#define MEDIAN     3.0f
#define ON_SHIFT   (MEDIAN + 2 * ON_HALF)               /* how far it pushes the left side out */
#define ON_CENTER  (-(ROAD_HALF + MEDIAN + ON_HALF))    /* its middle, when fully there */

#define WHEEL_TURN 0.3f                 /* how fast wheels seem to turn: radians a metre */
#define ROAD_BACK  40.0f                /* the road is laid out this far behind the car */
#define ROAD_AHEAD 420.0f               /* and this far ahead */

/* Everything that changes, shared by the parts. Nothing pulses or blinks:
 * the music sets only the speed and how busy the road is, smoothly. */
typedef struct {
    float    time;
    float    s;                         /* metres driven: the car's place on the road */
    float    speed;                     /* metres a second */
    float    heading;                   /* the road's heading at the car, all turns added up */
    float    curve;                     /* its curvature there, smoothed (1/m) */
    float    car_x, car_vx;             /* the car across the road, and how fast it moves so */
    uint32_t seed;

    /* the camera (car.c) */
    vp3d_view view;
    vp_vec3  eye;
    float    cam_yaw;                   /* where it looks, about y, in this frame's world */

    /* the music (main.c) */
    vp_music_feed m;
    int      have;
} world;

extern world G;

/* small helpers (main.c) */
float    clampf(float v, float lo, float hi);
float    smooth01(float t);             /* 0 .. 1, eased */
float    hash01(uint32_t i);            /* 0 .. 1 */
void     color_f(vp_dvg *d, float r, float g, float b, float k);  /* 0..255 times k */
float    fade(float dist);              /* brightness by distance from the camera */

/* road.c */
enum { Z_BEACH, Z_CITY, Z_DESERT, Z_POWER, Z_TUNNEL, Z_HIGHWAY, Z_COUNT };
#define ZONE_LEN 520.0f
float    road_curv(float s);
void     road_build(void);              /* once per frame, before the rest */
vp_vec3  road_point(float d, float x, float y);   /* d metres ahead of the car, x across, y up */
float    road_heading(float d);         /* this frame's world */
float    road_oncoming(float d);        /* 0 .. 1: how much of the oncoming carriageway is there */
int      zone_kind(long n);             /* stretch n: [n * ZONE_LEN, (n + 1) * ZONE_LEN) of G.s */
void     road_draw(vp3d_scene *sc, vp_dvg *d);

/* models.c */
enum { T_SEDAN, T_SPORTS, T_TRUCK, T_BIKE, T_VAN, T_PICKUP, T_BUS, T_COUNT };
enum { P_PALM, P_FRONDS, P_LAMP, P_PYLON, P_BILLBOARD1, P_BILLBOARD2, P_BILLBOARD3, P_CACTUS, P_ROCK,
       P_GANTRY, P_GANTRY_VP, P_ARCH, P_BOX, P_SIGN, P_COUNT };
#define WORD_W 8.7f                     /* VECTORPIE, 1 high */
int         vectorpie_strokes(float (*seg)[4], int max);   /* x0 y0 x1 y1, x from 0 to WORD_W */
int         models_init(void);
vp3d_mesh  *model_car(void);
vp3d_mesh  *model_vehicle(int type);
float       model_vehicle_length(int type);
float       model_vehicle_width(int type);
vp3d_mesh  *model_prop(int kind);
/* the spokes on a vehicle's wheels (type -1: the car), turned by `turn` */
void        model_spokes(vp3d_scene *sc, int type, const vp_xf *place, float turn);

/* props.c */
void     props_draw(vp3d_scene *sc, vp_dvg *d);

/* traffic.c */
void     traffic_update(float dt);
float    traffic_speed_cap(void);       /* how fast the car may go, behind someone */
float    traffic_limit(void);           /* how far the car may go: never into anyone */
void     traffic_hold_lane(int lane);   /* the camera wants the car in this lane (-1: any) */
int      traffic_in_lane(int lane);     /* is it there, settled? */
float    traffic_room(int lane);        /* metres free ahead of the car in this lane */
#define  RIGHT_LANE (LANES - 1)
void     traffic_draw(vp3d_scene *sc, vp_dvg *d);

/* flyers.c */
void     flyers_update(float dt);
void     flyers_draw(vp3d_scene *sc, vp_dvg *d);

/* car.c */
void     car_update(float dt);
void     camera_update(float dt);       /* after car_update, before drawing */
void     car_draw(vp3d_scene *sc, vp_dvg *d);

/* scenery.c */
void     scenery_update(float dt);
void     scenery_next(void);            /* a new track: on to the next scene, in a while */
void     sky_draw(vp3d_scene *sc, vp_dvg *d);

#endif
