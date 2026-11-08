/* weather.c — see weather.h. A thread of its own: fetching never holds up
 * a frame. */
#include "weather.h"
#include "lang.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#define popen _popen
#define pclose _pclose
static void nap_ms(int ms) { Sleep((DWORD)ms); }
#else
#include <unistd.h>
static void nap_ms(int ms) { usleep((useconds_t)ms * 1000); }
#endif

#define EVERY_S   (15 * 60)             /* a new fetch every quarter of an hour */
#define RETRY_S   60                    /* after a failure */

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static weather_t latest;
/* what the thread is to do (under the lock) */
static int want_serial, want_has;
static double want_lat, want_lon;
static char want_search[64];
static int want_search_lang, search_state;
static place_t found[6];
static int nfound;

/* ---- fetching ------------------------------------------------------------- */

static char *fetch(const char *url)
{
    char cmd[1024];
    /* double quotes: the same in sh and in cmd */
    snprintf(cmd, sizeof cmd, "curl -s --connect-timeout 6 -m 15 \"%s\" || wget -q -t 1 -T 15 -O - \"%s\"", url, url);
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

/* the number after "key": in the json from `from` */
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

/* "key":["2026-11-07T07:21", ...]: the times as minutes after midnight */
static int times_after(const char *from, const char *key, int *out, int max)
{
    char pat[64];
    snprintf(pat, sizeof pat, "\"%s\":[", key);
    const char *p = strstr(from, pat);
    if (!p) return 0;
    p += strlen(pat);
    int n = 0;
    while (n < max && *p == '"') {
        const char *t = strchr(p + 1, 'T'), *e = strchr(p + 1, '"');
        if (!e) break;
        out[n++] = t && t < e ? atoi(t + 1) * 60 + atoi(t + 4) : -1;
        p = e + 1;
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

/* a name for the screen: capitals, umlauts and accents spelt out or dropped */
static void screen_name(char *out, size_t size, const char *s)
{
    size_t n = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p && n + 3 < size; p++) {
        if (p[0] == 0xC3 && p[1]) {                     /* UTF-8 Latin-1 letters */
            unsigned c = p[1];
            const char *r = c == 0xA4 || c == 0x84 ? "AE" : c == 0xB6 || c == 0x96 ? "OE"
                          : c == 0xBC || c == 0x9C ? "UE" : c == 0x9F ? "SS"
                          : (c >= 0x80 && c <= 0x85) || (c >= 0xA0 && c <= 0xA5) ? "A"
                          : (c >= 0x88 && c <= 0x8B) || (c >= 0xA8 && c <= 0xAB) ? "E"
                          : (c >= 0x8C && c <= 0x8F) || (c >= 0xAC && c <= 0xAF) ? "I"
                          : (c >= 0x92 && c <= 0x95) || (c >= 0xB2 && c <= 0xB5) ? "O"
                          : (c >= 0x99 && c <= 0x9B) || (c >= 0xB9 && c <= 0xBB) ? "U"
                          : c == 0x91 || c == 0xB1 ? "N" : c == 0x87 || c == 0xA7 ? "C" : "";
            for (; *r && n + 1 < size; r++) out[n++] = *r;
            p++;
            continue;
        }
        if (*p < 128) out[n++] = (char)(*p >= 'a' && *p <= 'z' ? *p - 32 : *p);
    }
    out[n] = 0;
}

static int fetch_weather(int serial, double lat, double lon)
{
    char url[512];
    snprintf(url, sizeof url,
             "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,apparent_temperature,weather_code,is_day,wind_speed_10m,"
             "wind_direction_10m,relative_humidity_2m"
             "&hourly=temperature_2m,weather_code,precipitation_probability&forecast_hours=24"
             "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max,"
             "sunrise,sunset&timezone=auto&forecast_days=7",
             lat, lon);
    char *js = fetch(url);
    if (!js) return 0;
    weather_t w;
    memset(&w, 0, sizeof w);
    const char *cur = strstr(js, "\"current\":{"), *day = strstr(js, "\"daily\":{"), *hour = strstr(js, "\"hourly\":{");
    double t, c, dayf = 1, wind = 0, hum = 0, off = 0, feels, wdir = 0;
    int ok = cur && num_after(cur, "temperature_2m", &t) && num_after(cur, "weather_code", &c);
    if (ok) {
        num_after(cur, "is_day", &dayf);
        num_after(cur, "wind_speed_10m", &wind);
        num_after(cur, "relative_humidity_2m", &hum);
        num_after(js, "utc_offset_seconds", &off);
        w.ok = 1;
        w.serial = serial;
        w.temp = (float)t;
        w.code = (int)c;
        w.is_day = dayf > 0.5;
        w.wind = (float)wind;
        w.humidity = (float)hum;
        w.utc_offset = (int)off;
        w.feels = num_after(cur, "apparent_temperature", &feels) ? (float)feels : w.temp;
        num_after(cur, "wind_direction_10m", &wdir);
        w.wind_dir = (int)wdir;
        w.sunrise = w.sunset = -1;
        if (day) {
            double a[WX_DAYS], b[WX_DAYS], e[WX_DAYS], pp[WX_DAYS];
            int n1 = array_after(day, "weather_code", a, WX_DAYS), n2 = array_after(day, "temperature_2m_max", b, WX_DAYS),
                n3 = array_after(day, "temperature_2m_min", e, WX_DAYS);
            int np = array_after(day, "precipitation_probability_max", pp, WX_DAYS);
            int n = n1 < n2 ? n1 : n2;
            n = n < n3 ? n : n3;
            for (int i = 0; i < n; i++) {
                w.day_code[i] = (int)a[i];
                w.day_max[i] = (float)b[i];
                w.day_min[i] = (float)e[i];
                w.day_pop[i] = i < np ? (int)pp[i] : -1;
            }
            w.days = n;
            int sr[1], ss[1];
            if (times_after(day, "sunrise", sr, 1) == 1) w.sunrise = sr[0];
            if (times_after(day, "sunset", ss, 1) == 1) w.sunset = ss[0];
        }
        if (hour) {
            double a[WX_HOURS], b[WX_HOURS], pp[WX_HOURS];
            int tm[WX_HOURS];
            int n1 = times_after(hour, "time", tm, WX_HOURS), n2 = array_after(hour, "temperature_2m", a, WX_HOURS),
                n3 = array_after(hour, "weather_code", b, WX_HOURS);
            int np = array_after(hour, "precipitation_probability", pp, WX_HOURS);
            int n = n1 < n2 ? n1 : n2;
            n = n < n3 ? n : n3;
            for (int i = 0; i < n; i++) {
                w.hour_of[i] = tm[i] / 60;
                w.hour_temp[i] = (float)a[i];
                w.hour_code[i] = (int)b[i];
                w.hour_pop[i] = i < np ? (int)pp[i] : 0;
            }
            w.hours = n;
        }
        pthread_mutex_lock(&lock);
        if (serial == want_serial) latest = w;          /* (not if the place changed meanwhile) */
        pthread_mutex_unlock(&lock);
    }
    free(js);
    return ok;
}

static void do_search(const char *q, int lang)
{
    static const char *LC[NLANG] = { "en", "de", "fr", "es" };
    char name[160], url[400];
    url_name(name, sizeof name, q);
    snprintf(url, sizeof url, "https://geocoding-api.open-meteo.com/v1/search?name=%s&count=6&language=%s", name,
             LC[lang >= 0 && lang < NLANG ? lang : 0]);
    char *js = fetch(url);
    place_t res[6];
    int n = 0;
    if (js) {
        const char *p = strstr(js, "\"results\"");
        while (p && n < 6 && (p = strstr(p, "\"name\":\"")) != NULL) {
            const char *s = p + 8, *e = strchr(s, '"');
            if (!e) break;
            char raw[96];
            size_t l = (size_t)(e - s) < sizeof raw - 1 ? (size_t)(e - s) : sizeof raw - 1;
            memcpy(raw, s, l);
            raw[l] = 0;
            double lat, lon;
            const char *next = strstr(e, "\"name\":\"");
            /* its country, if within this result */
            const char *cc = strstr(e, "\"country\":\"");
            char country[48] = "";
            if (cc && (!next || cc < next)) {
                const char *cs = cc + 11, *ce = strchr(cs, '"');
                if (ce && ce - cs < (int)sizeof country) { memcpy(country, cs, (size_t)(ce - cs)); country[ce - cs] = 0; }
            }
            if (num_after(e, "latitude", &lat) && num_after(e, "longitude", &lon)) {
                char full[160];
                snprintf(full, sizeof full, country[0] ? "%s, %s" : "%s", raw, country);
                screen_name(res[n].name, sizeof res[n].name, full);
                res[n].lat = lat;
                res[n].lon = lon;
                n++;
            }
            p = next;
        }
        free(js);
    }
    pthread_mutex_lock(&lock);
    memcpy(found, res, sizeof res);
    nfound = n;
    search_state = js ? 2 : 3;
    pthread_mutex_unlock(&lock);
}

static void *worker(void *arg)
{
    (void)arg;
    int done_serial = -1;
    time_t next = 0;
    for (;;) {
        pthread_mutex_lock(&lock);
        int serial = want_serial, has = want_has;
        double lat = want_lat, lon = want_lon;
        char q[64];
        snprintf(q, sizeof q, "%s", want_search);
        int lang = want_search_lang;
        want_search[0] = 0;
        pthread_mutex_unlock(&lock);
        if (q[0]) do_search(q, lang);
        time_t now = time(NULL);
        if (has && (serial != done_serial || now >= next)) {
            int ok = fetch_weather(serial, lat, lon);
            done_serial = serial;
            next = now + (ok ? EVERY_S : RETRY_S);
        }
        nap_ms(250);
    }
    return NULL;
}

void weather_start(void)
{
    pthread_t t;
    if (pthread_create(&t, NULL, worker, NULL) == 0) pthread_detach(t);
}

int weather_set_place(int has, double lat, double lon)
{
    pthread_mutex_lock(&lock);
    want_serial++;
    want_has = has;
    want_lat = lat;
    want_lon = lon;
    memset(&latest, 0, sizeof latest);                  /* the old place's weather is gone */
    int s = want_serial;
    pthread_mutex_unlock(&lock);
    return s;
}

void weather_get(weather_t *out)
{
    pthread_mutex_lock(&lock);
    *out = latest;
    pthread_mutex_unlock(&lock);
}

void weather_search(const char *query, int lang)
{
    pthread_mutex_lock(&lock);
    snprintf(want_search, sizeof want_search, "%s", query);
    want_search_lang = lang;
    search_state = 1;
    nfound = 0;
    pthread_mutex_unlock(&lock);
}

int weather_search_state(place_t *out, int max, int *n)
{
    pthread_mutex_lock(&lock);
    int s = search_state, k = nfound < max ? nfound : max;
    for (int i = 0; i < k; i++) out[i] = found[i];
    *n = k;
    pthread_mutex_unlock(&lock);
    return s;
}

const char *weather_words(int c, int lang)
{
    if (lang < 0 || lang >= NLANG) lang = 0;
    int i = c == 0 ? 0 : c == 1 ? 1 : c == 2 ? 2 : c == 3 ? 3 : (c == 45 || c == 48) ? 4
          : (c >= 51 && c <= 57) ? 5 : (c >= 61 && c <= 67) ? 6 : (c >= 71 && c <= 77) ? 7
          : (c >= 80 && c <= 82) ? 8 : (c >= 85 && c <= 86) ? 9 : c >= 95 ? 10 : -1;
    return i < 0 ? "" : L_WEATHER[lang][i];
}
