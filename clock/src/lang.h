/* lang.h — the clock's words in its four languages: English (the
 * standard), German, French, Spanish. Capitals, without accents or umlauts
 * (the fonts know only ASCII). */
#ifndef LANG_H
#define LANG_H

enum { LANG_EN, LANG_DE, LANG_FR, LANG_ES, NLANG };

static const char *const LANG_NAME[NLANG] = { "ENGLISH", "DEUTSCH", "FRANCAIS", "ESPANOL" };

static const char *const L_WEEKDAY[NLANG][7] = {
    { "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY" },
    { "SONNTAG", "MONTAG", "DIENSTAG", "MITTWOCH", "DONNERSTAG", "FREITAG", "SAMSTAG" },
    { "DIMANCHE", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI" },
    { "DOMINGO", "LUNES", "MARTES", "MIERCOLES", "JUEVES", "VIERNES", "SABADO" },
};
static const char *const L_WEEKDAY_SHORT[NLANG][7] = {
    { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" },
    { "SO", "MO", "DI", "MI", "DO", "FR", "SA" },
    { "DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM" },
    { "DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB" },
};
static const char *const L_MONTH[NLANG][12] = {
    { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" },
    { "JAN", "FEB", "MAR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ" },
    { "JANV", "FEVR", "MARS", "AVR", "MAI", "JUIN", "JUIL", "AOUT", "SEPT", "OCT", "NOV", "DEC" },
    { "ENE", "FEB", "MAR", "ABR", "MAY", "JUN", "JUL", "AGO", "SEP", "OCT", "NOV", "DIC" },
};

/* the weather: clear, mainly clear, partly cloudy, overcast, fog, drizzle,
 * rain, snow, showers, snow showers, thunderstorm */
static const char *const L_WEATHER[NLANG][11] = {
    { "CLEAR", "MAINLY CLEAR", "PARTLY CLOUDY", "OVERCAST", "FOG", "DRIZZLE", "RAIN", "SNOW", "SHOWERS",
      "SNOW SHOWERS", "THUNDERSTORM" },
    { "KLAR", "HEITER", "WOLKIG", "BEDECKT", "NEBEL", "NIESELREGEN", "REGEN", "SCHNEE", "SCHAUER",
      "SCHNEESCHAUER", "GEWITTER" },
    { "DEGAGE", "PEU NUAGEUX", "NUAGEUX", "COUVERT", "BROUILLARD", "BRUINE", "PLUIE", "NEIGE", "AVERSES",
      "AVERSES DE NEIGE", "ORAGE" },
    { "DESPEJADO", "POCO NUBOSO", "NUBOSO", "CUBIERTO", "NIEBLA", "LLOVIZNA", "LLUVIA", "NIEVE", "CHUBASCOS",
      "CHUBASCOS DE NIEVE", "TORMENTA" },
};

static const char *const L_FACE[NLANG][10] = {
    { "ANALOG", "DIGITAL", "FLIP", "BINARY", "WORDS", "NIXIE", "ORBIT", "TUNNEL", "ROCKS", "WEATHER" },
    { "ANALOG", "DIGITAL", "KLAPPZAHLEN", "BINAER", "WORTUHR", "NIXIE", "ORBIT", "TUNNEL", "ROCKS", "WETTER" },
    { "ANALOGIQUE", "NUMERIQUE", "VOLETS", "BINAIRE", "MOTS", "NIXIE", "ORBITE", "TUNNEL", "ROCHERS", "METEO" },
    { "ANALOGICO", "DIGITAL", "PALETAS", "BINARIO", "PALABRAS", "NIXIE", "ORBITA", "TUNEL", "ROCAS", "TIEMPO" },
};

/* the words of the screens and the menu */
enum {
    S_TODAY, S_WIND, S_HUMIDITY, S_NO_WEATHER, S_NO_WEATHER_HINT, S_SETTINGS,
    S_FACE, S_CHANGE, S_COLOR, S_SECONDS, S_DATE, S_WEATHER, S_H24, S_TITLE, S_PLACE, S_LANGUAGE, S_CLOSE,
    S_ON, S_OFF, S_WITH_TRACK, S_EVERY_MIN, S_LOCAL, S_SEARCH, S_SEARCHING, S_NOTHING_FOUND, S_DELETE,
    S_SPACE, S_GO, S_BACK_HINT, S_MENU_HINT, NSTR
};
static const char *const L_STR[NLANG][NSTR] = {
    { "TODAY", "WIND", "HUMIDITY", "NO WEATHER", "CHOOSE A PLACE IN THE MENU", "CLOCK SETTINGS",
      "FACE", "CHANGE FACE", "COLOR", "SECONDS", "DATE", "WEATHER", "24 HOURS", "TRACK TITLE", "PLACE", "LANGUAGE",
      "CLOSE", "ON", "OFF", "WITH EACH TRACK", "EVERY %d MIN", "LOCAL TIME", "SEARCH...", "SEARCHING...",
      "NOTHING FOUND", "DEL", "SPACE", "SEARCH", "BACK: CLOSE", "START 1: MENU" },
    { "HEUTE", "WIND", "FEUCHTE", "KEIN WETTER", "ORT IM MENUE WAEHLEN", "UHR EINSTELLEN",
      "ZIFFERBLATT", "WECHSELN", "FARBE", "SEKUNDEN", "DATUM", "WETTER", "24 STUNDEN", "TRACKTITEL", "ORT", "SPRACHE",
      "SCHLIESSEN", "AN", "AUS", "MIT JEDEM TRACK", "ALLE %d MIN", "ORTSZEIT", "SUCHEN...", "SUCHE...",
      "NICHTS GEFUNDEN", "LOESCHEN", "LEER", "SUCHEN", "ZURUECK: SCHLIESSEN", "START 1: MENUE" },
    { "AUJ.", "VENT", "HUMIDITE", "PAS DE METEO", "CHOISIR UN LIEU DANS LE MENU", "REGLAGES",
      "CADRAN", "CHANGER", "COULEUR", "SECONDES", "DATE", "METEO", "24 HEURES", "TITRE", "LIEU", "LANGUE",
      "FERMER", "OUI", "NON", "A CHAQUE MORCEAU", "TOUTES LES %d MIN", "HEURE LOCALE", "CHERCHER...",
      "RECHERCHE...", "RIEN TROUVE", "EFFACER", "ESPACE", "CHERCHER", "RETOUR: FERMER", "START 1: MENU" },
    { "HOY", "VIENTO", "HUMEDAD", "SIN TIEMPO", "ELIGE UN LUGAR EN EL MENU", "AJUSTES",
      "ESFERA", "CAMBIAR", "COLOR", "SEGUNDOS", "FECHA", "TIEMPO", "24 HORAS", "TITULO", "LUGAR", "IDIOMA",
      "CERRAR", "SI", "NO", "CON CADA PISTA", "CADA %d MIN", "HORA LOCAL", "BUSCAR...", "BUSCANDO...",
      "SIN RESULTADOS", "BORRAR", "ESPACIO", "BUSCAR", "ATRAS: CERRAR", "START 1: MENU" },
};

/* the colors: the faces' own, or all in one */
#define NTHEME 9
static const char *const L_THEME[NLANG][NTHEME] = {
    { "ORIGINAL", "CYAN", "GREEN", "AMBER", "RED", "MAGENTA", "BLUE", "WHITE", "RAINBOW" },
    { "ORIGINAL", "CYAN", "GRUEN", "BERNSTEIN", "ROT", "MAGENTA", "BLAU", "WEISS", "REGENBOGEN" },
    { "ORIGINAL", "CYAN", "VERT", "AMBRE", "ROUGE", "MAGENTA", "BLEU", "BLANC", "ARC-EN-CIEL" },
    { "ORIGINAL", "CIAN", "VERDE", "AMBAR", "ROJO", "MAGENTA", "AZUL", "BLANCO", "ARCOIRIS" },
};
static const float THEME_RGB[NTHEME][3] = {
    { 0, 0, 0 }, { 60, 235, 255 }, { 80, 255, 120 }, { 255, 175, 50 }, { 255, 70, 60 },
    { 255, 80, 220 }, { 90, 130, 255 }, { 235, 240, 255 }, { 0, 0, 0 },
};

/* changing faces by itself: off, with each track, every so many minutes */
#define NCHANGE 6
static const int CHANGE_MIN[NCHANGE] = { 0, 0, 1, 5, 15, 60 };

#endif
