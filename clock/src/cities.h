/* cities.h — places to choose in the menu without typing (any other can be
 * searched for). Their time zone comes with the weather (Open-Meteo gives
 * the place's offset from UTC, summer time included). */
#ifndef CITIES_H
#define CITIES_H

static const struct { const char *name; float lat, lon; } CITY[] = {
    { "BERLIN", 52.52f, 13.41f },        { "HAMBURG", 53.55f, 9.99f },        { "MUNICH", 48.14f, 11.58f },
    { "COLOGNE", 50.94f, 6.96f },        { "FRANKFURT", 50.11f, 8.68f },      { "STUTTGART", 48.78f, 9.18f },
    { "DUSSELDORF", 51.23f, 6.78f },     { "LEIPZIG", 51.34f, 12.37f },       { "DRESDEN", 51.05f, 13.74f },
    { "HANOVER", 52.37f, 9.73f },        { "VIENNA", 48.21f, 16.37f },        { "ZURICH", 47.38f, 8.54f },
    { "LONDON", 51.51f, -0.13f },        { "DUBLIN", 53.35f, -6.26f },        { "PARIS", 48.86f, 2.35f },
    { "LYON", 45.76f, 4.84f },           { "MARSEILLE", 43.30f, 5.37f },      { "BRUSSELS", 50.85f, 4.35f },
    { "AMSTERDAM", 52.37f, 4.90f },      { "LUXEMBOURG", 49.61f, 6.13f },     { "MADRID", 40.42f, -3.70f },
    { "BARCELONA", 41.39f, 2.17f },      { "SEVILLE", 37.39f, -5.98f },       { "VALENCIA", 39.47f, -0.38f },
    { "LISBON", 38.72f, -9.14f },        { "ROME", 41.90f, 12.50f },          { "MILAN", 45.46f, 9.19f },
    { "COPENHAGEN", 55.68f, 12.57f },    { "STOCKHOLM", 59.33f, 18.07f },     { "OSLO", 59.91f, 10.75f },
    { "HELSINKI", 60.17f, 24.94f },      { "WARSAW", 52.23f, 21.01f },        { "PRAGUE", 50.08f, 14.44f },
    { "BUDAPEST", 47.50f, 19.04f },      { "ATHENS", 37.98f, 23.73f },        { "ISTANBUL", 41.01f, 28.98f },
    { "MOSCOW", 55.76f, 37.62f },        { "CAIRO", 30.04f, 31.24f },         { "CAPE TOWN", -33.92f, 18.42f },
    { "DUBAI", 25.20f, 55.27f },         { "MUMBAI", 19.08f, 72.88f },        { "BANGKOK", 13.76f, 100.50f },
    { "SINGAPORE", 1.35f, 103.82f },     { "HONG KONG", 22.32f, 114.17f },    { "TOKYO", 35.68f, 139.69f },
    { "SEOUL", 37.57f, 126.98f },        { "SYDNEY", -33.87f, 151.21f },      { "AUCKLAND", -36.85f, 174.76f },
    { "HONOLULU", 21.31f, -157.86f },    { "LOS ANGELES", 34.05f, -118.24f }, { "SAN FRANCISCO", 37.77f, -122.42f },
    { "DENVER", 39.74f, -104.99f },      { "CHICAGO", 41.88f, -87.63f },      { "NEW YORK", 40.71f, -74.01f },
    { "TORONTO", 43.65f, -79.38f },      { "MEXICO CITY", 19.43f, -99.13f },  { "SAO PAULO", -23.55f, -46.63f },
    { "BUENOS AIRES", -34.60f, -58.38f },
};
#define NCITY ((int)(sizeof CITY / sizeof CITY[0]))

#endif
