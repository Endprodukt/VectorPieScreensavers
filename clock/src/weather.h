/* weather.h — the weather, fetched now and then in the background from
 * Open-Meteo (open-meteo.com: free, no key), by curl (or wget).
 *
 * The place comes from clock.ini (beside the program, or in the folder it
 * runs in):
 *
 *   place = Berlin              (looked up by Open-Meteo's geocoding)
 *   ; or exactly:
 *   lat = 52.52
 *   lon = 13.41
 *   name = Berlin
 *
 * No place, no network: weather_get says so, and the clock shows none.
 */
#ifndef WEATHER_H
#define WEATHER_H

typedef struct {
    int ok;                     /* 1: the fields below hold something */
    char place[48];
    float temp, wind, humidity; /* now: degrees C, km/h, % */
    int code, is_day;           /* WMO weather code; 1 by day */
    int days;                   /* the forecast: today and the next days */
    int day_code[4];
    float day_max[4], day_min[4];
    double fetched;             /* when (seconds since start) */
} weather_t;

/* Start fetching (once, at start), with the folder of the program. */
void weather_start(const char *exe_path);

/* The latest weather (a copy). */
void weather_get(weather_t *out);

/* WMO weather code as German words, in capitals without umlauts. */
const char *weather_words(int code);

#endif
