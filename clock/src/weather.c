/* weather.c — see weather.h. A thread of its own: fetching never holds up
 * a frame. */
#include "weather.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#define popen _popen
#define pclose _pclose
static void nap(int s) { Sleep((DWORD)s * 1000); }
#else
#include <unistd.h>
static void nap(int s) { sleep((unsigned)s); }
#endif

#define EVERY_S   (15 * 60)             /* a new fetch every quarter of an hour */
#define RETRY_S   60                    /* after a failure */

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static weather_t latest;
static char cfg_place[64], cfg_name[48];
static double cfg_lat, cfg_lon;
static int cfg_has_ll;

/* ---- clock.ini ---------------------------------------------------------- */

static void trim(char *s)
{
    char *e = s + strlen(s);
    while (e > s && (e[-1] == '\n' || e[-1] == '\r' || e[-1] == ' ' || e[-1] == '\t')) *--e = 0;
    char *b = s;
    while (*b == ' ' || *b == '\t') b++;
    memmove(s, b, strlen(b) + 1);
}

static int read_config(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    char ln[256];
    int lat = 0, lon = 0;
    while (fgets(ln, sizeof ln, f)) {
        char *c = strchr(ln, ';');
        if (c) *c = 0;
        char *eq = strchr(ln, '=');
        if (!eq) continue;
        *eq = 0;
        char *k = ln, *v = eq + 1;
        trim(k);
        trim(v);
        if (!strcmp(k, "place")) snprintf(cfg_place, sizeof cfg_place, "%s", v);
        else if (!strcmp(k, "name")) snprintf(cfg_name, sizeof cfg_name, "%s", v);
        else if (!strcmp(k, "lat")) { cfg_lat = atof(v); lat = 1; }
        else if (!strcmp(k, "lon")) { cfg_lon = atof(v); lon = 1; }
    }
    fclose(f);
    cfg_has_ll = lat && lon;
    return 1;
}

/* ---- fetching ------------------------------------------------------------- */

static char *fetch(const char *url)
{
    char cmd[1024];
    /* double quotes: the same in sh and in cmd */
    snprintf(cmd, sizeof cmd, "curl -s -m 15 \"%s\" || wget -q -T 15 -O - \"%s\"", url, url);
    FILE *p = popen(cmd, "r");
    if (!p) return NULL;
    size_t cap = 1 << 16, n = 0;
    char *buf = malloc(cap);
    if (!buf) { pclose(p); return NULL; }
    size_t r;
    while ((r = fread(buf + n, 1, cap - 1 - n, p)) > 0) {
        n += r;
        if (n >= cap - 1) break;
    }
    buf[n] = 0;
    pclose(p);
    if (n == 0) { free(buf); return NULL; }
    return buf;
}

/* the number after "key": inside the part of json from `from` */
static int num_after(const char *from, const char *key, double *out)
{
    char pat[64];
    snprintf(pat, sizeof pat, "\"%s\":", key);
    const char *p = strstr(from, pat);
    if (!p) return 0;
    p += strlen(pat);
    char *end;
    double v = strtod(p, &end);
    if (end == p) return 0;
    *out = v;
    return 1;
}

static int array_after(const char *from, const char *key, double *out, int max)
{
    char pat[64];
    snprintf(pat, sizeof pat, "\"%s\":[", key);
    const char *p = strstr(from, pat);
    if (!p) return 0;
    p += strlen(pat);
    int n = 0;
    while (n < max && *p && *p != ']') {
        char *end;
        double v = strtod(p, &end);
        if (end == p) break;
        out[n++] = v;
        p = end;
        while (*p == ',' || *p == ' ') p++;
    }
    return n;
}

static void url_name(char *out, size_t size, const char *s)
{
    size_t n = 0;
    for (; *s && n + 4 < size; s++) {
        unsigned char c = (unsigned char)*s;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '.')
            out[n++] = (char)c;
        else n += (size_t)snprintf(out + n, size - n, "%%%02X", c);
    }
    out[n] = 0;
}

/* the place's name for the screen: capitals, umlauts spelt out */
static void screen_name(char *out, size_t size, const char *s)
{
    size_t n = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p && n + 3 < size; p++) {
        if (p[0] == 0xC3 && p[1]) {                     /* UTF-8 umlauts */
            const char *r = p[1] == 0xA4 || p[1] == 0x84 ? "AE" : p[1] == 0xB6 || p[1] == 0x96 ? "OE"
                          : p[1] == 0xBC || p[1] == 0x9C ? "UE" : p[1] == 0x9F ? "SS" : "";
            for (; *r && n + 1 < size; r++) out[n++] = *r;
            p++;
            continue;
        }
        if (*p < 128) out[n++] = (char)(*p >= 'a' && *p <= 'z' ? *p - 32 : *p);
    }
    out[n] = 0;
}

static int locate(void)
{
    if (cfg_has_ll) return 1;
    if (!cfg_place[0]) return 0;
    char q[160], url[400];
    url_name(q, sizeof q, cfg_place);
    snprintf(url, sizeof url, "https://geocoding-api.open-meteo.com/v1/search?name=%s&count=1&language=de", q);
    char *js = fetch(url);
    if (!js) return 0;
    const char *res = strstr(js, "\"results\"");
    int ok = res && num_after(res, "latitude", &cfg_lat) && num_after(res, "longitude", &cfg_lon);
    if (ok && !cfg_name[0]) {
        const char *p = strstr(res, "\"name\":\"");
        if (p) {
            p += 8;
            const char *e = strchr(p, '"');
            if (e && e - p < (int)sizeof cfg_name) { memcpy(cfg_name, p, (size_t)(e - p)); cfg_name[e - p] = 0; }
        }
    }
    free(js);
    cfg_has_ll = ok;
    return ok;
}

static int fetch_weather(void)
{
    char url[512];
    snprintf(url, sizeof url,
             "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,weather_code,is_day,wind_speed_10m,relative_humidity_2m"
             "&daily=weather_code,temperature_2m_max,temperature_2m_min&timezone=auto&forecast_days=4",
             cfg_lat, cfg_lon);
    char *js = fetch(url);
    if (!js) return 0;
    weather_t w;
    memset(&w, 0, sizeof w);
    const char *cur = strstr(js, "\"current\":{"), *day = strstr(js, "\"daily\":{");
    double t, c, dayf = 1, wind = 0, hum = 0;
    int ok = cur && num_after(cur, "temperature_2m", &t) && num_after(cur, "weather_code", &c);
    if (ok) {
        num_after(cur, "is_day", &dayf);
        num_after(cur, "wind_speed_10m", &wind);
        num_after(cur, "relative_humidity_2m", &hum);
        w.ok = 1;
        w.temp = (float)t;
        w.code = (int)c;
        w.is_day = dayf > 0.5;
        w.wind = (float)wind;
        w.humidity = (float)hum;
        screen_name(w.place, sizeof w.place, cfg_name[0] ? cfg_name : cfg_place);
        if (day) {
            double a[4], b[4], e[4];
            int n1 = array_after(day, "weather_code", a, 4), n2 = array_after(day, "temperature_2m_max", b, 4),
                n3 = array_after(day, "temperature_2m_min", e, 4);
            int n = n1 < n2 ? n1 : n2;
            n = n < n3 ? n : n3;
            for (int i = 0; i < n; i++) {
                w.day_code[i] = (int)a[i];
                w.day_max[i] = (float)b[i];
                w.day_min[i] = (float)e[i];
            }
            w.days = n;
        }
        w.fetched = (double)time(NULL);
        pthread_mutex_lock(&lock);
        latest = w;
        pthread_mutex_unlock(&lock);
    }
    free(js);
    return ok;
}

static void *worker(void *arg)
{
    (void)arg;
    for (;;) {
        int ok = locate() && fetch_weather();
        nap(ok ? EVERY_S : RETRY_S);
    }
    return NULL;
}

void weather_start(const char *exe_path)
{
    /* clock.ini beside the program, else in the folder it runs in */
    char path[512];
    snprintf(path, sizeof path, "%s", exe_path ? exe_path : "");
    char *slash = strrchr(path, '/'), *bslash = strrchr(path, '\\');
    if (bslash > slash) slash = bslash;
    int found = 0;
    if (slash) {
        snprintf(slash + 1, sizeof path - (size_t)(slash + 1 - path), "clock.ini");
        found = read_config(path);
    }
    if (!found) found = read_config("clock.ini");
    if (!found || (!cfg_place[0] && !cfg_has_ll)) return;   /* no place: no weather */
    pthread_t t;
    if (pthread_create(&t, NULL, worker, NULL) == 0) pthread_detach(t);
}

void weather_get(weather_t *out)
{
    pthread_mutex_lock(&lock);
    *out = latest;
    pthread_mutex_unlock(&lock);
}

const char *weather_words(int c)
{
    if (c == 0) return "KLAR";
    if (c == 1) return "HEITER";
    if (c == 2) return "WOLKIG";
    if (c == 3) return "BEDECKT";
    if (c == 45 || c == 48) return "NEBEL";
    if (c >= 51 && c <= 57) return "NIESELREGEN";
    if (c >= 61 && c <= 67) return "REGEN";
    if (c >= 71 && c <= 77) return "SCHNEE";
    if (c >= 80 && c <= 82) return "SCHAUER";
    if (c >= 85 && c <= 86) return "SCHNEESCHAUER";
    if (c >= 95) return "GEWITTER";
    return "";
}
