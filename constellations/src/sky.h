/* sky.h — the real sky: the stars of 26 constellations with their figures,
 * and some bright stars beside them.
 *
 * Positions J2000 (right ascension in hours, declination in degrees), visual
 * magnitudes, from the usual catalogues (rounded, good to a fraction of a
 * degree: plenty for the screen). The figures follow the common modern
 * drawings (as in star atlases and planetarium programs). Names are the
 * German ones, in capitals without umlauts (the fonts know only ASCII).
 */
#ifndef SKY_H
#define SKY_H

typedef struct {
    float ra, dec, mag;     /* hours, degrees, magnitude */
    char tint;              /* 'r' orange-red, 'y' yellowish, 'b' blue-white, 0 white */
    char shared;            /* 1: the same star as in another figure (drawn there) */
    const char *name;       /* shown when its figure is shown, or 0 */
} sky_star;

typedef struct {
    const char *name, *latin;
    int n;
    const sky_star *s;
    const signed char *path;    /* star indices, a line through them; -1 starts a new one */
    int npath;
} sky_con;

#define CON(var, name, latin) { name, latin, (int)(sizeof var##_s / sizeof var##_s[0]), var##_s, \
                                var##_p, (int)(sizeof var##_p) }

/* ---- the constellations ---------------------------------------------- */

static const sky_star uma_s[] = {
    { 11.062f, 61.751f, 1.79f, 'y', 0, "DUBHE" },  { 11.031f, 56.382f, 2.37f, 0, 0, "MERAK" },
    { 11.897f, 53.695f, 2.44f, 0, 0, 0 },          { 12.257f, 57.033f, 3.31f, 0, 0, 0 },
    { 12.900f, 55.960f, 1.77f, 0, 0, "ALIOTH" },   { 13.399f, 54.925f, 2.27f, 0, 0, "MIZAR" },
    { 13.792f, 49.313f, 1.86f, 'b', 0, "ALKAID" }, { 8.504f, 60.718f, 3.36f, 0, 0, 0 },
    { 9.849f, 59.039f, 3.80f, 0, 0, 0 },           { 9.548f, 51.678f, 3.17f, 0, 0, 0 },
    { 8.987f, 48.042f, 3.14f, 0, 0, 0 },           { 9.060f, 47.157f, 3.60f, 0, 0, 0 },
    { 11.767f, 47.779f, 3.71f, 0, 0, 0 },          { 11.161f, 44.498f, 3.01f, 0, 0, 0 },
    { 10.372f, 41.499f, 3.05f, 'r', 0, 0 },        { 10.285f, 42.915f, 3.45f, 0, 0, 0 },
    { 11.308f, 33.094f, 3.48f, 0, 0, 0 },          { 11.303f, 31.529f, 3.79f, 0, 0, 0 },
    { 9.525f, 63.062f, 3.65f, 0, 0, 0 },
};
static const signed char uma_p[] = { 6, 5, 4, 3, 0, 1, 2, 3, -1, 0, 18, 7, 8, 1, -1, 8, 9, 10, -1,
                                     2, 12, 13, 14, 15, -1, 12, 16, 17 };

static const sky_star umi_s[] = {
    { 2.530f, 89.264f, 1.98f, 'y', 0, "POLARSTERN" }, { 17.537f, 86.586f, 4.35f, 0, 0, 0 },
    { 16.766f, 82.037f, 4.21f, 0, 0, 0 },             { 15.734f, 77.795f, 4.29f, 0, 0, 0 },
    { 14.845f, 74.156f, 2.07f, 'r', 0, "KOCHAB" },    { 15.345f, 71.834f, 3.00f, 0, 0, 0 },
    { 16.292f, 75.755f, 4.95f, 0, 0, 0 },
};
static const signed char umi_p[] = { 0, 1, 2, 3, 4, 5, 6, 3 };

static const sky_star dra_s[] = {
    { 17.943f, 51.489f, 2.24f, 'r', 0, "ETAMIN" }, { 17.507f, 52.301f, 2.79f, 'y', 0, "RASTABAN" },
    { 17.536f, 55.173f, 4.88f, 0, 0, 0 },          { 17.892f, 56.873f, 3.75f, 0, 0, 0 },
    { 19.209f, 67.662f, 3.07f, 0, 0, 0 },          { 19.803f, 70.268f, 3.83f, 0, 0, 0 },
    { 18.351f, 72.733f, 3.55f, 0, 0, 0 },          { 17.146f, 65.715f, 3.17f, 'b', 0, 0 },
    { 16.400f, 61.514f, 2.73f, 'y', 0, 0 },        { 16.031f, 58.565f, 4.01f, 0, 0, 0 },
    { 15.416f, 58.966f, 3.29f, 'r', 0, 0 },        { 14.073f, 64.376f, 3.65f, 0, 0, "THUBAN" },
    { 12.558f, 69.788f, 3.87f, 0, 0, 0 },          { 11.523f, 69.331f, 3.84f, 0, 0, 0 },
};
static const signed char dra_p[] = { 0, 1, 2, 3, 0, -1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };

static const sky_star cep_s[] = {
    { 21.310f, 62.585f, 2.45f, 0, 0, "ALDERAMIN" }, { 21.478f, 70.561f, 3.23f, 'b', 0, 0 },
    { 23.656f, 77.632f, 3.21f, 'y', 0, 0 },         { 22.828f, 66.200f, 3.52f, 0, 0, 0 },
    { 22.181f, 58.201f, 3.35f, 'y', 0, 0 },         { 22.486f, 58.415f, 4.07f, 'y', 0, 0 },
    { 22.250f, 57.044f, 4.19f, 0, 0, 0 },
};
static const signed char cep_p[] = { 0, 1, 2, 3, 4, 0, -1, 1, 3, -1, 5, 4, 6 };

static const sky_star cas_s[] = {
    { 0.153f, 59.150f, 2.27f, 0, 0, "CAPH" },   { 0.675f, 56.537f, 2.24f, 'y', 0, "SCHEDIR" },
    { 0.945f, 60.717f, 2.15f, 'b', 0, 0 },      { 1.430f, 60.235f, 2.66f, 0, 0, "RUCHBAH" },
    { 1.907f, 63.670f, 3.35f, 'b', 0, 0 },
};
static const signed char cas_p[] = { 0, 1, 2, 3, 4 };

static const sky_star and_s[] = {
    { 0.140f, 29.091f, 2.06f, 'b', 0, "ALPHERATZ" }, { 0.655f, 30.861f, 3.27f, 'y', 0, 0 },
    { 1.162f, 35.621f, 2.05f, 'r', 0, "MIRACH" },    { 2.065f, 42.330f, 2.10f, 'y', 0, "ALMACH" },
    { 0.946f, 38.499f, 3.87f, 0, 0, 0 },             { 0.830f, 41.079f, 4.53f, 0, 0, 0 },
    { 0.614f, 33.719f, 4.36f, 'b', 0, 0 },           { 0.643f, 29.312f, 4.37f, 0, 0, 0 },
};
static const signed char and_p[] = { 0, 1, 2, 3, -1, 2, 4, 5 };

static const sky_star peg_s[] = {
    { 23.079f, 15.205f, 2.48f, 0, 0, "MARKAB" },  { 23.063f, 28.083f, 2.42f, 'r', 0, "SCHEAT" },
    { 0.220f, 15.184f, 2.83f, 'b', 0, "ALGENIB" }, { 0.140f, 29.091f, 2.06f, 'b', 1, 0 },
    { 21.736f, 9.875f, 2.39f, 'y', 0, "ENIF" },   { 22.170f, 6.198f, 3.53f, 0, 0, 0 },
    { 22.691f, 10.831f, 3.40f, 0, 0, 0 },         { 22.717f, 30.221f, 2.94f, 'y', 0, 0 },
    { 22.834f, 24.602f, 3.48f, 0, 0, 0 },         { 22.775f, 23.566f, 3.95f, 0, 0, 0 },
    { 22.117f, 25.345f, 3.76f, 0, 0, 0 },         { 21.744f, 25.645f, 4.13f, 0, 0, 0 },
};
static const signed char peg_p[] = { 0, 1, 3, 2, 0, 6, 5, 4, -1, 1, 7, -1, 1, 8, 9, 10, 11 };

static const sky_star ari_s[] = {
    { 2.120f, 23.463f, 2.00f, 'y', 0, "HAMAL" }, { 1.911f, 20.808f, 2.64f, 0, 0, "SHERATAN" },
    { 1.892f, 19.294f, 3.88f, 0, 0, 0 },         { 2.833f, 27.260f, 3.61f, 'b', 0, 0 },
};
static const signed char ari_p[] = { 3, 0, 1, 2 };

static const sky_star per_s[] = {
    { 3.405f, 49.861f, 1.79f, 0, 0, "MIRFAK" }, { 3.136f, 40.956f, 2.09f, 'b', 0, "ALGOL" },
    { 3.080f, 53.506f, 2.93f, 'y', 0, 0 },      { 3.715f, 47.788f, 3.01f, 'b', 0, 0 },
    { 3.964f, 40.010f, 2.89f, 'b', 0, 0 },      { 3.902f, 31.884f, 2.85f, 'b', 0, 0 },
    { 2.845f, 55.896f, 3.76f, 'r', 0, 0 },      { 3.158f, 44.857f, 3.80f, 0, 0, 0 },
    { 3.086f, 38.840f, 3.39f, 'r', 0, 0 },      { 3.753f, 42.578f, 3.77f, 0, 0, 0 },
    { 3.738f, 32.288f, 3.83f, 'b', 0, 0 },      { 3.983f, 35.791f, 4.04f, 'b', 0, 0 },
};
static const signed char per_p[] = { 6, 2, 0, 3, 9, 4, 11, 5, 10, -1, 0, 7, 1, 8 };

static const sky_star aur_s[] = {
    { 5.278f, 45.998f, 0.08f, 'y', 0, "KAPELLA" }, { 5.992f, 44.948f, 1.90f, 0, 0, "MENKALINAN" },
    { 5.995f, 37.213f, 2.62f, 0, 0, 0 },           { 5.438f, 28.608f, 1.65f, 'b', 1, 0 },
    { 4.950f, 33.166f, 2.69f, 'r', 0, 0 },         { 5.033f, 43.823f, 2.99f, 'y', 0, 0 },
    { 5.041f, 41.076f, 3.75f, 'r', 0, 0 },         { 5.109f, 41.234f, 3.17f, 'b', 0, 0 },
};
static const signed char aur_p[] = { 0, 1, 2, 3, 4, 0, -1, 0, 5, 6, 7 };

static const sky_star tau_s[] = {
    { 4.599f, 16.509f, 0.85f, 'r', 0, "ALDEBARAN" }, { 5.438f, 28.608f, 1.65f, 'b', 0, "ELNATH" },
    { 5.627f, 21.143f, 3.00f, 'b', 0, 0 },           { 4.478f, 15.871f, 3.40f, 0, 0, 0 },
    { 4.330f, 15.628f, 3.65f, 'y', 0, 0 },           { 4.383f, 17.543f, 3.76f, 'y', 0, 0 },
    { 4.477f, 19.180f, 3.53f, 'y', 0, 0 },           { 4.011f, 12.490f, 3.47f, 'b', 0, 0 },
    { 3.414f, 9.029f, 3.60f, 'y', 0, 0 },            { 3.453f, 9.733f, 3.73f, 0, 0, 0 },
    { 3.791f, 24.105f, 2.87f, 'b', 0, "PLEJADEN" },  { 3.819f, 24.053f, 3.62f, 'b', 0, 0 },
    { 3.748f, 24.113f, 3.70f, 'b', 0, 0 },           { 3.763f, 24.367f, 3.87f, 'b', 0, 0 },
    { 3.772f, 23.948f, 4.18f, 'b', 0, 0 },           { 3.753f, 24.467f, 4.30f, 'b', 0, 0 },
};
static const signed char tau_p[] = { 1, 6, 5, 4, 3, 0, 2, -1, 4, 7, 8 };

static const sky_star gem_s[] = {
    { 7.577f, 31.888f, 1.58f, 0, 0, "KASTOR" },  { 7.755f, 28.026f, 1.14f, 'y', 0, "POLLUX" },
    { 6.629f, 16.399f, 1.93f, 0, 0, "ALHENA" },  { 6.732f, 25.131f, 2.98f, 'y', 0, 0 },
    { 6.383f, 22.514f, 2.87f, 'r', 0, 0 },       { 6.248f, 22.507f, 3.28f, 'r', 0, 0 },
    { 7.335f, 21.982f, 3.53f, 0, 0, 0 },         { 7.068f, 20.570f, 3.79f, 'y', 0, 0 },
    { 6.755f, 12.896f, 3.35f, 0, 0, 0 },         { 7.186f, 30.245f, 4.41f, 'y', 0, 0 },
    { 7.741f, 24.398f, 3.57f, 'y', 0, 0 },       { 7.599f, 26.896f, 4.06f, 'r', 0, 0 },
    { 7.301f, 16.540f, 3.58f, 0, 0, 0 },         { 6.483f, 20.212f, 4.13f, 'b', 0, 0 },
};
static const signed char gem_p[] = { 0, 9, 3, 4, 5, -1, 1, 11, 6, 7, 2, -1, 11, 10, -1, 6, 12, 8, -1, 9, 11 };

static const sky_star cmi_s[] = {
    { 7.655f, 5.225f, 0.34f, 0, 0, "PROKYON" }, { 7.453f, 8.289f, 2.89f, 'b', 0, "GOMEISA" },
};
static const signed char cmi_p[] = { 0, 1 };

static const sky_star cma_s[] = {
    { 6.752f, -16.716f, -1.46f, 'b', 0, "SIRIUS" }, { 6.378f, -17.956f, 1.98f, 'b', 0, "MIRZAM" },
    { 6.977f, -28.972f, 1.50f, 'b', 0, "ADHARA" },  { 7.140f, -26.393f, 1.83f, 0, 0, "WEZEN" },
    { 7.402f, -29.303f, 2.45f, 'b', 0, 0 },         { 6.338f, -30.063f, 3.02f, 'b', 0, 0 },
    { 7.050f, -23.833f, 3.02f, 'b', 0, 0 },         { 7.063f, -15.633f, 4.12f, 'b', 0, 0 },
    { 6.936f, -17.054f, 4.37f, 'b', 0, 0 },         { 6.903f, -12.039f, 4.07f, 'y', 0, 0 },
};
static const signed char cma_p[] = { 1, 0, 6, 3, 4, -1, 3, 2, 5, -1, 0, 8, 7, 9 };

static const sky_star ori_s[] = {
    { 5.919f, 7.407f, 0.50f, 'r', 0, "BETEIGEUZE" }, { 5.242f, -8.202f, 0.13f, 'b', 0, "RIGEL" },
    { 5.419f, 6.350f, 1.64f, 'b', 0, "BELLATRIX" },  { 5.533f, -0.299f, 2.23f, 'b', 0, 0 },
    { 5.604f, -1.202f, 1.69f, 'b', 0, 0 },           { 5.679f, -1.943f, 1.77f, 'b', 0, 0 },
    { 5.796f, -9.670f, 2.06f, 'b', 0, "SAIPH" },     { 5.585f, 9.934f, 3.39f, 'b', 0, 0 },
    { 4.830f, 6.961f, 3.19f, 0, 0, 0 },              { 4.843f, 8.900f, 4.35f, 0, 0, 0 },
    { 4.853f, 5.605f, 3.68f, 'b', 0, 0 },            { 4.904f, 2.441f, 3.71f, 'b', 0, 0 },
    { 4.976f, 1.714f, 4.47f, 'r', 0, 0 },            { 6.040f, 9.648f, 4.12f, 0, 0, 0 },
    { 6.200f, 14.208f, 4.45f, 0, 0, 0 },             { 6.126f, 14.768f, 4.42f, 'b', 0, 0 },
    { 5.906f, 20.276f, 4.39f, 'y', 0, 0 },           { 6.064f, 20.138f, 4.63f, 'b', 0, 0 },
    { 4.907f, 10.151f, 4.65f, 0, 0, 0 },
};
static const signed char ori_p[] = { 7, 0, 5, 4, 3, 2, 7, -1, 5, 6, -1, 3, 1, -1, 2, 8, 9, 18, -1, 8, 10, 11, 12,
                                     -1, 0, 13, 14, 17, -1, 13, 15, 16 };

static const sky_star lyr_s[] = {
    { 18.616f, 38.784f, 0.03f, 'b', 0, "WEGA" }, { 18.739f, 39.670f, 4.67f, 0, 0, 0 },
    { 18.746f, 37.605f, 4.36f, 0, 0, 0 },        { 18.908f, 36.899f, 4.30f, 'r', 0, 0 },
    { 18.982f, 32.690f, 3.24f, 'b', 0, "SULAFAT" }, { 18.835f, 33.363f, 3.52f, 'b', 0, "SHELIAK" },
};
static const signed char lyr_p[] = { 1, 0, 2, 3, 4, 5, 2 };

static const sky_star cyg_s[] = {
    { 20.690f, 45.280f, 1.25f, 0, 0, "DENEB" },   { 20.370f, 40.257f, 2.23f, 'y', 0, "SADR" },
    { 19.512f, 27.960f, 3.05f, 'y', 0, "ALBIREO" }, { 20.770f, 33.970f, 2.48f, 'y', 0, 0 },
    { 19.750f, 45.131f, 2.87f, 'b', 0, 0 },       { 19.938f, 35.083f, 3.89f, 'y', 0, 0 },
    { 21.216f, 30.227f, 3.21f, 'y', 0, 0 },       { 19.495f, 51.730f, 3.79f, 0, 0, 0 },
    { 19.285f, 53.368f, 3.80f, 'y', 0, 0 },
};
static const signed char cyg_p[] = { 0, 1, 5, 2, -1, 4, 1, 3, 6, -1, 4, 7, 8 };

static const sky_star aql_s[] = {
    { 19.846f, 8.868f, 0.77f, 0, 0, "ATAIR" },  { 19.771f, 10.613f, 2.72f, 'r', 0, "TARAZED" },
    { 19.922f, 6.407f, 3.71f, 'y', 0, 0 },      { 19.425f, 3.115f, 3.36f, 0, 0, 0 },
    { 19.090f, 13.863f, 2.99f, 0, 0, 0 },       { 20.188f, -0.821f, 3.23f, 'b', 0, 0 },
    { 19.104f, -4.882f, 3.43f, 'b', 0, 0 },     { 19.874f, 1.006f, 3.87f, 'y', 0, 0 },
};
static const signed char aql_p[] = { 4, 1, 0, 2, 7, 5, -1, 0, 3, 6 };

static const sky_star del_s[] = {
    { 20.553f, 11.303f, 4.03f, 'y', 0, 0 }, { 20.626f, 14.595f, 3.63f, 0, 0, 0 },
    { 20.661f, 15.912f, 3.77f, 'b', 0, 0 }, { 20.777f, 16.124f, 3.87f, 'y', 0, 0 },
    { 20.724f, 15.068f, 4.43f, 0, 0, 0 },
};
static const signed char del_p[] = { 0, 1, 2, 3, 4, 1 };

static const sky_star her_s[] = {
    { 16.688f, 31.603f, 2.81f, 0, 0, 0 },       { 16.715f, 38.922f, 3.48f, 'y', 0, 0 },
    { 17.251f, 36.809f, 3.16f, 'r', 0, 0 },     { 17.005f, 30.926f, 3.92f, 'b', 0, 0 },
    { 16.504f, 21.490f, 2.78f, 'y', 0, "KORNEPHOROS" }, { 17.250f, 24.839f, 3.14f, 0, 0, 0 },
    { 17.244f, 14.390f, 3.10f, 'r', 0, "RAS ALGETHI" }, { 17.774f, 27.720f, 3.42f, 'y', 0, 0 },
    { 17.938f, 37.250f, 3.86f, 0, 0, 0 },       { 17.657f, 46.006f, 3.80f, 'b', 0, 0 },
    { 16.568f, 42.437f, 4.20f, 0, 0, 0 },       { 16.330f, 46.313f, 3.89f, 'b', 0, 0 },
};
static const signed char her_p[] = { 0, 1, 2, 3, 0, 4, -1, 3, 5, 6, -1, 5, 7, -1, 2, 8, 9, -1, 1, 10, 11 };

static const sky_star crb_s[] = {
    { 15.549f, 31.359f, 4.14f, 0, 0, 0 },          { 15.464f, 29.106f, 3.66f, 0, 0, 0 },
    { 15.578f, 26.715f, 2.23f, 0, 0, "GEMMA" },    { 15.712f, 26.296f, 3.81f, 0, 0, 0 },
    { 15.826f, 26.068f, 4.59f, 'y', 0, 0 },        { 15.960f, 26.878f, 4.14f, 'y', 0, 0 },
    { 16.024f, 29.851f, 4.98f, 0, 0, 0 },
};
static const signed char crb_p[] = { 0, 1, 2, 3, 4, 5, 6 };

static const sky_star boo_s[] = {
    { 14.261f, 19.182f, -0.05f, 'r', 0, "ARKTUR" }, { 14.750f, 27.074f, 2.35f, 'y', 0, "IZAR" },
    { 15.258f, 33.315f, 3.47f, 'y', 0, 0 },         { 15.032f, 40.390f, 3.49f, 'y', 0, 0 },
    { 14.535f, 38.308f, 3.03f, 0, 0, 0 },           { 14.531f, 30.371f, 3.58f, 'y', 0, 0 },
    { 13.911f, 18.398f, 2.68f, 'y', 0, "MUPHRID" }, { 14.686f, 13.728f, 3.78f, 0, 0, 0 },
};
static const signed char boo_p[] = { 0, 1, 2, 3, 4, 5, 0, -1, 0, 6, -1, 0, 7 };

static const sky_star vir_s[] = {
    { 13.420f, -11.161f, 0.98f, 'b', 0, "SPICA" }, { 12.694f, -1.449f, 2.74f, 0, 0, "PORRIMA" },
    { 13.036f, 10.959f, 2.85f, 'y', 0, "VINDEMIATRIX" }, { 12.927f, 3.397f, 3.38f, 'r', 0, 0 },
    { 13.578f, -0.596f, 3.37f, 0, 0, 0 },          { 11.845f, 1.765f, 3.61f, 0, 0, 0 },
    { 12.332f, -0.667f, 3.89f, 0, 0, 0 },          { 13.166f, -5.539f, 4.38f, 0, 0, 0 },
    { 14.267f, -6.000f, 4.08f, 0, 0, 0 },          { 14.718f, -5.658f, 3.88f, 0, 0, 0 },
};
static const signed char vir_p[] = { 5, 6, 1, 3, 2, -1, 1, 7, 0, 4, 3, -1, 4, 8, 9 };

static const sky_star leo_s[] = {
    { 10.140f, 11.967f, 1.35f, 'b', 0, "REGULUS" }, { 10.122f, 16.763f, 3.48f, 0, 0, 0 },
    { 10.333f, 19.842f, 2.01f, 'y', 0, "ALGIEBA" }, { 10.278f, 23.417f, 3.43f, 0, 0, 0 },
    { 9.879f, 26.007f, 3.88f, 'y', 0, 0 },          { 9.764f, 23.774f, 2.98f, 'y', 0, 0 },
    { 11.818f, 14.572f, 2.14f, 0, 0, "DENEBOLA" },  { 11.235f, 20.524f, 2.56f, 0, 0, "ZOSMA" },
    { 11.237f, 15.430f, 3.33f, 0, 0, 0 },
};
static const signed char leo_p[] = { 0, 1, 2, 3, 4, 5, -1, 2, 7, 6, 8, 0, -1, 7, 8 };

static const sky_star sco_s[] = {
    { 16.490f, -26.432f, 1.06f, 'r', 0, "ANTARES" }, { 16.353f, -25.593f, 2.89f, 'b', 0, 0 },
    { 16.006f, -22.622f, 2.29f, 'b', 0, "DSCHUBBA" }, { 16.091f, -19.806f, 2.62f, 'b', 0, "ACRAB" },
    { 15.981f, -26.114f, 2.89f, 'b', 0, 0 },         { 16.598f, -28.216f, 2.82f, 'b', 0, 0 },
    { 16.836f, -34.293f, 2.29f, 'y', 0, 0 },         { 16.864f, -38.048f, 3.00f, 'b', 0, 0 },
    { 16.910f, -42.362f, 3.62f, 'y', 0, 0 },         { 17.203f, -43.239f, 3.32f, 0, 0, 0 },
    { 17.622f, -42.998f, 1.86f, 0, 0, "SARGAS" },    { 17.793f, -40.127f, 2.99f, 'y', 0, 0 },
    { 17.708f, -39.030f, 2.39f, 'b', 0, 0 },         { 17.560f, -37.104f, 1.62f, 'b', 0, "SHAULA" },
    { 17.513f, -37.296f, 2.69f, 'b', 0, 0 },
};
static const signed char sco_p[] = { 3, 2, 4, -1, 2, 1, 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };

static const sky_star sgr_s[] = {
    { 18.403f, -34.384f, 1.85f, 'b', 0, "KAUS AUSTRALIS" }, { 18.350f, -29.828f, 2.70f, 'y', 0, 0 },
    { 18.466f, -25.422f, 2.81f, 'y', 0, 0 },     { 18.097f, -30.424f, 2.98f, 'y', 0, 0 },
    { 18.761f, -26.991f, 3.17f, 'b', 0, 0 },     { 18.921f, -26.297f, 2.05f, 'b', 0, "NUNKI" },
    { 19.116f, -27.671f, 3.32f, 'y', 0, 0 },     { 19.044f, -29.880f, 2.60f, 0, 0, "ASCELLA" },
};
static const signed char sgr_p[] = { 3, 1, 2, 4, 1, 0, 3, -1, 0, 7, 4, 5, 6, 7 };

/* the tour, one figure after the next, each near the last */
static const sky_con SKY_CON[] = {
    CON(uma, "GROSSER BAER", "URSA MAIOR"),
    CON(umi, "KLEINER BAER", "URSA MINOR"),
    CON(dra, "DRACHE", "DRACO"),
    CON(her, "HERKULES", "HERCULES"),
    CON(crb, "NOERDLICHE KRONE", "CORONA BOREALIS"),
    CON(boo, "BAERENHUETER", "BOOTES"),
    CON(vir, "JUNGFRAU", "VIRGO"),
    CON(sco, "SKORPION", "SCORPIUS"),
    CON(sgr, "SCHUETZE", "SAGITTARIUS"),
    CON(aql, "ADLER", "AQUILA"),
    CON(del, "DELFIN", "DELPHINUS"),
    CON(lyr, "LEIER", "LYRA"),
    CON(cyg, "SCHWAN", "CYGNUS"),
    CON(cep, "KEPHEUS", "CEPHEUS"),
    CON(cas, "KASSIOPEIA", "CASSIOPEIA"),
    CON(and, "ANDROMEDA", "ANDROMEDA"),
    CON(peg, "PEGASUS", "PEGASUS"),
    CON(ari, "WIDDER", "ARIES"),
    CON(per, "PERSEUS", "PERSEUS"),
    CON(aur, "FUHRMANN", "AURIGA"),
    CON(tau, "STIER", "TAURUS"),
    CON(ori, "ORION", "ORION"),
    CON(cma, "GROSSER HUND", "CANIS MAIOR"),
    CON(cmi, "KLEINER HUND", "CANIS MINOR"),
    CON(gem, "ZWILLINGE", "GEMINI"),
    CON(leo, "LOEWE", "LEO"),
};
#define SKY_NCON ((int)(sizeof SKY_CON / sizeof SKY_CON[0]))

/* bright stars outside the figures */
static const sky_star SKY_LONE[] = {
    { 22.961f, -29.622f, 1.16f, 0, 0, 0 },   /* Fomalhaut */
    { 9.460f, -8.659f, 1.98f, 'r', 0, 0 },   /* Alphard */
    { 17.582f, 12.560f, 2.07f, 0, 0, 0 },    /* Rasalhague */
    { 17.173f, -15.725f, 2.43f, 0, 0, 0 },   /* Sabik */
    { 0.727f, -17.987f, 2.04f, 'y', 0, 0 },  /* Diphda */
    { 15.738f, 6.426f, 2.63f, 'y', 0, 0 },   /* Unukalhai */
    { 14.848f, -16.042f, 2.75f, 0, 0, 0 },   /* Zubenelgenubi */
    { 15.283f, -9.383f, 2.61f, 'b', 0, 0 },  /* Zubeneschamali */
    { 3.038f, 4.090f, 2.54f, 'r', 0, 0 },    /* Menkar */
    { 12.263f, -17.542f, 2.59f, 'b', 0, 0 }, /* Gienah (Rabe) */
    { 12.573f, -23.397f, 2.65f, 'y', 0, 0 }, /* Kraz */
    { 12.498f, -16.515f, 2.95f, 0, 0, 0 },   /* Algorab */
    { 12.140f, -24.729f, 4.02f, 0, 0, 0 },   /* Alchiba */
    { 16.619f, -10.567f, 2.56f, 'b', 0, 0 }, /* Zeta Ophiuchi */
    { 16.239f, -3.694f, 2.73f, 'r', 0, 0 },  /* Yed Prior */
    { 17.725f, 4.567f, 2.77f, 'y', 0, 0 },   /* Cebalrai */
    { 20.350f, -14.781f, 3.08f, 'y', 0, 0 }, /* Dabih */
    { 21.526f, -5.571f, 2.87f, 'y', 0, 0 },  /* Sadalsuud */
    { 22.096f, -0.320f, 2.95f, 'y', 0, 0 },  /* Sadalmelik */
    { 21.784f, -16.127f, 2.85f, 0, 0, 0 },   /* Deneb Algedi */
    { 8.275f, 9.186f, 3.52f, 'r', 0, 0 },    /* Altarf (Krebs) */
    { 7.167f, -0.493f, 3.75f, 0, 0, 0 },     /* (Einhorn) */
};
#define SKY_NLONE ((int)(sizeof SKY_LONE / sizeof SKY_LONE[0]))

#endif
