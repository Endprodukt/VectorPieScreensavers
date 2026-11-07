/* weather.h — the weather, fetched now and then in the background from
 * Open-Meteo (open-meteo.com: free, no key), by curl (or wget), for the
 * place the menu sets (settings.h). With it comes the place's offset from
 * UTC, so the clock can show the time there. Places can be searched for by
 * name (Open-Meteo's geocoding).
 *
 * No place, no network: weather_get says so, and the clock shows none.
 */
#ifndef WEATHER_H
#define WEATHER_H

#define WX_DAYS  7
#define WX_HOURS 24

typedef struct {
    int ok;                     /* 1: the fields below hold something */
    int serial;                 /* the place it is for (weather_set_place's count) */
    float temp, feels, wind, humidity;  /* now: degrees C, felt, km/h, % */
    int wind_dir;               /* where the wind comes from, degrees */
    int code, is_day;           /* WMO weather code; 1 by day */
    int utc_offset;             /* seconds the place is ahead of UTC, summer time included */
    int sunrise, sunset;        /* today, minutes after midnight there; -1 not known */
    int days;                   /* the forecast: today and the next days */
    int day_code[WX_DAYS], day_pop[WX_DAYS];        /* pop: the chance of rain, % */
    float day_max[WX_DAYS], day_min[WX_DAYS];
    int hours;                  /* the next hours, from this one */
    int hour_of[WX_HOURS], hour_code[WX_HOURS], hour_pop[WX_HOURS];
    float hour_temp[WX_HOURS];
} weather_t;

typedef struct { char name[48]; double lat, lon; } place_t;

/* Start the fetching thread (once). */
void weather_start(void);

/* A new place (has 0: none); fetched at once. Returns its serial. */
int  weather_set_place(int has, double lat, double lon);

/* The latest weather (a copy). */
void weather_get(weather_t *out);

/* Search places by name, in the background; then weather_search_state: 0
 * nothing asked, 1 searching, 2 done (n results in out), 3 failed. */
void weather_search(const char *query, int lang);
int  weather_search_state(place_t *out, int max, int *n);

/* WMO weather code as words in the language (lang.h). */
const char *weather_words(int code, int lang);

#endif
