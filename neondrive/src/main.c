/* Neon Drive — a VectorPie screensaver: an endless drive into an 80s sunset.
 *
 * A car drives down an endless road over a glowing grid, through bends and
 * stretches of every kind (beach, city, desert, power lines, neon tunnel,
 * highway; now and then with an oncoming carriageway beside it), weaving
 * through the traffic: cars, sports cars, trucks, motorbikes, vans,
 * pickups, buses, all in 3D, under flocks of birds and now and then a
 * plane towing a VECTORPIE banner. The camera follows it from changing
 * places.
 * Towards a striped sun on the horizon one far scene follows another (a
 * city, mountains, pyramids, monuments, mesas): they rise, come a little
 * nearer and sink away again, never reached.
 *
 * To the music, smoothly: the speed and the traffic follow the loudness.
 * Nothing pulses, flashes or blinks. A new track shows its title. Without
 * music it drives on, quietly.
 *
 * On the cabinet: installed from the .vps (tools/make_vps.sh) in SETTINGS ->
 * SCREENSAVERS -> INSTALL, chosen in SETTINGS -> SCREENSAVERS -> SELECT. Any
 * control wakes the cabinet, START 2 skips to the next track.
 * On a PC: saver_neondrive <a .ogg/.wav or a folder of them>. Esc quits.
 * SAVER_SHOTS=<folder> saves a picture of the screen every 2 s (for
 * development: the window itself cannot be photographed on Windows).
 */
#include "saver.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

world G;

static struct {
    unsigned last_track;
    float    title_time;                /* s the title still shows */
    char     title[VP_MUSIC_TEXT];
} M;

static vp3d_scene *scene;

/* ---- helpers ----------------------------------------------------------- */

float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }

float smooth01(float t)
{
    t = clampf(t, 0, 1);
    return t * t * (3 - 2 * t);
}

float hash01(uint32_t i)
{
    i *= 2654435761u;
    i ^= i >> 15;
    i *= 2246822519u;
    i ^= i >> 13;
    return (float)(i & 0xffffff) / 16777215.0f;
}

void color_f(vp_dvg *d, float r, float g, float b, float k)
{
    k = clampf(k, 0, 1);
    vp_color(d, (uint8_t)clampf(r * k, 0, 255), (uint8_t)clampf(g * k, 0, 255), (uint8_t)clampf(b * k, 0, 255));
}

float fade(float dist) { return clampf(1.15f - dist / 380, 0.12f, 1); }

/* ---- the music --------------------------------------------------------- */

static void music(void)
{
    G.have = vp_saver_music(&G.m);
    if (G.have && G.m.track != M.last_track) {  /* a new track: its title, the next scene */
        M.last_track = G.m.track;
        snprintf(M.title, sizeof M.title, "%s", G.m.title);
        M.title_time = M.title[0] ? 7 : 0;
        scenery_next();
    }
}

/* ---- the game ---------------------------------------------------------- */

static int init(vp_game *g)
{
    (void)g;
    G.seed = (uint32_t)vp_ticks_ms() * 2654435761u;   /* a different road each run */
    G.speed = 30;
    if (models_init() != VP_OK) return VP_ERR_NOMEM;
    if (vp3d_scene_create(&scene) != VP_OK) return VP_ERR_NOMEM;
    vp3d_scene_set_contact(scene, 0.3f);        /* lines on the ground stay seen */
    return VP_OK;
}

static void update(vp_game *g, float dt)
{
    (void)g;
    G.time += dt;
    music();
    static float loud;                          /* the loudness, slowed right down */
    loud += ((G.have ? G.m.level : 0) - loud) * fminf(1, dt * 0.25f);
    float target = fminf(30 + loud * 16, traffic_speed_cap());
    G.speed += (target - G.speed) * fminf(1, dt * (target < G.speed ? 3.0f : 0.8f));   /* brake harder */
    float c = road_curv(G.s);
    float s = fminf(G.s + G.speed * dt, traffic_limit());     /* never into anyone */
    if (s < G.s + G.speed * dt) G.speed = fmaxf(0, (s - G.s) / dt);
    G.s = fmaxf(G.s, s);
    G.heading += c * G.speed * dt;
    G.curve += (c - G.curve) * fminf(1, dt * 2);
    M.title_time = fmaxf(0, M.title_time - dt);
    road_build();
    scenery_update(dt);
    traffic_update(dt);
    flyers_update(dt);
    car_update(dt);
    camera_update(dt);
}

/* The track's title over the sky, in two colors a little apart, then gone. */
static void title(vp_dvg *d)
{
    if (M.title_time <= 0) return;
    float k = smooth01(M.title_time / 1.5f) * smooth01((7 - M.title_time) / 0.5f);
    vp_text_font(VP_FONT_ASTEROIDS);
    color_f(d, 255, 40, 200, k);
    vp_text_fit(d, M.title, CX + 14, W * 0.915f - 14, 120, W * 0.84f, VP_TEXT_CENTER);
    color_f(d, 0, 240, 255, k);
    vp_text_fit(d, M.title, CX, W * 0.915f, 120, W * 0.84f, VP_TEXT_CENTER);
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
    vp3d_scene_begin(scene, d, &G.view);
    /* the ground: invisible, hiding all below it (a scene sinking away) */
    const float r = 9000;
    vp_vec3 ground[4] = { vp_v3(G.eye.x - r, 0, G.eye.z - r), vp_v3(G.eye.x + r, 0, G.eye.z - r),
                          vp_v3(G.eye.x + r, 0, G.eye.z + r), vp_v3(G.eye.x - r, 0, G.eye.z + r) };
    vp3d_scene_polygon(scene, ground, 4, 0);
    sky_draw(scene, d);
    road_draw(scene, d);
    props_draw(scene, d);
    traffic_draw(scene, d);
    flyers_draw(scene, d);
    car_draw(scene, d);
    vp3d_scene_end(scene);
    title(d);
    shots(d);
}

#ifndef SAVER_SOAK
int main(int argc, char **argv)
{
    vp_game_desc desc = {0};
    desc.title = "Neon Drive";
    desc.init = init;
    desc.update = update;
    desc.draw = draw;
    return vp_run(&desc, argc, argv);
}
#else
/* saver_soak [hours] [seed]: a test, not the screensaver. Hours of driving
 * as fast as the computer goes, without music, drawing a frame every half
 * second (into the frame only: nothing is shown); prints the heaviest
 * frames and where it got to. For crashes that take long to come.
 * Its window is hidden at once, and its messages answered as it goes, so
 * that it never shows, nor counts as hanging ("not responding"). */
#define SDL_MAIN_HANDLED
#include <SDL.h>

int main(int argc, char **argv)
{
    double hours = argc > 1 ? atof(argv[1]) : 1;
    vp_dvg_options o = {0};
    o.output = VP_DVG_OUTPUT_WINDOW;
    o.window_title = "Neon Drive soak";
    vp_dvg *d;
    if (vp_dvg_open_ex(&o, &d) != VP_OK || init(NULL) != VP_OK) return 1;
    if (SDL_GL_GetCurrentWindow()) SDL_HideWindow(SDL_GL_GetCurrentWindow());
    if (argc > 2) G.seed = (uint32_t)strtoul(argv[2], 0, 10);
    printf("soak: seed %u, %.2f h\n", G.seed, hours);
    long steps = (long)(hours * 3600 * 60);
    uint32_t peak = 0;
    for (long i = 0; i < steps; i++) {
        update(NULL, 1 / 60.0f);
        if (i % 30) continue;
        SDL_PumpEvents();                       /* (answering the window's messages) */
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
        vp_dvg_begin_frame(d);
        draw(NULL, d);
        uint32_t n = 0;
        vp3d_scene_stats st;
        vp3d_scene_get_stats(scene, &st);
        n = st.pieces;
        if (n > peak) {
            peak = n;
            printf("soak: %.0f s, %.0f m: %u lines (zone %d)\n", G.time, G.s, n,
                   zone_kind((long)floorf(G.s / ZONE_LEN)));
        }
        if (i % (60 * 600) == 0) { printf("soak: %.0f min\n", G.time / 60); fflush(stdout); }
    }
    printf("soak: done, %.2f h, %.0f km, heaviest %u lines\n", hours, G.s / 1000, peak);
    vp_dvg_close(d);
    return 0;
}
#endif
