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

#define NFACES 15               /* the weather last */
static const char *const L_FACE[NLANG][NFACES] = {
    { "ANALOG", "DIGITAL", "FLIP", "BINARY", "WORDS", "NIXIE", "ORBIT", "TUNNEL", "ROCKS",
      "MINIMAL", "NUMERALS", "ROMAN", "SQUARE", "FULL SCREEN", "WEATHER" },
    { "ANALOG", "DIGITAL", "KLAPPZAHLEN", "BINAER", "WORTUHR", "NIXIE", "ORBIT", "TUNNEL", "ROCKS",
      "MINIMAL", "NUR ZAHLEN", "ROEMISCH", "ECKIG", "VOLLBILD", "WETTER" },
    { "ANALOGIQUE", "NUMERIQUE", "VOLETS", "BINAIRE", "MOTS", "NIXIE", "ORBITE", "TUNNEL", "ROCHERS",
      "MINIMAL", "CHIFFRES", "ROMAIN", "CARRE", "PLEIN ECRAN", "METEO" },
    { "ANALOGICO", "DIGITAL", "PALETAS", "BINARIO", "PALABRAS", "NIXIE", "ORBITA", "TUNEL", "ROCAS",
      "MINIMAL", "NUMEROS", "ROMANO", "CUADRADO", "PANTALLA", "TIEMPO" },
};

/* the words of the screens and the menu */
enum {
    S_TODAY, S_WIND, S_HUMIDITY, S_NO_WEATHER, S_NO_WEATHER_HINT, S_SETTINGS,
    S_FACE, S_CHANGE, S_COLOR, S_SECONDS, S_DATE, S_WEATHER, S_H24, S_TITLE, S_PLACE, S_LANGUAGE, S_CLOSE,
    S_ON, S_OFF, S_WITH_TRACK, S_EVERY_MIN, S_LOCAL, S_SEARCH, S_SEARCHING, S_NOTHING_FOUND, S_DELETE,
    S_SPACE, S_GO, S_BACK_HINT, S_MENU_HINT,
    S_RANDOM, S_AT_START, S_ALWAYS, S_CLOCK_COLOR, S_DATE_COLOR, S_WX_COLOR, S_WX_LINE, S_REPORT, S_REPORT_LEN,
    S_SEC, S_WX_STYLE, S_WX_COLORS, S_COLORFUL, S_ONE_COLOR, S_AFTER_REPORT, S_FEELS, S_MINMAX, S_RAIN,
    S_SUNRISE, S_SUNSET, S_NOW, S_SWITCH, S_PRESS_BUTTON, S_LEARN_HINT, S_SMOOTH, S_TICK, NSTR
};
static const char *const L_STR[NLANG][NSTR] = {
    { "TODAY", "WIND", "HUMIDITY", "NO WEATHER", "CHOOSE A PLACE IN THE MENU", "CLOCK SETTINGS",
      "FACE", "CHANGE FACE", "COLOR", "SECONDS", "DATE", "WEATHER", "24 HOURS", "TRACK TITLE", "PLACE", "LANGUAGE",
      "CLOSE", "ON", "OFF", "WITH EACH TRACK", "EVERY %d MIN", "LOCAL TIME", "SEARCH...", "SEARCHING...",
      "NOTHING FOUND", "DEL", "SPACE", "SEARCH", "BACK: CLOSE", "F2 / TAB: MENU",
      "RANDOM FACE", "AT START", "ALWAYS", "CLOCK COLOR", "DATE COLOR", "WEATHER COLOR", "WEATHER LINE",
      "WEATHER REPORT", "REPORT LENGTH", "%d SEC", "WEATHER STYLE", "WEATHER COLORS", "COLORFUL", "ONE COLOR",
      "AFTER WEATHER REPORT", "FEELS LIKE", "MIN / MAX", "RAIN", "SUNRISE", "SUNSET", "NOW",
      "SWITCH FACE WITH", "PRESS A BUTTON", "F2 / TAB: CANCEL", "SMOOTH", "TICK" },
    { "HEUTE", "WIND", "FEUCHTE", "KEIN WETTER", "ORT IM MENUE WAEHLEN", "UHR EINSTELLEN",
      "ZIFFERBLATT", "WECHSELN", "FARBE", "SEKUNDEN", "DATUM", "WETTER", "24 STUNDEN", "TRACKTITEL", "ORT", "SPRACHE",
      "SCHLIESSEN", "AN", "AUS", "MIT JEDEM TRACK", "ALLE %d MIN", "ORTSZEIT", "SUCHEN...", "SUCHE...",
      "NICHTS GEFUNDEN", "LOESCHEN", "LEER", "SUCHEN", "ZURUECK: SCHLIESSEN", "F2 / TAB: MENUE",
      "ZUFAELLIG", "BEIM START", "IMMER", "FARBE UHR", "FARBE DATUM", "FARBE WETTER", "WETTERZEILE",
      "WETTERBERICHT", "DAUER BERICHT", "%d SEK", "WETTER-ANSICHT", "WETTERFARBEN", "BUNT", "EINFARBIG",
      "NACH WETTERBERICHT", "GEFUEHLT", "MIN / MAX", "REGEN", "SONNENAUFGANG", "SONNENUNTERGANG", "JETZT",
      "UMSCHALTEN MIT", "TASTE DRUECKEN", "F2 / TAB: ABBRECHEN", "GLEITEND", "TICKEND" },
    { "AUJ.", "VENT", "HUMIDITE", "PAS DE METEO", "CHOISIR UN LIEU DANS LE MENU", "REGLAGES",
      "CADRAN", "CHANGER", "COULEUR", "SECONDES", "DATE", "METEO", "24 HEURES", "TITRE", "LIEU", "LANGUE",
      "FERMER", "OUI", "NON", "A CHAQUE MORCEAU", "TOUTES LES %d MIN", "HEURE LOCALE", "CHERCHER...",
      "RECHERCHE...", "RIEN TROUVE", "EFFACER", "ESPACE", "CHERCHER", "RETOUR: FERMER", "F2 / TAB: MENU",
      "CADRAN AU HASARD", "AU DEMARRAGE", "TOUJOURS", "COULEUR HORLOGE", "COULEUR DATE", "COULEUR METEO",
      "LIGNE METEO", "BULLETIN METEO", "DUREE BULLETIN", "%d S", "STYLE METEO", "COULEURS METEO", "MULTICOLORE",
      "UNICOLORE", "APRES LE BULLETIN", "RESSENTI", "MIN / MAX", "PLUIE", "LEVER", "COUCHER", "MAINTENANT",
      "SUIVANT AVEC", "APPUYEZ SUR UN BOUTON", "F2 / TAB: ANNULER", "FLUIDE", "SAUTANTE" },
    { "HOY", "VIENTO", "HUMEDAD", "SIN TIEMPO", "ELIGE UN LUGAR EN EL MENU", "AJUSTES",
      "ESFERA", "CAMBIAR", "COLOR", "SEGUNDOS", "FECHA", "TIEMPO", "24 HORAS", "TITULO", "LUGAR", "IDIOMA",
      "CERRAR", "SI", "NO", "CON CADA PISTA", "CADA %d MIN", "HORA LOCAL", "BUSCAR...", "BUSCANDO...",
      "SIN RESULTADOS", "BORRAR", "ESPACIO", "BUSCAR", "ATRAS: CERRAR", "F2 / TAB: MENU",
      "ESFERA AL AZAR", "AL INICIO", "SIEMPRE", "COLOR RELOJ", "COLOR FECHA", "COLOR TIEMPO", "LINEA TIEMPO",
      "PARTE DEL TIEMPO", "DURACION PARTE", "%d S", "ESTILO TIEMPO", "COLORES TIEMPO", "MULTICOLOR", "UN COLOR",
      "TRAS EL PARTE", "SENSACION", "MIN / MAX", "LLUVIA", "AMANECER", "ATARDECER", "AHORA",
      "SIGUIENTE CON", "PULSA UN BOTON", "F2 / TAB: CANCELAR", "CONTINUO", "A SALTOS" },
};

/* what steps to the next face (settings.h SW_*) */
static const char *const L_SWITCH[NLANG][4] = {
    { "SPINNER", "LEFT / RIGHT", "CUSTOM", "NONE" },
    { "SPINNER", "LINKS / RECHTS", "EIGENE TASTE", "KEINE" },
    { "SPINNER", "GAUCHE / DROITE", "TOUCHE PERSO", "AUCUN" },
    { "SPINNER", "IZQ. / DER.", "BOTON PROPIO", "NINGUNO" },
};

/* the colors: the faces' own, or all in one */
#define NTHEME 10
#define THEME_RAINBOW 8
#define THEME_RANDOM 9          /* a plain color at random, a new one with each face */
static const char *const L_THEME[NLANG][NTHEME] = {
    { "ORIGINAL", "CYAN", "GREEN", "AMBER", "RED", "MAGENTA", "BLUE", "WHITE", "RAINBOW", "RANDOM" },
    { "ORIGINAL", "CYAN", "GRUEN", "BERNSTEIN", "ROT", "MAGENTA", "BLAU", "WEISS", "REGENBOGEN", "ZUFALL" },
    { "ORIGINAL", "CYAN", "VERT", "AMBRE", "ROUGE", "MAGENTA", "BLEU", "BLANC", "ARC-EN-CIEL", "ALEATOIRE" },
    { "ORIGINAL", "CIAN", "VERDE", "AMBAR", "ROJO", "MAGENTA", "AZUL", "BLANCO", "ARCOIRIS", "ALEATORIO" },
};
static const float THEME_RGB[NTHEME][3] = {
    { 0, 0, 0 }, { 60, 235, 255 }, { 80, 255, 120 }, { 255, 175, 50 }, { 255, 70, 60 },
    { 255, 80, 220 }, { 90, 130, 255 }, { 235, 240, 255 }, { 0, 0, 0 }, { 0, 0, 0 },
};

/* changing faces by itself: off, with each track, every so many minutes,
 * after each weather report */
#define NCHANGE 7
#define CHANGE_AFTER_REPORT 6
static const int CHANGE_MIN[NCHANGE] = { 0, 0, 1, 5, 15, 60, 0 };

/* the weather report: off or every so many minutes, for so many seconds */
#define NREPORT_LEN 4
static const int REPORT_LEN_S[NREPORT_LEN] = { 10, 20, 30, 60 };

/* how the weather is shown */
enum { WS_OVERVIEW, WS_SIMPLE, WS_TODAY, WS_DAYS, WS_HOURS, WS_ROTATE, NWSTYLE };
static const char *const L_WSTYLE[NLANG][NWSTYLE] = {
    { "OVERVIEW", "SIMPLE", "TODAY", "DAYS", "HOURS", "ROTATING" },
    { "UEBERSICHT", "EINFACH", "HEUTE", "TAGE", "STUNDEN", "WECHSELND" },
    { "APERCU", "SIMPLE", "JOUR", "JOURS", "HEURES", "EN ALTERNANCE" },
    { "RESUMEN", "SIMPLE", "HOY", "DIAS", "HORAS", "ALTERNANDO" },
};

/* where the wind comes from, in eighths from the north */
static const char *const L_WIND_DIR[NLANG][8] = {
    { "N", "NE", "E", "SE", "S", "SW", "W", "NW" },
    { "N", "NO", "O", "SO", "S", "SW", "W", "NW" },
    { "N", "NE", "E", "SE", "S", "SO", "O", "NO" },
    { "N", "NE", "E", "SE", "S", "SO", "O", "NO" },
};

#endif
