/* Neon Ridge — a VectorPie screensaver: a still night. Under a starry sky,
 * its horizon glowing cyan, a mountain range drawn in contour lines; before
 * it a dark plain of lines coming slowly nearer, and across the plain a
 * pink trail of light sweeping off into the distance.
 *
 * In the manner of the retrowave mountain screensaver videos: the camera
 * stands still, the plain and the trail are what move.
 *
 * To the music, smoothly: the plain comes faster when it is louder, the
 * trail sways with it, the horizon glows a little brighter. Nothing flashes,
 * blinks or pulses; the stars twinkle, softly and each in its own time. A new
 * track shows its title.
 *
 * On the cabinet: installed in SETTINGS -> SCREENSAVERS (tools/deploy.sh or
 * the .vps). Any control wakes the cabinet, START 2 skips to the next track.
 * On a PC: saver_neonridge <a .ogg/.wav or a folder of them>. Esc quits.
 * SAVER_SHOTS=<folder> saves a picture every 2 s (for development).
 */
#include <vectorpie/vp.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define W         VP_DVG_COORD_MAX
#define CX        (W / 2.0f)
#define PI        3.14159265f
#define EYE_Y     8.0f
#define PITCH     0.115f                /* tan of the look upwards: the horizon below the middle */
/* the range: rows of contour lines across, front to back */
#define M_ROWS    30
#define M_COLS    41
#define M_Z0      700.0f
#define M_Z1      1150.0f
#define M_X       1000.0f               /* from -M_X to M_X */
/* the plain */
#define P_RATIO   1.17f                 /* each line this much farther than the last */
#define P_NEAR    19.0f                 /* (nearer is below the screen) */
#define P_FAR     220.0f
#define HORIZON_Z 700.0f                /* the plain's far edge */
#define STARS     140
#define TRAIL_N   48

static struct {
    float time, loud, flow, sway;       /* flow: how far the plain has come; sway: the trail's phase */
    vp_music_feed m;
    int have;
    unsigned last_track;
    float title_time;
    char title[VP_MUSIC_TEXT];
    uint32_t seed;
} G;

static float MH[M_ROWS][M_COLS];        /* the range's heights */
static float STAR[STARS][5];            /* direction across, up, brightness, twinkle rate, phase */

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

static float noise2(float x, float y)
{
    int ix = (int)floorf(x), iy = (int)floorf(y);
    float fx = smooth01(x - ix), fy = smooth01(y - iy);
#define HN(a, b) hash01((uint32_t)(a) * 73856093u ^ (uint32_t)(b) * 19349663u)
    float a = HN(ix, iy), b = HN(ix + 1, iy), c = HN(ix, iy + 1), d = HN(ix + 1, iy + 1);
#undef HN
    return (a + (b - a) * fx) + ((c + (d - c) * fx) - (a + (b - a) * fx)) * fy;
}

static void color(vp_dvg *d, float r, float g, float b, float k)
{
    k = clampf(k, 0, 1);
    vp_color(d, (uint8_t)clampf(r * k, 0, 255), (uint8_t)clampf(g * k, 0, 255), (uint8_t)clampf(b * k, 0, 255));
}

/* ---- the range ------------------------------------------------------------- */

/* a peak at (px, pz), h high, w wide: a sharp top, long slopes */
static float peak(float x, float z, float px, float pz, float h, float w)
{
    float dx = x - px, dz = (z - pz) * 1.3f;
    return h * expf(-sqrtf(dx * dx + dz * dz) / w);
}

static void range_make(void)
{
    /* the tall one in the middle, lower ones beside it, a ridge between */
    const float P[][4] = {
        { 0, 880, 400, 230 }, { -640, 940, 210, 210 }, { 660, 900, 230, 200 },
        { -330, 850, 170, 180 }, { 340, 920, 180, 190 }, { -920, 880, 150, 170 }, { 930, 950, 170, 170 },
    };
    for (int r = 0; r < M_ROWS; r++)
        for (int c = 0; c < M_COLS; c++) {
            float z = M_Z0 + (M_Z1 - M_Z0) * r / (M_ROWS - 1), x = -M_X + 2 * M_X * c / (M_COLS - 1);
            float h = 45 + 25 * noise2(x * 0.004f, z * 0.004f);
            for (int i = 0; i < 7; i++) h = fmaxf(h, peak(x, z, P[i][0], P[i][1], P[i][2], P[i][3]) + h * 0.3f);
            /* rough slopes: the contour lines wander */
            h *= 0.82f + 0.36f * noise2(x * 0.03f + 7, z * 0.03f);
            /* down to the plain at the front and the sides */
            h *= smooth01((z - M_Z0) / 120 + 0.15f) * smooth01((M_X - fabsf(x)) / 150 + 0.2f);
            MH[r][c] = h;
        }
}

static void range_draw(vp_dvg *d)
{
    /* invisible screens between the rows: the front hides the back */
    for (int r = 0; r + 1 < M_ROWS; r++) {
        float z0 = M_Z0 + (M_Z1 - M_Z0) * r / (M_ROWS - 1), z1 = M_Z0 + (M_Z1 - M_Z0) * (r + 1) / (M_ROWS - 1);
        for (int c = 0; c + 1 < M_COLS; c++) {
            float x0 = -M_X + 2 * M_X * c / (M_COLS - 1), x1 = -M_X + 2 * M_X * (c + 1) / (M_COLS - 1);
            vp_vec3 a = vp_v3(x0, MH[r][c], z0), b = vp_v3(x1, MH[r][c + 1], z0);
            vp_vec3 e = vp_v3(x1, MH[r + 1][c + 1], z1), f = vp_v3(x0, MH[r + 1][c], z1);
            vp_vec3 t0[3] = { a, b, e }, t1[3] = { a, e, f };
            vp3d_scene_polygon(scene, t0, 3, 0);
            vp3d_scene_polygon(scene, t1, 3, 0);
        }
    }
    /* the contour lines, back and forth; higher is lighter, as if lit from the glow */
    for (int r = 0; r < M_ROWS; r++) {
        float z = M_Z0 + (M_Z1 - M_Z0) * r / (M_ROWS - 1);
        float k = 0.55f + 0.45f * r / (M_ROWS - 1);            /* the back rows make the skyline */
        for (int i = 0; i + 1 < M_COLS; i++) {
            int c = r & 1 ? M_COLS - 2 - i : i, a = r & 1 ? c + 1 : c, b = r & 1 ? c : c + 1;
            float t = clampf(fmaxf(MH[r][a], MH[r][b]) / 400, 0, 1);
            color(d, 70 + 60 * t, 90 + 150 * t, 230 + 25 * t, k * (0.7f + 0.3f * t));
            vp3d_scene_line(scene, vp_v3(-M_X + 2 * M_X * a / (M_COLS - 1), MH[r][a], z),
                            vp_v3(-M_X + 2 * M_X * b / (M_COLS - 1), MH[r][b], z));
        }
    }
}

/* ---- the sky -------------------------------------------------------------- */

static void sky_draw(vp_dvg *d)
{
    /* the glow above the horizon, behind the range: lines, cyan to blue, fading upwards */
    const float z = 4000;
    float glow = 0.75f + 0.25f * G.loud;
    for (int i = 0; i < 9; i++) {
        float t = i / 8.0f, y = z * (0.05f + 0.25f * powf(t, 1.3f));
        float half = z * 0.95f;
        color(d, 0 + 50 * t, 235 - 155 * t, 255, glow * (1.2f - 0.9f * t));
        if (i & 1) vp3d_scene_line(scene, vp_v3(half, y, z), vp_v3(-half, y, z));
        else       vp3d_scene_line(scene, vp_v3(-half, y, z), vp_v3(half, y, z));
    }
    /* the stars: of different brightness, in bands back and forth; each
     * twinkles, smoothly, two waves of its own making it uneven */
    const float sz = 5000;
    for (int i = 0; i < STARS; i++) {
        float x = STAR[i][0] * sz, y = STAR[i][1] * sz, w = STAR[i][3] * G.time + STAR[i][4];
        float tw = 0.5f + 0.3f * sinf(w) + 0.2f * sinf(w * 1.73f + 2.1f);    /* 0..1 */
        float k = STAR[i][2] * (0.25f + 0.75f * tw * tw);
        color(d, 170, 190, 255, k);
        vp3d_scene_line(scene, vp_v3(x, y, sz), vp_v3(x + 4, y, sz));
    }
}

static void stars_make(void)
{
    /* sorted into bands, each band left to right or back: a short way between them */
    const int BANDS = 7;
    int n = 0;
    for (int b = 0; b < BANDS && n < STARS; b++) {
        int count = STARS / BANDS, start = n;
        for (int i = 0; i < count && n < STARS; i++, n++) {
            float up = 0.02f + 0.75f * (b + hash01((uint32_t)n * 3 + 1)) / BANDS;
            up = 0.03f + up * up * 1.2f;                       /* fewer low down, in the glow */
            STAR[n][0] = (hash01((uint32_t)n * 3) - 0.5f) * 1.9f;
            STAR[n][1] = up;
            STAR[n][2] = 0.3f + 0.7f * hash01((uint32_t)n * 3 + 2) * hash01((uint32_t)n * 3 + 2);
            STAR[n][3] = 1.2f + 2.4f * hash01((uint32_t)n * 7 + 5);   /* a wave every 1.7 to 5 s */
            STAR[n][4] = 2 * PI * hash01((uint32_t)n * 7 + 6);
        }
        for (int i = start + 1; i < n; i++)                    /* by x, alternate bands reversed */
            for (int j = i; j > start && ((STAR[j][0] < STAR[j - 1][0]) ^ (b & 1)); j--) {
                for (int k = 0; k < 5; k++) { float t = STAR[j][k]; STAR[j][k] = STAR[j - 1][k]; STAR[j - 1][k] = t; }
            }
    }
}

/* ---- the plain and the trail ------------------------------------------------- */

static void plain_draw(vp_dvg *d)
{
    /* lines across, coming nearer, fading into the distance; each farther
     * than the last by a ratio (evenly apart on the screen, as in the
     * distance) */
    float ph = fmodf(G.flow, 1);
    int i = 0;
    for (float z = P_FAR * powf(P_RATIO, -ph); z > P_NEAR; z /= P_RATIO, i++) {
        float k = smooth01((z - P_NEAR) / 6) * (1 - 0.75f * smooth01(z / P_FAR));
        float half = z * 0.85f + 10;
        color(d, 120, 90, 255, k * 0.7f);
        if (i & 1) vp3d_scene_line(scene, vp_v3(half, 0, z), vp_v3(-half, 0, z));
        else       vp3d_scene_line(scene, vp_v3(-half, 0, z), vp_v3(half, 0, z));
    }
    /* the far edge: a long line, pink on the left to cyan on the right */
    const int SEG = 12;
    for (int s = 0; s < SEG; s++) {
        float t0 = (float)s / SEG, t1 = (float)(s + 1) / SEG, half = HORIZON_Z * 0.95f;
        color(d, 255 - 230 * t0, 40 + 190 * t0, 220 + 35 * t0, 0.9f);
        vp3d_scene_line(scene, vp_v3(-half + 2 * half * t0, 0, HORIZON_Z), vp_v3(-half + 2 * half * t1, 0, HORIZON_Z));
    }
}

/* the trail's path on the plain, s 0 (near, left) .. 1 (far, off to the left) */
static vp_vec3 trail_at(float s)
{
    /* in from the left near by, out to the right, then far away back left */
    /* across the screen: u -1 its left edge, 1 its right (at that distance);
     * out to the right edge not far below the horizon, there round, and back
     * along the horizon */
    float z, u;
    if (s < 0.55f) {
        float t = s / 0.55f;
        z = 24 * powf(140.0f / 24, powf(t, 0.55f));    /* quickly away: it bends towards the horizon */
        u = -1.15f + 2.3f * (1 - (1 - t) * (1 - t));
    } else {
        float t = (s - 0.55f) / 0.45f;
        z = 140 * powf(640.0f / 140, t);
        u = 1.15f - 2.4f * smooth01(t * 1.1f - 0.05f);
    }
    u += 0.08f * sinf(G.sway + s * 5) * (1 - s);             /* a slow sway, more near by */
    return vp_v3(u * z * 0.73f, 0.05f, z);
}

static void trail_draw(vp_dvg *d)
{
    /* a bright core and a glow either side, as three lines (out, back, out) */
    vp_vec3 p[TRAIL_N];
    for (int i = 0; i < TRAIL_N; i++) p[i] = trail_at((float)i / (TRAIL_N - 1));
    static const float SIDE[3] = { -1, 0, 1 };
    for (int l = 0; l < 3; l++) {
        float w = SIDE[l] * 0.35f;
        for (int j = 0; j + 1 < TRAIL_N; j++) {
            int a = l == 1 ? TRAIL_N - 1 - j : j, b = l == 1 ? a - 1 : a + 1;
            float far = (float)(a > b ? a : b) / (TRAIL_N - 1);
            vp_vec3 dir = vp_v3_normalize(vp_v3_sub(p[b], p[a]));
            vp_vec3 side = vp_v3(dir.z, 0, -dir.x);
            float wa = w * (1 + p[a].z * 0.01f), wb = w * (1 + p[b].z * 0.01f);
            if (l == 1) color(d, 255, 190, 255, 1 - 0.3f * far);
            else        color(d, 255, 0, 200, 0.75f - 0.35f * far);
            vp3d_scene_line(scene, vp_v3_add(p[a], vp_v3_scale(side, wa)), vp_v3_add(p[b], vp_v3_scale(side, wb)));
        }
    }
}

/* ---- the game --------------------------------------------------------------- */

static int init(vp_game *g)
{
    (void)g;
    G.seed = (uint32_t)vp_ticks_ms() * 2654435761u;     /* the range's details differ each run */
    range_make();
    stars_make();
    if (vp3d_scene_create(&scene) != VP_OK) return VP_ERR_NOMEM;
    vp3d_scene_set_contact(scene, 0.5f);
    vp3d_view_init(&view, 72);
    view.near_z = 1;
    view.far_z = 8000;
    vp3d_look_at(&view, vp_v3(0, EYE_Y, 0), vp_v3(0, EYE_Y + PITCH * 100, 100), vp_v3(0, 1, 0));
    return VP_OK;
}

static void update(vp_game *g, float dt)
{
    (void)g;
    G.time += dt;
    G.have = vp_saver_music(&G.m);
    if (G.have && G.m.track != G.last_track) {
        G.last_track = G.m.track;
        snprintf(G.title, sizeof G.title, "%s", G.m.title);
        G.title_time = G.title[0] ? 7 : 0;
    }
    G.title_time = fmaxf(0, G.title_time - dt);
    G.loud += ((G.have ? G.m.level : 0) - G.loud) * fminf(1, dt * 0.4f);   /* slowed right down */
    G.flow += dt * (0.35f + 1.0f * G.loud);               /* lines a second */
    G.sway += dt * (0.25f + 0.6f * G.loud);
}

/* The track's title over the sky, then gone. */
static void title(vp_dvg *d)
{
    if (G.title_time <= 0) return;
    float k = smooth01(G.title_time / 1.5f) * smooth01((7 - G.title_time) / 0.5f);
    vp_text_font(VP_FONT_ASTEROIDS);
    color(d, 255, 120, 230, k);
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
    vp3d_scene_begin(scene, d, &view);
    /* the ground: invisible, hiding the sky below the horizon */
    const float r = 9000;
    vp_vec3 ground[4] = { vp_v3(-r, 0, 1), vp_v3(r, 0, 1), vp_v3(r, 0, r), vp_v3(-r, 0, r) };
    vp3d_scene_polygon(scene, ground, 4, 0);
    sky_draw(d);
    range_draw(d);
    plain_draw(d);
    trail_draw(d);
    vp3d_scene_end(scene);
    title(d);
    shots(d);
}

int main(int argc, char **argv)
{
    vp_game_desc desc = {0};
    desc.title = "Neon Ridge";
    desc.init = init;
    desc.update = update;
    desc.draw = draw;
    return vp_run(&desc, argc, argv);
}
