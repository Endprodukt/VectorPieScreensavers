/* models.c — the meshes: the car, the traffic, what stands by the road.
 *
 * All built in code from boxes (each may lean in at the top: a hood, a
 * cabin, a roof) and lone lines (palms, lattices, the pictures on the
 * billboards), in metres. A vehicle stands on the ground, its back at z 0,
 * its front at +z, its middle at x 0. A prop stands on the ground at its
 * own origin; things that reach over the road reach towards -x.
 *
 * Faces take the color current when the mesh is drawn, except where a
 * mesh's own faces are colored: the lights (red at the back, white at the
 * front), the tyres, the car's cabin.
 *
 * VECTORPIE is written in its own strokes (the SDK's text is 2D only): on
 * the trucks' trailers, on sign gantries, on the planes' banners.
 */
#include "saver.h"

#include <math.h>

static vp3d_mesh *car_mesh, *vehicles[T_COUNT], *props[P_COUNT];
static float lengths[T_COUNT], widths[T_COUNT];

/* the wheels of the car (index T_COUNT) and of each kind of vehicle, for
 * their spokes: the outer face's x, the axle's z, the radius */
#define MAXW 8
typedef struct { float x, z, r; } wheel_at;
static wheel_at wheel_list[T_COUNT + 1][MAXW];
static int nwheel[T_COUNT + 1];
static int building;                    /* whose wheels are being made */

static vp3d_mesh *cur;
static int nfaces;

static void begin(void)
{
    vp3d_mesh_create(&cur);
    nfaces = 0;
}

static vp3d_mesh *end(void)
{
    vp3d_mesh_finish(cur);
    return cur;
}

static int v(float x, float y, float z) { return vp3d_mesh_vertex(cur, x, y, z); }

/* A box from (x0, y0, z0) to (x1, y1, z1), its top drawn in by ix at the
 * sides, izf at the front (+z) and izb at the back. Returns its first face. */
static int box(float x0, float x1, float y0, float y1, float z0, float z1, float ix, float izf, float izb)
{
    int b0 = v(x0, y0, z0), b1 = v(x1, y0, z0), b2 = v(x1, y0, z1), b3 = v(x0, y0, z1);
    int t0 = v(x0 + ix, y1, z0 + izb), t1 = v(x1 - ix, y1, z0 + izb);
    int t2 = v(x1 - ix, y1, z1 - izf), t3 = v(x0 + ix, y1, z1 - izf);
    int f[6][4] = { { b0, b1, b2, b3 }, { t0, t3, t2, t1 }, { b3, b2, t2, t3 },
                    { b0, t0, t1, b1 }, { b1, t1, t2, b2 }, { b0, b3, t3, t0 } };
    for (int i = 0; i < 6; i++) vp3d_mesh_face(cur, f[i], 4);
    nfaces += 6;
    return nfaces - 6;
}

static int cube(float x0, float x1, float y0, float y1, float z0, float z1)
{
    return box(x0, x1, y0, y1, z0, z1, 0, 0, 0);
}

static void paint(int face, uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < 6; i++) vp3d_mesh_face_color(cur, face + i, r, g, b);
}

static void tail(int face) { paint(face, 255, 30, 50); }

static void head(int face) { paint(face, 255, 240, 200); }

static void lin(float x0, float y0, float z0, float x1, float y1, float z1)
{
    vp3d_mesh_line(cur, v(x0, y0, z0), v(x1, y1, z1));
}

/* ---- VECTORPIE ---------------------------------------------------------- */

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

int vectorpie_strokes(float (*seg)[4], int max)
{
    static const struct { const float (*s)[4]; int n; } word[9] = {
        { LETTER_V, 2 }, { LETTER_E, 4 }, { LETTER_C, 3 }, { LETTER_T, 2 }, { LETTER_O, 4 },
        { LETTER_R, 5 }, { LETTER_P, 4 }, { LETTER_I, 3 }, { LETTER_E, 4 },
    };
    int n = 0;
    for (int c = 0; c < 9; c++)
        for (int i = 0; i < word[c].n && n < max; i++, n++) {
            seg[n][0] = word[c].s[i][0] + c; seg[n][1] = word[c].s[i][1];
            seg[n][2] = word[c].s[i][2] + c; seg[n][3] = word[c].s[i][3];
        }
    return n;
}

/* VECTORPIE on a face: from o, along u, up `up`, h high */
static void word(vp_vec3 o, vp_vec3 u, vp_vec3 up, float h)
{
    float seg[40][4];
    int n = vectorpie_strokes(seg, 40);
    for (int i = 0; i < n; i++) {
        vp_vec3 a = vp_v3_add(o, vp_v3_add(vp_v3_scale(u, seg[i][0] * h), vp_v3_scale(up, seg[i][1] * h)));
        vp_vec3 b = vp_v3_add(o, vp_v3_add(vp_v3_scale(u, seg[i][2] * h), vp_v3_scale(up, seg[i][3] * h)));
        lin(a.x, a.y, a.z, b.x, b.y, b.z);
    }
}

/* A face of points p (indices idx), turned to face away from `inside`. */
static void face_out(int *idx, const vp_vec3 *p, int n, vp_vec3 inside)
{
    vp_vec3 nrm = vp_v3_cross(vp_v3_sub(p[1], p[0]), vp_v3_sub(p[2], p[0]));
    if (vp_v3_dot(nrm, vp_v3_sub(p[0], inside)) < 0)
        for (int i = 0; i < n / 2; i++) { int t = idx[i]; idx[i] = idx[n - 1 - i]; idx[n - 1 - i] = t; }
    vp3d_mesh_face(cur, idx, n);
    nfaces++;
}

/* A round wheel: a ring of SIDES faces, its axle across (x) from x0 to x1 at
 * height r and z zc. Its outer face (the one further from the middle) gets
 * the spokes, drawn turning (model_spokes). */
#define SIDES 10
static void wheel(float x0, float x1, float r, float zc)
{
    int a[SIDES], b[SIDES];
    vp_vec3 pa[SIDES], pb[SIDES];
    for (int i = 0; i < SIDES; i++) {
        float t = 2 * PI_F * i / SIDES, y = r + r * cosf(t), z = zc + r * sinf(t);
        pa[i] = vp_v3(x0, y, z); pb[i] = vp_v3(x1, y, z);
        a[i] = v(x0, y, z); b[i] = v(x1, y, z);
    }
    int first = nfaces;
    vp_vec3 mid = vp_v3((x0 + x1) / 2, r, zc);
    for (int i = 0; i < SIDES; i++) {
        int j = (i + 1) % SIDES;
        int q[4] = { a[i], a[j], b[j], b[i] };
        vp_vec3 qp[4] = { pa[i], pa[j], pb[j], pb[i] };
        face_out(q, qp, 4, mid);
    }
    face_out(a, pa, SIDES, mid);
    face_out(b, pb, SIDES, mid);
    for (int f = first; f < nfaces; f++) vp3d_mesh_face_color(cur, f, 110, 60, 170);
    if (nwheel[building] < MAXW) {
        wheel_at *w = &wheel_list[building][nwheel[building]++];
        w->x = fabsf(x0) > fabsf(x1) ? x0 : x1;
        w->z = zc;
        w->r = r;
    }
}

/* four wheels, at the sides from xi to xo, axles at za and zb */
static void wheels(float xi, float xo, float r, float za, float zb)
{
    float z[2] = { za, zb };
    for (int i = 0; i < 2; i++) {
        wheel(xi, xo, r, z[i]);
        wheel(-xi, -xo, r, z[i]);
    }
}

/* ---- the car ------------------------------------------------------------ */

static vp3d_mesh *make_car(void)
{
    begin();
    building = T_COUNT;
    wheels(0.70f, 0.98f, 0.33f, 0.85f, 3.65f);
    box(-0.98f, 0.98f, 0.25f, 0.78f, 0, 4.5f, 0.04f, 0.9f, 0.1f);
    paint(box(-0.75f, 0.75f, 0.78f, 1.16f, 1.0f, 3.0f, 0.14f, 0.75f, 0.45f), 0, 220, 255);
    cube(-0.97f, 0.97f, 1.02f, 1.08f, 0.05f, 0.42f);            /* the wing, on its posts */
    cube(-0.58f, -0.5f, 0.78f, 1.02f, 0.15f, 0.3f);
    cube(0.5f, 0.58f, 0.78f, 1.02f, 0.15f, 0.3f);
    tail(cube(-0.9f, 0.9f, 0.55f, 0.64f, -0.03f, 0));           /* the light bar */
    head(cube(-0.85f, -0.55f, 0.6f, 0.68f, 4.38f, 4.42f));
    head(cube(0.55f, 0.85f, 0.6f, 0.68f, 4.38f, 4.42f));
    for (int i = 0; i < 4; i++) {                               /* louvres over the rear window */
        float y = 0.84f + i * 0.08f, t = (y - 0.78f) / 0.38f;
        float z = 1.0f + t * 0.45f - 0.02f, w = (0.75f - 0.14f * t) * 0.85f;
        lin(-w, y, z, w, y, z);
    }
    return end();
}

/* ---- the traffic -------------------------------------------------------- */

static vp3d_mesh *make_vehicle(int t)
{
    begin();
    building = t;
    switch (t) {
    case T_SEDAN:
        wheels(0.62f, 0.9f, 0.32f, 0.9f, 3.7f);
        box(-0.9f, 0.9f, 0.28f, 0.85f, 0, 4.6f, 0.03f, 0.5f, 0.2f);
        box(-0.78f, 0.78f, 0.85f, 1.38f, 1.2f, 3.5f, 0.14f, 0.6f, 0.5f);
        tail(cube(-0.85f, -0.55f, 0.62f, 0.72f, -0.03f, 0));
        tail(cube(0.55f, 0.85f, 0.62f, 0.72f, -0.03f, 0));
        head(cube(-0.85f, -0.55f, 0.62f, 0.7f, 4.6f, 4.63f));
        head(cube(0.55f, 0.85f, 0.62f, 0.7f, 4.6f, 4.63f));
        lengths[t] = 4.6f; widths[t] = 1.8f;
        break;
    case T_SPORTS:
        wheels(0.66f, 0.95f, 0.31f, 0.85f, 3.45f);
        box(-0.95f, 0.95f, 0.22f, 0.72f, 0, 4.3f, 0.05f, 1.0f, 0.1f);
        box(-0.68f, 0.68f, 0.72f, 1.08f, 1.3f, 2.9f, 0.14f, 0.6f, 0.4f);
        cube(-0.95f, 0.95f, 0.95f, 1.0f, 0.05f, 0.35f);
        cube(-0.58f, -0.5f, 0.72f, 0.95f, 0.12f, 0.28f);
        cube(0.5f, 0.58f, 0.72f, 0.95f, 0.12f, 0.28f);
        tail(cube(-0.85f, 0.85f, 0.5f, 0.58f, -0.03f, 0));
        head(cube(-0.8f, -0.5f, 0.52f, 0.58f, 4.25f, 4.29f));
        head(cube(0.5f, 0.8f, 0.52f, 0.58f, 4.25f, 4.29f));
        lengths[t] = 4.3f; widths[t] = 1.9f;
        break;
    case T_TRUCK:
        cube(-1.25f, 1.25f, 1.1f, 3.9f, 0, 11.5f);              /* the trailer */
        cube(-1.0f, 1.0f, 0.6f, 1.1f, 0, 15.2f);                /* the chassis */
        box(-1.2f, 1.2f, 1.1f, 3.3f, 12.2f, 15.2f, 0.1f, 0.35f, 0);   /* the cab */
        for (int i = 0; i < 2; i++) {                           /* the trailer's axles, the cab's */
            float z = 1.3f + i * 1.15f;
            wheel(0.8f, 1.25f, 0.5f, z);
            wheel(-0.8f, -1.25f, 0.5f, z);
        }
        wheel(0.8f, 1.2f, 0.5f, 14.0f);
        wheel(-0.8f, -1.2f, 0.5f, 14.0f);
        tail(cube(-1.2f, -0.95f, 0.7f, 0.9f, -0.03f, 0));
        tail(cube(0.95f, 1.2f, 0.7f, 0.9f, -0.03f, 0));
        for (int i = -1; i <= 1; i++) paint(cube(i * 0.6f - 0.1f, i * 0.6f + 0.1f, 3.75f, 3.85f, -0.03f, 0), 255, 170, 40);
        head(cube(-1.1f, -0.75f, 1.3f, 1.5f, 15.2f, 15.23f));
        head(cube(0.75f, 1.1f, 1.3f, 1.5f, 15.2f, 15.23f));
        lin(0, 1.15f, -0.02f, 0, 3.85f, -0.02f);               /* its doors */
        word(vp_v3(1.27f, 1.9f, 1.0f), vp_v3(0, 0, 1), vp_v3(0, 1, 0), 1.1f);       /* its name, */
        word(vp_v3(-1.27f, 1.9f, 1.0f + WORD_W * 1.1f), vp_v3(0, 0, -1), vp_v3(0, 1, 0), 1.1f);   /* both sides */
        lengths[t] = 15.2f; widths[t] = 2.5f;
        break;
    case T_BIKE:
        wheel(-0.06f, 0.06f, 0.33f, 0.33f);
        wheel(-0.06f, 0.06f, 0.33f, 1.77f);
        box(-0.2f, 0.2f, 0.45f, 0.9f, 0.35f, 1.75f, 0.05f, 0.3f, 0.1f);
        box(-0.22f, 0.22f, 0.9f, 1.45f, 0.75f, 1.2f, 0.04f, 0, 0.1f);   /* the rider */
        cube(-0.14f, 0.14f, 1.45f, 1.75f, 0.85f, 1.15f);
        cube(-0.4f, 0.4f, 1.0f, 1.05f, 1.55f, 1.6f);                    /* the handlebar */
        tail(cube(-0.1f, 0.1f, 0.75f, 0.82f, 0.3f, 0.35f));
        head(cube(-0.1f, 0.1f, 0.85f, 0.95f, 1.8f, 1.85f));
        lengths[t] = 2.1f; widths[t] = 0.8f;
        break;
    case T_VAN:
        wheels(0.67f, 0.95f, 0.34f, 0.95f, 4.0f);
        box(-0.97f, 0.97f, 0.3f, 2.25f, 0, 4.1f, 0.04f, 0.25f, 0.05f);
        box(-0.95f, 0.95f, 0.3f, 1.15f, 4.1f, 5.0f, 0.05f, 0.4f, 0);
        tail(cube(-0.97f, -0.84f, 0.6f, 1.3f, -0.03f, 0));
        tail(cube(0.84f, 0.97f, 0.6f, 1.3f, -0.03f, 0));
        head(cube(-0.85f, -0.55f, 0.75f, 0.85f, 5.0f, 5.03f));
        head(cube(0.55f, 0.85f, 0.75f, 0.85f, 5.0f, 5.03f));
        for (int s = -1; s <= 1; s += 2) {                     /* the rear windows */
            float a = s * 0.1f, b = s * 0.8f;
            lin(a, 1.4f, -0.02f, b, 1.4f, -0.02f); lin(b, 1.4f, -0.02f, b, 2.0f, -0.02f);
            lin(b, 2.0f, -0.02f, a, 2.0f, -0.02f); lin(a, 2.0f, -0.02f, a, 1.4f, -0.02f);
        }
        lin(0, 0.35f, -0.02f, 0, 2.2f, -0.02f);
        lengths[t] = 5.0f; widths[t] = 1.95f;
        break;
    case T_PICKUP:
        wheels(0.67f, 0.95f, 0.36f, 1.0f, 4.2f);
        box(-0.97f, 0.97f, 0.4f, 1.05f, 0, 2.3f, 0, 0, 0);
        box(-0.95f, 0.95f, 0.4f, 1.85f, 2.3f, 4.1f, 0.08f, 0.35f, 0.05f);
        box(-0.95f, 0.95f, 0.4f, 1.15f, 4.1f, 5.3f, 0.03f, 0.3f, 0);
        tail(cube(-0.95f, -0.8f, 0.7f, 0.95f, -0.03f, 0));
        tail(cube(0.8f, 0.95f, 0.7f, 0.95f, -0.03f, 0));
        head(cube(-0.85f, -0.55f, 0.8f, 0.9f, 5.3f, 5.33f));
        head(cube(0.55f, 0.85f, 0.8f, 0.9f, 5.3f, 5.33f));
        lengths[t] = 5.3f; widths[t] = 1.95f;
        break;
    default:                                                    /* T_BUS */
        wheels(0.9f, 1.25f, 0.5f, 2.2f, 9.6f);
        box(-1.25f, 1.25f, 0.35f, 3.3f, 0, 12.0f, 0.05f, 0.1f, 0.05f);
        for (int s = -1; s <= 1; s += 2) {                     /* the window band */
            lin(s * 1.26f, 2.0f, 1.0f, s * 1.26f, 2.0f, 11.0f);
            lin(s * 1.26f, 2.9f, 1.0f, s * 1.26f, 2.9f, 11.0f);
        }
        lin(-1.0f, 2.1f, -0.02f, 1.0f, 2.1f, -0.02f); lin(-1.0f, 3.0f, -0.02f, 1.0f, 3.0f, -0.02f);
        tail(cube(-1.2f, -0.95f, 0.8f, 1.1f, -0.03f, 0));
        tail(cube(0.95f, 1.2f, 0.8f, 1.1f, -0.03f, 0));
        head(cube(-1.1f, -0.75f, 0.7f, 0.85f, 12.0f, 12.03f));
        head(cube(0.75f, 1.1f, 0.7f, 0.85f, 12.0f, 12.03f));
        lengths[t] = 12.0f; widths[t] = 2.5f;
        break;
    }
    return end();
}

/* ---- what stands by the road -------------------------------------------- */

static vp3d_mesh *make_prop(int k)
{
    begin();
    switch (k) {
    case P_PALM: {                                              /* the trunk, bending to the road */
        float px = 0, py = 0;
        for (int i = 1; i <= 6; i++) {
            float t = i / 6.0f, x = -1.4f * t * t, y = 7.0f * t;
            lin(px, py, 0, x, y, 0);
            px = x; py = y;
        }
        break;
    }
    case P_FRONDS:                                              /* its leaves, round the top */
        for (int j = 0; j < 8; j++) {
            float a = 2 * PI_F * j / 8 + 0.3f, cx = cosf(a), cz = sinf(a);
            float px = -1.4f, py = 7.0f, pz = 0;
            for (int q = 1; q <= 3; q++) {
                float r = 1.2f * q, x = -1.4f + cx * r, y = 7.0f + 0.9f * q - 0.45f * q * q, z = cz * r;
                lin(px, py, pz, x, y, z);
                px = x; py = y; pz = z;
            }
        }
        break;
    case P_LAMP:
        lin(0, 0, 0, 0, 8, 0);
        lin(0, 8, 0, -3.3f, 8.4f, 0);
        paint(cube(-3.9f, -2.9f, 8.1f, 8.35f, -0.3f, 0.3f), 255, 230, 140);
        break;
    case P_PYLON: {
        const float hs[] = { 0, 7, 14, 20, 24 }, ws[] = { 2.6f, 1.9f, 1.25f, 0.8f, 0.45f };
        for (int i = 0; i < 4; i++) {
            float y0 = hs[i], y1 = hs[i + 1], a = ws[i], b = ws[i + 1];
            for (int s = -1; s <= 1; s += 2) {
                lin(-a, y0, s * a, -b, y1, s * b);              /* the legs */
                lin(a, y0, s * a, b, y1, s * b);
                lin(-a, y0, s * a, b, y1, s * b);               /* bracing, front and back */
                lin(a, y0, s * a, -b, y1, s * b);
            }
            lin(-b, y1, -b, b, y1, -b); lin(-b, y1, b, b, y1, b);
            lin(-b, y1, -b, -b, y1, b); lin(b, y1, -b, b, y1, b);
        }
        lin(-7, 20, 0, 7, 20, 0); lin(-5, 16, 0, 5, 16, 0);     /* the arms, the insulators */
        for (int s = -1; s <= 1; s += 2) {
            lin(s * 7, 20, 0, s * 7, 19, 0); lin(s * 5, 16, 0, s * 5, 15, 0);
            lin(s * 0.8f, 20, 0, s * 7, 20.8f, 0);
        }
        break;
    }
    case P_BILLBOARD1: case P_BILLBOARD2: case P_BILLBOARD3: {
        lin(-2.5f, 0, 0, -2.5f, 4, 0); lin(2.5f, 0, 0, 2.5f, 4, 0);
        cube(-4, 4, 4, 8.5f, -0.15f, 0.15f);
        const float z = -0.17f, cx = 0, cy = 6.25f;
        if (k == P_BILLBOARD1) {                                /* a sunset */
            for (int i = 0; i < 10; i++) {
                float a0 = PI_F * i / 10, a1 = PI_F * (i + 1) / 10;
                lin(cx + cosf(a0) * 1.8f, cy - 0.9f + sinf(a0) * 1.8f, z, cx + cosf(a1) * 1.8f, cy - 0.9f + sinf(a1) * 1.8f, z);
            }
            for (int i = 0; i < 4; i++) {
                float y = cy - 0.9f + 0.3f + i * 0.35f, w = sqrtf(fmaxf(0, 3.24f - (y - cy + 0.9f) * (y - cy + 0.9f)));
                lin(cx - w, y, z, cx + w, y, z);
            }
            lin(-3.5f, cy - 0.9f, z, 3.5f, cy - 0.9f, z);
            word(vp_v3(-WORD_W * 0.55f / 2, 4.45f, z), vp_v3(1, 0, 0), vp_v3(0, 1, 0), 0.55f);
        } else if (k == P_BILLBOARD2) {                         /* mountains and a zigzag */
            lin(-3.5f, 4.6f, z, -1.5f, 7.6f, z); lin(-1.5f, 7.6f, z, 0, 5.8f, z);
            lin(0, 5.8f, z, 1.6f, 7.9f, z); lin(1.6f, 7.9f, z, 3.5f, 4.6f, z);
            for (int i = 0; i < 6; i++) lin(-3 + i, 5.0f + (i & 1) * 0.5f, z, -2 + i, 5.0f + ((i + 1) & 1) * 0.5f, z);
        } else {                                                /* VECTORPIE, big, under chevrons */
            for (int i = 0; i < 3; i++) {
                float x = -1.8f + i * 1.2f;
                lin(x, 8.0f, z, x + 0.6f, 7.4f, z); lin(x + 0.6f, 7.4f, z, x, 6.8f, z);
            }
            word(vp_v3(-WORD_W * 0.85f / 2, 4.9f, z), vp_v3(1, 0, 0), vp_v3(0, 1, 0), 0.85f);
        }
        break;
    }
    case P_CACTUS:
        box(-0.3f, 0.3f, 0, 4.2f, -0.3f, 0.3f, 0.08f, 0.08f, 0.08f);
        cube(0.3f, 1.1f, 1.6f, 1.9f, -0.2f, 0.2f);
        box(0.8f, 1.1f, 1.9f, 3.1f, -0.2f, 0.2f, 0.04f, 0.04f, 0.04f);
        cube(-1.0f, -0.3f, 2.3f, 2.6f, -0.2f, 0.2f);
        box(-1.0f, -0.7f, 2.6f, 3.5f, -0.2f, 0.2f, 0.04f, 0.04f, 0.04f);
        break;
    case P_ROCK:
        box(-2.2f, 2.0f, 0, 1.7f, -1.6f, 1.8f, 0.8f, 0.7f, 0.5f);
        box(1.2f, 3.2f, 0, 1.0f, -1.0f, 0.9f, 0.5f, 0.3f, 0.4f);
        break;
    case P_GANTRY: {
        float x = ROAD_HALF + 1.2f;
        cube(-x - 0.25f, -x + 0.25f, 0, 7, -0.25f, 0.25f);
        cube(x - 0.25f, x + 0.25f, 0, 7, -0.25f, 0.25f);
        cube(-x - 0.5f, x + 0.5f, 6.3f, 7.0f, -0.3f, 0.3f);
        for (int i = 0; i < 2; i++) {
            float a = -4.6f + i * 5.0f;
            cube(a, a + 4.0f, 4.3f, 6.3f, -0.4f, -0.3f);
            float m = a + 2.0f, z = -0.42f;                    /* an arrow down to a lane */
            lin(m, 5.9f, z, m, 4.8f, z); lin(m - 0.45f, 5.25f, z, m, 4.8f, z); lin(m + 0.45f, 5.25f, z, m, 4.8f, z);
        }
        break;
    }
    case P_GANTRY_VP: {                                         /* the same, with one wide sign */
        float x = ROAD_HALF + 1.2f;
        cube(-x - 0.25f, -x + 0.25f, 0, 7, -0.25f, 0.25f);
        cube(x - 0.25f, x + 0.25f, 0, 7, -0.25f, 0.25f);
        cube(-x - 0.5f, x + 0.5f, 6.3f, 7.0f, -0.3f, 0.3f);
        cube(-5.2f, 5.2f, 4.2f, 6.3f, -0.4f, -0.3f);
        word(vp_v3(-WORD_W / 2, 4.75f, -0.42f), vp_v3(1, 0, 0), vp_v3(0, 1, 0), 1.0f);
        break;
    }
    case P_ARCH: {                                              /* a neon gate over the road */
        float r = ROAD_HALF + 1.2f, legs = 3.0f;
        lin(-r, 0, 0, -r, legs, 0);
        lin(r, 0, 0, r, legs, 0);
        for (int i = 0; i < 12; i++) {
            float a0 = PI_F * i / 12, a1 = PI_F * (i + 1) / 12;
            lin(-cosf(a0) * r, legs + sinf(a0) * r * 0.55f, 0, -cosf(a1) * r, legs + sinf(a1) * r * 0.55f, 0);
        }
        break;
    }
    case P_BOX:
        cube(-0.5f, 0.5f, 0, 1, -0.5f, 0.5f);
        break;
    default:                                                    /* P_SIGN: VECTORPIE by the road */
        lin(-1.4f, 0, 0, -1.4f, 2.0f, 0);
        lin(1.4f, 0, 0, 1.4f, 2.0f, 0);
        cube(-1.9f, 1.9f, 2.0f, 3.2f, -0.05f, 0.05f);
        word(vp_v3(-WORD_W * 0.4f / 2, 2.4f, -0.07f), vp_v3(1, 0, 0), vp_v3(0, 1, 0), 0.4f);
        break;
    }
    return end();
}

int models_init(void)
{
    car_mesh = make_car();
    for (int t = 0; t < T_COUNT; t++) vehicles[t] = make_vehicle(t);
    for (int k = 0; k < P_COUNT; k++) props[k] = make_prop(k);
    return VP_OK;
}

vp3d_mesh *model_car(void) { return car_mesh; }
vp3d_mesh *model_vehicle(int type) { return vehicles[type]; }
float model_vehicle_length(int type) { return lengths[type]; }
float model_vehicle_width(int type) { return widths[type]; }
vp3d_mesh *model_prop(int kind) { return props[kind]; }

void model_spokes(vp3d_scene *sc, int type, const vp_xf *place, float turn)
{
    int who = type < 0 ? T_COUNT : type;
    for (int i = 0; i < nwheel[who]; i++) {
        const wheel_at *w = &wheel_list[who][i];
        float x = w->x + (w->x < 0 ? -0.02f : 0.02f), r = w->r * 0.85f;
        for (int k = 0; k < 3; k++) {                       /* three spokes, turning as it rolls */
            float t = turn + PI_F * k / 3, dy = cosf(t) * r, dz = sinf(t) * r;
            vp3d_scene_line(sc, vp_xf_point(place, vp_v3(x, w->r + dy, w->z + dz)),
                                vp_xf_point(place, vp_v3(x, w->r - dy, w->z - dz)));
        }
    }
}
