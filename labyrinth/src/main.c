/* Labyrinth — a VectorPie screensaver: a walk through one maze after another.
 *
 * Seen from inside, at eye height: walls of lines, a grid on the floor and
 * the ceiling. The walker keeps a hand on the right-hand wall, which in
 * these mazes always leads to the way out: it walks a corridor, stops,
 * turns, walks on. In the mazes:
 *   - VECTORPIE plates on some of the walls;
 *   - spinning crystals: walking through one turns the world slowly upside
 *     down (and the next one back again);
 *   - the way out, a spinning pie with a slice missing: there the maze fades
 *     away and the next one, in other colors, fades in.
 *
 * To the music, smoothly: the walker goes faster when it is louder. Nothing
 * pulses, flashes or blinks. A new track shows its title.
 *
 * On the cabinet: installed in SETTINGS -> SCREENSAVERS (tools/deploy.sh or
 * the .vps). Any control wakes the cabinet, START 2 skips to the next track.
 * On a PC: saver_labyrinth <a .ogg/.wav or a folder of them>. Esc quits.
 * SAVER_SHOTS=<folder> saves a picture every 2 s; SAVER_TEMPO=<factor>
 * walks that many times faster (both for development).
 */
#include <vectorpie/vp.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define W        VP_DVG_COORD_MAX
#define CX       (W / 2.0f)
#define PI       3.14159265f
#define NMAX     16                     /* the largest maze, cells a side */
#define S        2.4f                   /* a cell, m */
#define H        2.2f                   /* the walls' height */
#define EYE      (H / 2)                /* (the middle: upside down it is the same) */
#define SEEN     9                      /* cells drawn around the walker */
#define CRYSTALS 3
#define WALK_MAX (4 * NMAX * NMAX + 4)

/* ---- the maze ---------------------------------------------------------- */

/* hw[y][x]: a wall along z = y*S from x to x+1; vw[y][x]: along x = x*S from y to y+1 */
static struct {
    int     n;
    uint8_t hw[NMAX + 1][NMAX + 1], vw[NMAX + 1][NMAX + 1];
    uint32_t salt;                      /* which walls carry a plate */
    int     style;                      /* the walls' decoration */
    int     pal;                        /* the colors */
    /* the walk: cells and the direction each was entered in */
    int     path[WALK_MAX][3];
    int     len;
    int     crystal[CRYSTALS][2];       /* cells */
    int     crystal_at[CRYSTALS];       /* where on the walk */
    float   crystal_gone[CRYSTALS];     /* 0 there, 1 gone (taken) */
    int     crystal_taken[CRYSTALS];
} M;

/* dir: 0 = +z, 1 = +x, 2 = -z, 3 = -x (heading = dir * 90 degrees, to the right) */
static const int DX[4] = { 0, 1, 0, -1 }, DZ[4] = { 1, 0, -1, 0 };

static uint32_t rng_state = 1;
static uint32_t rnd(void)
{
    uint32_t x = rng_state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return rng_state = x;
}
static float rndf(void) { return (rnd() & 0xffffff) / 16777216.0f; }

static int open_side(int x, int y, int d)
{
    switch (d) {
    case 0:  return !M.hw[y + 1][x];
    case 1:  return !M.vw[y][x + 1];
    case 2:  return !M.hw[y][x];
    default: return !M.vw[y][x];
    }
}

static void knock(int x, int y, int d)
{
    switch (d) {
    case 0:  M.hw[y + 1][x] = 0; break;
    case 1:  M.vw[y][x + 1] = 0; break;
    case 2:  M.hw[y][x] = 0; break;
    default: M.vw[y][x] = 0; break;
    }
}

/* A new maze: every cell reachable by exactly one way (a depth-first carve),
 * then the right-hand walk from the corner to the opposite one. */
static void maze_new(void)
{
    static uint8_t seen[NMAX][NMAX];
    static int stack[NMAX * NMAX][2];
    int n = M.n = 9 + (int)(rnd() % 8);
    for (int y = 0; y <= n; y++)
        for (int x = 0; x <= n; x++) {
            M.hw[y][x] = x < n;
            M.vw[y][x] = y < n;
        }
    for (int y = 0; y < n; y++)
        for (int x = 0; x < n; x++) seen[y][x] = 0;
    int sp = 0;
    stack[sp][0] = 0; stack[sp][1] = 0; sp++;
    seen[0][0] = 1;
    while (sp > 0) {
        int x = stack[sp - 1][0], y = stack[sp - 1][1], ways[4], k = 0;
        for (int d = 0; d < 4; d++) {
            int nx = x + DX[d], ny = y + DZ[d];
            if (nx >= 0 && ny >= 0 && nx < n && ny < n && !seen[ny][nx]) ways[k++] = d;
        }
        if (!k) { sp--; continue; }
        int d = ways[rnd() % k];
        knock(x, y, d);
        x += DX[d]; y += DZ[d];
        seen[y][x] = 1;
        stack[sp][0] = x; stack[sp][1] = y; sp++;
    }

    /* the walk: right hand on the wall (right, ahead, left, back) */
    int x = 0, y = 0, d = open_side(0, 0, 0) ? 0 : 1;
    M.len = 0;
    M.path[M.len][0] = x; M.path[M.len][1] = y; M.path[M.len][2] = d; M.len++;
    while ((x != n - 1 || y != n - 1) && M.len < WALK_MAX) {
        static const int TRY[4] = { 1, 0, 3, 2 };
        for (int t = 0; t < 4; t++) {
            int nd = (d + TRY[t]) & 3;
            if (open_side(x, y, nd)) { d = nd; break; }
        }
        x += DX[d]; y += DZ[d];
        M.path[M.len][0] = x; M.path[M.len][1] = y; M.path[M.len][2] = d; M.len++;
    }

    /* the crystals, on the way, well apart */
    for (int i = 0; i < CRYSTALS; i++) {
        int k = (int)((0.2f + 0.25f * i + 0.1f * rndf()) * M.len);
        /* on a cell's first visit: else the walk would pass through it before (its way in and out of a dead end) */
        for (int again = 1; again && k < M.len - 2; ) {
            again = 0;
            for (int j = 0; j < k && !again; j++)
                again = M.path[j][0] == M.path[k][0] && M.path[j][1] == M.path[k][1];
            if (again) k++;
        }
        M.crystal_at[i] = k;
        M.crystal[i][0] = M.path[k][0];
        M.crystal[i][1] = M.path[k][1];
        M.crystal_gone[i] = 0;
        M.crystal_taken[i] = 0;
    }
    M.salt = rnd();
    M.style = (int)(rnd() % 4);
    M.pal = (M.pal + 1 + (int)(rnd() % 4)) % 5;
}

/* ---- the colors: walls, floor, ceiling, crystals, the way out ------------ */

typedef struct { float r, g, b; } rgb;
static const rgb PAL[5][5] = {
    { { 0, 220, 255 },  { 200, 0, 255 },  { 60, 60, 200 },  { 255, 60, 200 }, { 255, 210, 0 } },
    { { 60, 255, 90 },  { 0, 140, 60 },   { 0, 90, 140 },   { 200, 255, 255 }, { 255, 160, 0 } },
    { { 255, 140, 0 },  { 255, 40, 80 },  { 120, 40, 160 }, { 0, 240, 255 },  { 255, 255, 120 } },
    { { 230, 230, 255 }, { 40, 120, 255 }, { 80, 80, 140 }, { 255, 80, 255 }, { 255, 200, 0 } },
    { { 255, 50, 160 }, { 0, 200, 255 },  { 90, 0, 160 },   { 255, 255, 255 }, { 120, 255, 60 } },
};

/* ---- the walker ---------------------------------------------------------- */

static struct {
    float u;                            /* how far along the walk, in cells */
    float heading;                      /* unwrapped */
    float roll, roll_from, roll_to, roll_t;
    float fade;                         /* 0 dark .. 1 seen */
    int   leaving;                      /* at the way out: fading away */
    float time, loud, tempo;
    vp_music_feed m;
    int   have;
    unsigned last_track;
    float title_time;
    char  title[VP_MUSIC_TEXT];
} G;

static vp3d_scene *scene;
static vp3d_mesh *crystal_mesh, *pie_mesh;
static vp3d_view view;

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
static float smooth01(float t) { t = clampf(t, 0, 1); return t * t * (3 - 2 * t); }

static void start_maze(void)
{
    maze_new();
    G.u = 0;
    G.heading = M.path[M.len > 1 ? 1 : 0][2] * PI / 2;
    G.leaving = 0;
}

/* Where the walk is at u (in cells along it): on the floor, and which way
 * it looks. It never stops: round each corner it walks a curve, turning as
 * it goes (from half a cell before the corner to half a cell after); at a
 * dead end it walks up, turns round to the right and walks back. bend: the
 * turn there, in radians (0 straight on, PI round). */
static void walk_at(float u, float *x, float *z, float *heading, float *bend)
{
    int k = (int)floorf(u + 0.5f);
    if (k < 0) k = 0;
    if (k > M.len - 1) k = M.len - 1;
    float t = clampf(u - k + 0.5f, 0, 1);
    int dout = M.path[k + 1 < M.len ? k + 1 : k][2];
    int din = k > 0 ? M.path[k][2] : dout;
    float turn = remainderf((dout - din) * PI / 2, 2 * PI);
    if (fabsf(fabsf(turn) - PI) < 0.01f) turn = PI;          /* round: always to the right */
    /* a curve from half a cell before the corner, A, past its middle C, to half after, B */
    float cx = M.path[k][0] + 0.5f, cz = M.path[k][1] + 0.5f;
    float ax = cx - DX[din] * 0.5f, az = cz - DZ[din] * 0.5f;
    float bx = cx + DX[dout] * 0.5f, bz = cz + DZ[dout] * 0.5f;
    float a = (1 - t) * (1 - t), c = 2 * t * (1 - t), b = t * t;
    *x = (a * ax + c * cx + b * bx) * S;
    *z = (a * az + c * cz + b * bz) * S;
    *heading = din * PI / 2 + turn * smooth01(t);
    *bend = fabsf(turn);
}

static void walker_pos(float *x, float *z)
{
    float h, bend;
    walk_at(G.u, x, z, &h, &bend);
}

static void walker_update(float dt)
{
    float pace = (2.0f + 1.2f * G.loud) * G.tempo;     /* cells a second, straight on */

    if (G.leaving) {                        /* at the way out: fade, then the next maze */
        G.fade -= dt / 1.5f;
        if (G.fade <= 0) { G.fade = 0; start_maze(); }
        return;
    }
    G.fade = fminf(1, G.fade + dt / 1.5f);

    float x, z, h, bend;
    walk_at(G.u, &x, &z, &h, &bend);
    G.u += dt * pace * (bend > 3 ? 0.4f : bend > 1 ? 0.75f : 1);   /* slower round corners */
    if (G.u >= M.len - 1.45f) {             /* before the way out: stop, fade away */
        G.u = M.len - 1.45f;
        G.leaving = 1;
    }
    walk_at(G.u, &x, &z, &h, &bend);
    G.heading += remainderf(h - G.heading, 2 * PI);

    /* walking through a crystal turns the world over. It goes from a cell and
     * a half before it and is gone half a cell before it, whatever the pace:
     * the eye never comes inside it (its edges would cut across the screen). */
    for (int i = 0; i < CRYSTALS; i++) {
        if (!M.crystal_taken[i] && G.u >= M.crystal_at[i] - 1.5f) {
            M.crystal_taken[i] = 1;
            G.roll_from = G.roll;
            G.roll_to = G.roll_to > PI / 2 ? 0 : PI;
            G.roll_t = 0;
        }
    }
}

static void world_update(float dt)
{
    for (int i = 0; i < CRYSTALS; i++)
        if (M.crystal_taken[i])
            M.crystal_gone[i] = fmaxf(fminf(1, M.crystal_gone[i] + dt / 0.3f),
                                      clampf(G.u - (M.crystal_at[i] - 1.5f), 0, 1));
    if (G.roll_t < 1) {
        G.roll_t = fminf(1, G.roll_t + dt / 3.0f);
        G.roll = G.roll_from + (G.roll_to - G.roll_from) * smooth01(G.roll_t);
    }
}

/* ---- the models -------------------------------------------------------- */

/* A crystal: an octahedron, taller than wide. */
static int make_crystal(void)
{
    if (vp3d_mesh_create(&crystal_mesh) != VP_OK) return VP_ERR_NOMEM;
    vp3d_mesh *m = crystal_mesh;
    vp3d_mesh_vertex(m, 0, 0.45f, 0);
    vp3d_mesh_vertex(m, 0.28f, 0, 0);
    vp3d_mesh_vertex(m, 0, 0, 0.28f);
    vp3d_mesh_vertex(m, -0.28f, 0, 0);
    vp3d_mesh_vertex(m, 0, 0, -0.28f);
    vp3d_mesh_vertex(m, 0, -0.45f, 0);
    for (int i = 0; i < 4; i++) {
        int a = 1 + i, b = 1 + (i + 1) % 4;
        int up[3] = { 0, b, a }, dn[3] = { 5, a, b };
        vp3d_mesh_face(m, up, 3);
        vp3d_mesh_face(m, dn, 3);
    }
    vp3d_mesh_set_crease(m, 0);
    return vp3d_mesh_finish(m);
}

/* The way out: a pie standing on its edge, a slice missing. */
static int make_pie(void)
{
    if (vp3d_mesh_create(&pie_mesh) != VP_OK) return VP_ERR_NOMEM;
    vp3d_mesh *m = pie_mesh;
    enum { SEG = 12 };
    const float r = 0.55f, t = 0.09f, gap = PI / 3;
    /* the rim from the gap round to the gap, and the center, front (z -t) and back */
    for (int s = 0; s <= SEG; s++) {
        float a = gap / 2 + (2 * PI - gap) * s / SEG;
        vp3d_mesh_vertex(m, r * sinf(a), r * cosf(a), -t);
        vp3d_mesh_vertex(m, r * sinf(a), r * cosf(a), t);
    }
    int c0 = vp3d_mesh_vertex(m, 0, 0, -t), c1 = vp3d_mesh_vertex(m, 0, 0, t);
    for (int s = 0; s < SEG; s++) {
        int f0 = 2 * s, b0 = 2 * s + 1, f1 = 2 * s + 2, b1 = 2 * s + 3;
        int front[3] = { c0, f1, f0 }, back[3] = { c1, b0, b1 }, rim[4] = { f0, f1, b1, b0 };
        vp3d_mesh_face(m, front, 3);
        vp3d_mesh_face(m, back, 3);
        vp3d_mesh_face(m, rim, 4);
    }
    int cut0[4] = { c0, 0, 1, c1 }, cut1[4] = { c0, c1, 2 * SEG + 1, 2 * SEG };
    vp3d_mesh_face(m, cut0, 4);
    vp3d_mesh_face(m, cut1, 4);
    return vp3d_mesh_finish(m);
}

/* ---- VECTORPIE (the SDK's text is 2D only: our own strokes) ------------- */

/* its letters, each 0.7 wide and 1 high, as strokes x0 y0 x1 y1 */
static const float LETTER_V[][4] = { { 0, 1, 0.35f, 0 }, { 0.35f, 0, 0.7f, 1 } };
static const float LETTER_E[][4] = { { 0.7f, 1, 0, 1 }, { 0, 1, 0, 0 }, { 0, 0, 0.7f, 0 }, { 0, 0.5f, 0.5f, 0.5f } };
static const float LETTER_C[][4] = { { 0.7f, 1, 0, 1 }, { 0, 1, 0, 0 }, { 0, 0, 0.7f, 0 } };
static const float LETTER_T[][4] = { { 0, 1, 0.7f, 1 }, { 0.35f, 1, 0.35f, 0 } };
static const float LETTER_O[][4] = { { 0, 0, 0.7f, 0 }, { 0.7f, 0, 0.7f, 1 }, { 0.7f, 1, 0, 1 }, { 0, 1, 0, 0 } };
static const float LETTER_R[][4] = { { 0, 0, 0, 1 }, { 0, 1, 0.7f, 1 }, { 0.7f, 1, 0.7f, 0.5f },
                                     { 0.7f, 0.5f, 0, 0.5f }, { 0.2f, 0.5f, 0.7f, 0 } };
static const float LETTER_P[][4] = { { 0, 0, 0, 1 }, { 0, 1, 0.7f, 1 }, { 0.7f, 1, 0.7f, 0.5f }, { 0.7f, 0.5f, 0, 0.5f } };
static const float LETTER_I[][4] = { { 0.35f, 0, 0.35f, 1 }, { 0.1f, 0, 0.6f, 0 }, { 0.1f, 1, 0.6f, 1 } };

/* VECTORPIE on a wall: from o, along u, up `up`, h high (8.7 h long) */
static void word(vp_vec3 o, vp_vec3 u, vp_vec3 up, float h)
{
    static const struct { const float (*s)[4]; int n; } WORD[9] = {
        { LETTER_V, 2 }, { LETTER_E, 4 }, { LETTER_C, 3 }, { LETTER_T, 2 }, { LETTER_O, 4 },
        { LETTER_R, 5 }, { LETTER_P, 4 }, { LETTER_I, 3 }, { LETTER_E, 4 },
    };
    for (int c = 0; c < 9; c++)
        for (int i = 0; i < WORD[c].n; i++) {
            const float *s = WORD[c].s[i];
            vp_vec3 a = vp_v3_add(o, vp_v3_add(vp_v3_scale(u, (s[0] + c) * h), vp_v3_scale(up, s[1] * h)));
            vp_vec3 b = vp_v3_add(o, vp_v3_add(vp_v3_scale(u, (s[2] + c) * h), vp_v3_scale(up, s[3] * h)));
            vp3d_scene_line(scene, a, b);
        }
}

/* ---- drawing ------------------------------------------------------------ */

static void color(vp_dvg *d, rgb c, float k)
{
    k = clampf(k * G.fade, 0, 1);
    vp_color(d, (uint8_t)(c.r * k), (uint8_t)(c.g * k), (uint8_t)(c.b * k));
}

static uint32_t hash(uint32_t a, uint32_t b)
{
    uint32_t i = a * 73856093u ^ b * 19349663u ^ M.salt;
    i *= 2654435761u;
    i ^= i >> 15;
    return i;
}

/* The walls' patterns, in the wall: u along it, v up, both 0..1 (seen from
 * both sides). Without them a wall right in front of the walker, its edges
 * out of view, would be nothing but black. */
static const float BRICKS[][4] = {
    { 0, .2f, 1, .2f }, { 0, .4f, 1, .4f }, { 0, .6f, 1, .6f }, { 0, .8f, 1, .8f },
    { .5f, 0, .5f, .2f }, { .25f, .2f, .25f, .4f }, { .75f, .2f, .75f, .4f }, { .5f, .4f, .5f, .6f },
    { .25f, .6f, .25f, .8f }, { .75f, .6f, .75f, .8f }, { .5f, .8f, .5f, 1 },
};
static const float PANEL[][4] = {
    { .1f, .1f, .9f, .1f }, { .9f, .1f, .9f, .9f }, { .9f, .9f, .1f, .9f }, { .1f, .9f, .1f, .1f },
    { .1f, .1f, .2f, .22f }, { .9f, .1f, .8f, .22f }, { .9f, .9f, .8f, .78f }, { .1f, .9f, .2f, .78f },
    { .2f, .22f, .8f, .22f }, { .8f, .22f, .8f, .78f }, { .8f, .78f, .2f, .78f }, { .2f, .78f, .2f, .22f },
};
static const float TILES[][4] = {
    { 0, 1 / 3.0f, 1, 1 / 3.0f }, { 0, 2 / 3.0f, 1, 2 / 3.0f }, { 1 / 3.0f, 0, 1 / 3.0f, 1 }, { 2 / 3.0f, 0, 2 / 3.0f, 1 },
};
static const float STRIPES[][4] = {
    { 0, .24f, 1, .24f }, { 0, .3f, 1, .3f }, { 0, .7f, 1, .7f }, { 0, .76f, 1, .76f },
    { .5f, .36f, .65f, .5f }, { .65f, .5f, .5f, .64f }, { .5f, .64f, .35f, .5f }, { .35f, .5f, .5f, .36f },
};
static const struct { const float (*s)[4]; int n; } PATTERN[4] = {
    { BRICKS, 11 }, { PANEL, 12 }, { TILES, 4 }, { STRIPES, 8 },
};

/* A wall from (x0,z0) to (x1,z1); ends: draw its upright edge at the start, at
 * the end; pattern: with its pattern (near, and no plate on it). */
static void wall(vp_dvg *d, float x0, float z0, float x1, float z1, int end0, int end1, int pattern)
{
    vp_vec3 p[4] = { vp_v3(x0, 0, z0), vp_v3(x1, 0, z1), vp_v3(x1, H, z1), vp_v3(x0, H, z0) };
    color(d, PAL[M.pal][0], 1);
    vp3d_scene_polygon(scene, p, 4, 1u | 4u | (end1 ? 2u : 0) | (end0 ? 8u : 0));
    if (!pattern) return;
    color(d, PAL[M.pal][0], 0.5f);
    for (int i = 0; i < PATTERN[M.style].n; i++) {
        const float *s = PATTERN[M.style].s[i];
        vp3d_scene_line(scene, vp_v3(x0 + (x1 - x0) * s[0], H * s[1], z0 + (z1 - z0) * s[0]),
                        vp_v3(x0 + (x1 - x0) * s[2], H * s[3], z0 + (z1 - z0) * s[2]));
    }
}

/* VECTORPIE on one side of a wall (side: the cell it faces, as a normal) */
static void plate(vp_dvg *d, float x0, float z0, float x1, float z1, float nx, float nz)
{
    const float off = 0.06f, h = 0.17f, len = 8.7f * h;
    float ux = (x1 - x0) / S, uz = (z1 - z0) / S;
    if (ux * nz - uz * nx > 0) {            /* read left to right from that side */
        float t = x0; x0 = x1; x1 = t;
        t = z0; z0 = z1; z1 = t;
        ux = -ux; uz = -uz;
    }
    float mx = (x0 + x1) / 2 + nx * off, mz = (z0 + z1) / 2 + nz * off;
    float hw = len / 2 + 0.12f, y0 = H * 0.52f, y1 = y0 + h + 0.24f;
    vp_vec3 c[4] = { vp_v3(mx - ux * hw, y0, mz - uz * hw), vp_v3(mx + ux * hw, y0, mz + uz * hw),
                     vp_v3(mx + ux * hw, y1, mz + uz * hw), vp_v3(mx - ux * hw, y1, mz - uz * hw) };
    color(d, PAL[M.pal][4], 0.7f);
    for (int i = 0; i < 4; i++) vp3d_scene_line(scene, c[i], c[(i + 1) & 3]);
    color(d, PAL[M.pal][4], 1);
    word(vp_v3(mx - ux * len / 2, y0 + 0.12f, mz - uz * len / 2), vp_v3(ux, 0, uz), vp_v3(0, 1, 0), h);
}

static void maze_draw(vp_dvg *d, float ex, float ez, vp_vec3 fwd)
{
    int n = M.n;
    int gx = (int)floorf(ex / S), gz = (int)floorf(ez / S);
    int xa = gx - SEEN < 0 ? 0 : gx - SEEN, xb = gx + SEEN > n ? n : gx + SEEN;
    int za = gz - SEEN < 0 ? 0 : gz - SEEN, zb = gz + SEEN > n ? n : gz + SEEN;
#define BEHIND(px, pz) (((px) - ex) * fwd.x + ((pz) - ez) * fwd.z < -1.5f)
#define NEAR(px, pz) (fabsf((px) - ex) + fabsf((pz) - ez) < 5.5f * S)   /* patterns farther are lost in the fog */

    /* walls along x (z = y*S) */
    for (int y = za; y <= zb; y++)
        for (int x = xa; x < xb; x++) {
            if (!M.hw[y][x]) continue;
            float x0 = x * S, x1 = x0 + S, z = y * S;
            if (BEHIND(x0, z) && BEHIND(x1, z)) continue;
            uint32_t h = hash((uint32_t)x, (uint32_t)y * 2);
            wall(d, x0, z, x1, z, x == 0 || !M.hw[y][x - 1], x + 1 >= n || !M.hw[y][x + 1],
                 h % 11 && NEAR(x0 + S / 2, z));
            if (h % 11 == 0) {
                int up = y == 0 ? 1 : y == n ? 0 : (int)(h >> 8 & 1);
                plate(d, x0, z, x1, z, 0, up ? 1.0f : -1.0f);
            }
        }
    /* walls along z (x = x*S): their upright edge only where no wall along x ends there */
    for (int y = za; y < zb; y++)
        for (int x = xa; x <= xb; x++) {
            if (!M.vw[y][x]) continue;
            float z0 = y * S, z1 = z0 + S, xx = x * S;
            if (BEHIND(xx, z0) && BEHIND(xx, z1)) continue;
            int l0 = x > 0 && M.hw[y][x - 1], r0 = x < n && M.hw[y][x];
            int l1 = x > 0 && M.hw[y + 1][x - 1], r1 = x < n && M.hw[y + 1][x];
            int e0 = (y == 0 || !M.vw[y - 1][x]) && !(l0 ^ r0);
            int e1 = (y + 1 >= n || !M.vw[y + 1][x]) && !(l1 ^ r1);
            uint32_t h = hash((uint32_t)x, (uint32_t)y * 2 + 1);
            wall(d, xx, z0, xx, z1, e0, e1, h % 11 && NEAR(xx, z0 + S / 2));
            if (h % 11 == 0) {
                int east = x == 0 ? 1 : x == n ? 0 : (int)(h >> 8 & 1);
                plate(d, xx, z0, xx, z1, east ? 1.0f : -1.0f, 0);
            }
        }

    /* the floor and the ceiling: a grid on the cells' edges */
    float fx0 = xa * S, fx1 = xb * S, fz0 = za * S, fz1 = zb * S;
    color(d, PAL[M.pal][1], 0.8f);
    for (int x = xa; x <= xb; x++) vp3d_scene_line(scene, vp_v3(x * S, 0, fz0), vp_v3(x * S, 0, fz1));
    for (int y = za; y <= zb; y++) vp3d_scene_line(scene, vp_v3(fx0, 0, y * S), vp_v3(fx1, 0, y * S));
    color(d, PAL[M.pal][2], 0.8f);
    for (int x = xa; x <= xb; x++) vp3d_scene_line(scene, vp_v3(x * S, H, fz0), vp_v3(x * S, H, fz1));
    for (int y = za; y <= zb; y++) vp3d_scene_line(scene, vp_v3(fx0, H, y * S), vp_v3(fx1, H, y * S));
#undef BEHIND
#undef NEAR

    /* the crystals, slowly turning */
    for (int i = 0; i < CRYSTALS; i++) {
        float k = 1 - M.crystal_gone[i];
        if (k <= 0) continue;
        vp_xf place = vp_xf_mul2(vp_xf_translate((M.crystal[i][0] + 0.5f) * S, EYE, (M.crystal[i][1] + 0.5f) * S),
                                 vp_xf_mul2(vp_xf_rot_y(G.time * 0.9f + i * 2.0f), vp_xf_scale(k, k, k)));
        color(d, PAL[M.pal][3], 1);
        vp3d_scene_mesh(scene, crystal_mesh, &place, VP3D_SOLID);
    }
    /* the way out */
    vp_xf place = vp_xf_mul2(vp_xf_translate((n - 0.5f) * S, EYE, (n - 0.5f) * S), vp_xf_rot_y(G.time * 0.7f));
    color(d, PAL[M.pal][4], 1);
    vp3d_scene_mesh(scene, pie_mesh, &place, VP3D_SOLID);
}

/* The track's title, at the top, then gone. */
static void title(vp_dvg *d)
{
    if (G.title_time <= 0) return;
    float k = smooth01(G.title_time / 1.5f) * smooth01((7 - G.title_time) / 0.5f);
    vp_text_font(VP_FONT_ASTEROIDS);
    vp_color(d, (uint8_t)(255 * k), (uint8_t)(255 * k), (uint8_t)(255 * k));
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

/* ---- the game ------------------------------------------------------------ */

static int init(vp_game *g)
{
    (void)g;
    rng_state = (uint32_t)vp_ticks_ms() * 2654435761u | 1;     /* other mazes each run */
    const char *tempo = getenv("SAVER_TEMPO");
    G.tempo = tempo ? clampf((float)atof(tempo), 0.1f, 50) : 1;
    if (make_crystal() != VP_OK || make_pie() != VP_OK) return VP_ERR_NOMEM;
    if (vp3d_scene_create(&scene) != VP_OK) return VP_ERR_NOMEM;
    vp3d_scene_set_contact(scene, 0.05f);   /* lines on a wall or the floor stay seen */
    vp3d_view_init(&view, 78);
    view.near_z = 0.05f;
    view.far_z = 60;
    view.fog_start = 2;
    view.fog_end = SEEN * S;
    view.fog_floor = 0.1f;
    M.pal = (int)(rnd() % 5);
    start_maze();
    return VP_OK;
}

static void update(vp_game *g, float dt)
{
    (void)g;
    G.time += dt;
    G.have = vp_saver_music(&G.m);
    if (G.have && G.m.track != G.last_track) {     /* a new track: its title */
        G.last_track = G.m.track;
        snprintf(G.title, sizeof G.title, "%s", G.m.title);
        G.title_time = G.title[0] ? 7 : 0;
    }
    G.title_time = fmaxf(0, G.title_time - dt);
    G.loud += ((G.have ? G.m.level : 0) - G.loud) * fminf(1, dt * 0.25f);   /* slowed right down */
    walker_update(dt);
    world_update(dt);
}

static void draw(vp_game *g, vp_dvg *d)
{
    (void)g;
    float x, z;
    walker_pos(&x, &z);
    vp_vec3 eye = vp_v3(x, EYE, z);
    vp_vec3 fwd = vp_v3(sinf(G.heading), 0, cosf(G.heading));
    vp_vec3 right = vp_v3(cosf(G.heading), 0, -sinf(G.heading));
    vp_vec3 up = vp_v3_add(vp_v3_scale(vp_v3(0, 1, 0), cosf(G.roll)), vp_v3_scale(right, sinf(G.roll)));
    vp3d_look_at(&view, eye, vp_v3_add(eye, fwd), up);
    vp3d_scene_begin(scene, d, &view);
    maze_draw(d, x, z, fwd);
    vp3d_scene_end(scene);
    title(d);
    shots(d);
}

int main(int argc, char **argv)
{
    vp_game_desc desc = {0};
    desc.title = "Labyrinth";
    desc.init = init;
    desc.update = update;
    desc.draw = draw;
    return vp_run(&desc, argc, argv);
}
