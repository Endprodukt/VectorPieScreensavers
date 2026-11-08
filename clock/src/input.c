/* input.c — see input.h. */
#include "input.h"

#include <stdio.h>
#include <vectorpie/vp.h>

#include "settings.h"

static int learning, learned;

void input_learn(int on) { learning = on; }
int  input_learned(void) { return learned; }

/* the Linux key codes named here (linux/input-event-codes.h), so a PC knows them too */
enum { K_ESC = 1, K_1 = 2, K_2 = 3, K_5 = 6, K_6 = 7, K_TAB = 15, K_ENTER = 28, K_LEFTCTRL = 29, K_Z = 44,
       K_X = 45, K_LEFTSHIFT = 42, K_LEFTALT = 56, K_SPACE = 57, K_F2 = 60, K_UP = 103, K_LEFT = 105,
       K_RIGHT = 106, K_DOWN = 108, K_BTN_LEFT = 0x110, K_BTN_RIGHT = 0x111 };

void input_key_name(int code, char *buf, int size)
{
    /* the cabinet's buttons as the SDK's standard keys have them */
    static const struct { int code; const char *name; } NAME[] = {
        { K_LEFTCTRL, "BUTTON 1" }, { K_LEFTALT, "BUTTON 2" }, { K_SPACE, "BUTTON 3" }, { K_LEFTSHIFT, "BUTTON 4" },
        { K_Z, "BUTTON 5" }, { K_X, "BUTTON 6" }, { K_1, "START 1" }, { K_2, "START 2" }, { K_5, "COIN 1" },
        { K_6, "COIN 2" }, { K_ESC, "EXIT" }, { K_ENTER, "ENTER" }, { K_UP, "UP" }, { K_DOWN, "DOWN" },
        { K_LEFT, "LEFT" }, { K_RIGHT, "RIGHT" }, { K_BTN_LEFT, "MOUSE LEFT" }, { K_BTN_RIGHT, "MOUSE RIGHT" },
    };
    for (size_t i = 0; i < sizeof NAME / sizeof NAME[0]; i++)
        if (NAME[i].code == code) { snprintf(buf, (size_t)size, "%s", NAME[i].name); return; }
    snprintf(buf, (size_t)size, "KEY %d", code);
}

static int key_meaning(int code);

int input_key(int code, int menu)
{
    if (learning) {
        learning = 0;
        if (code == K_F2 || code == K_TAB) return IN_MENU;     /* given up */
        learned = code;
        return IN_LEARNED;
    }
    if (!menu && SET.switch_by == SW_CUSTOM && SET.switch_key > 0 && code == SET.switch_key) return IN_SPIN_R;
    int m = key_meaning(code);
    if (!menu && SET.switch_by == SW_LEFTRIGHT && (m == IN_LEFT || m == IN_RIGHT))
        return m == IN_LEFT ? IN_SPIN_L : IN_SPIN_R;
    /* outside the menu only F2/Tab and START 2 are the clock's: the rest wakes */
    if (!menu && m != IN_MENU && m != IN_START2) m = IN_WAKE;
    if (menu && m == IN_WAKE) return IN_NONE;              /* in the menu nothing wakes */
    return m;
}

#ifdef __linux__

#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MAXDEV 24
static struct {
    int fd;
    int has_abs[ABS_CNT];
    int lo[ABS_CNT], hi[ABS_CNT];
    int way[ABS_CNT];               /* where each axis is: -1, 0, 1 */
    int spinner, spin;              /* a spinner (by its name); its turn so far toward a step */
    double spin_rest;               /* till then (the event's time, s) its turning counts for nothing */
} dev[MAXDEV];
#define SPIN_STEP 240               /* a spinner's counts to one step */
#define SPIN_REST 0.35              /* after a step the spinner rests a while (s): one flick, one step */
static int ndev, grabbed, logging;

#define TEST_BIT(b, a) ((a[(b) / (8 * sizeof(long))] >> ((b) % (8 * sizeof(long)))) & 1)

int input_start(void)
{
    if (!vp_saver_active() || getenv("CLOCK_NO_GRAB")) return 0;
    logging = getenv("CLOCK_INPUT_LOG") != NULL;
    DIR *dir = opendir("/dev/input");
    if (!dir) return 0;
    struct dirent *e;
    while ((e = readdir(dir)) != NULL && ndev < MAXDEV) {
        if (strncmp(e->d_name, "event", 5) != 0) continue;
        char path[300];
        snprintf(path, sizeof path, "/dev/input/%s", e->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;
        unsigned long evbits[(EV_MAX + 8 * sizeof(long)) / (8 * sizeof(long))] = {0};
        ioctl(fd, EVIOCGBIT(0, sizeof evbits), evbits);
        if (!TEST_BIT(EV_KEY, evbits) && !TEST_BIT(EV_ABS, evbits)) { close(fd); continue; }
        /* only the controls: keyboards (the panel's encoder), joysticks,
         * spinners and mice. The power button, the volume keys and HDMI's
         * remote stay the system's. */
        unsigned long keybits[(KEY_MAX + 8 * sizeof(long)) / (8 * sizeof(long))] = {0};
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof keybits), keybits);
        char name[128] = "";
        ioctl(fd, EVIOCGNAME(sizeof name), name);
        int control = TEST_BIT(KEY_1, keybits) || TEST_BIT(KEY_LEFTCTRL, keybits) || TEST_BIT(BTN_LEFT, keybits)
                   || TEST_BIT(BTN_TRIGGER, keybits) || TEST_BIT(BTN_SOUTH, keybits) || TEST_BIT(EV_ABS, evbits);
        if (!control || strncmp(name, "vc4-hdmi", 8) == 0) {
            if (logging) fprintf(stderr, "clock input: left %s (%s)\n", path, name);
            close(fd);
            continue;
        }
        if (ioctl(fd, EVIOCGRAB, 1) != 0) { close(fd); continue; }
        memset(&dev[ndev], 0, sizeof dev[ndev]);
        dev[ndev].fd = fd;
        for (const char *c = name; *c && !dev[ndev].spinner; c++)    /* "SpinTrak", "spinner" ... */
            dev[ndev].spinner = (c[0] | 32) == 's' && (c[1] | 32) == 'p' && (c[2] | 32) == 'i' && (c[3] | 32) == 'n';
        if (TEST_BIT(EV_ABS, evbits)) {
            unsigned long absbits[(ABS_MAX + 8 * sizeof(long)) / (8 * sizeof(long))] = {0};
            ioctl(fd, EVIOCGBIT(EV_ABS, sizeof absbits), absbits);
            for (int a = 0; a < ABS_CNT; a++) {
                struct input_absinfo ai;
                if (TEST_BIT(a, absbits) && ioctl(fd, EVIOCGABS(a), &ai) == 0 && ai.maximum > ai.minimum) {
                    dev[ndev].has_abs[a] = 1;
                    dev[ndev].lo[a] = ai.minimum;
                    dev[ndev].hi[a] = ai.maximum;
                }
            }
        }
        if (logging) fprintf(stderr, "clock input: grabbed %s (%s)%s\n", path, name, dev[ndev].spinner ? " spinner" : "");
        ndev++;
    }
    closedir(dir);
    grabbed = ndev > 0;
    return grabbed;
}

void input_release(void)
{
    for (int i = 0; i < ndev; i++) {
        ioctl(dev[i].fd, EVIOCGRAB, 0);
        close(dev[i].fd);
    }
    ndev = 0;
    grabbed = 0;
}

/* a key or button: what it is to the clock */
static int key_meaning(int code)
{
    switch (code) {
    case KEY_UP: case KEY_KP8: case BTN_DPAD_UP: return IN_UP;
    case KEY_DOWN: case KEY_KP2: case BTN_DPAD_DOWN: return IN_DOWN;
    case KEY_LEFT: case KEY_KP4: case BTN_DPAD_LEFT: return IN_LEFT;
    case KEY_RIGHT: case KEY_KP6: case BTN_DPAD_RIGHT: return IN_RIGHT;
    case KEY_1: case BTN_START: return IN_START1;
    case KEY_2: case BTN_SELECT: return IN_START2;
    case KEY_F2: case KEY_TAB: return IN_MENU;             /* the clock's menu */
    /* the first buttons (MAME's standard keys, joystick buttons): OK */
    case KEY_ENTER: case KEY_KPENTER: case KEY_LEFTCTRL: case BTN_SOUTH: case BTN_TRIGGER: case BTN_0:
        return IN_OK;
    /* the second: back */
    case KEY_ESC: case KEY_BACKSPACE: case KEY_LEFTALT: case BTN_EAST: case BTN_THUMB: case BTN_1:
        return IN_BACK;
    }
    return IN_WAKE;
}

int input_next(int menu)
{
    if (!grabbed) return IN_NONE;
    struct input_event ev;
    for (int i = 0; i < ndev; i++) {
        while (read(dev[i].fd, &ev, sizeof ev) == (ssize_t)sizeof ev) {
            if (logging) fprintf(stderr, "clock input: dev %d type %d code %d value %d\n", i, ev.type, ev.code, ev.value);
            if (ev.type == EV_KEY) {
                if (ev.value == 0) continue;                        /* let go */
                if (ev.value == 2) {                                /* held: only to move in the menu */
                    int m = key_meaning(ev.code);
                    if (menu && !learning && m >= IN_UP && m <= IN_RIGHT) return m;
                    continue;
                }
                int m = input_key(ev.code, menu);
                if (m == IN_NONE) continue;
                return m;
            }
            if (ev.type == EV_REL && dev[i].spinner && (ev.code == REL_X || ev.code == REL_DIAL)) {
                if (!menu && SET.switch_by != SW_SPINNER) continue; /* the menu says: not the spinner */
                double t = ev.time.tv_sec + ev.time.tv_usec / 1e6;
                if (t < dev[i].spin_rest) continue;
                if ((ev.value > 0) != (dev[i].spin > 0)) dev[i].spin = 0;   /* turned back: start again */
                dev[i].spin += ev.value;
                if (dev[i].spin >= SPIN_STEP || dev[i].spin <= -SPIN_STEP) {
                    int right = dev[i].spin > 0;
                    dev[i].spin = 0;
                    dev[i].spin_rest = t + SPIN_REST;
                    return right ? (menu ? IN_DOWN : IN_SPIN_R) : (menu ? IN_UP : IN_SPIN_L);
                }
                continue;
            }
            if (ev.type == EV_ABS && ev.code < ABS_CNT && dev[i].has_abs[ev.code]) {
                int a = ev.code;
                int way = 0;
                if (a == ABS_HAT0X || a == ABS_HAT0Y) {
                    way = ev.value < 0 ? -1 : ev.value > 0 ? 1 : 0;
                } else if (a == ABS_X || a == ABS_Y) {
                    /* a stick: past three quarters of its way (no drift), only in the menu */
                    float mid = (dev[i].lo[a] + dev[i].hi[a]) / 2.0f, half = (dev[i].hi[a] - dev[i].lo[a]) / 2.0f;
                    float f = (ev.value - mid) / half;
                    way = f < -0.75f ? -1 : f > 0.75f ? 1 : (f > -0.4f && f < 0.4f) ? 0 : dev[i].way[a];
                    if (!menu && !(a == ABS_X && SET.switch_by == SW_LEFTRIGHT)) { dev[i].way[a] = way; continue; }
                } else {
                    continue;
                }
                if (way == dev[i].way[a]) continue;
                dev[i].way[a] = way;
                if (way == 0) continue;
                int x = a == ABS_HAT0X || a == ABS_X;
                int m = x ? (way < 0 ? IN_LEFT : IN_RIGHT) : (way < 0 ? IN_UP : IN_DOWN);
                if (!menu && x && SET.switch_by == SW_LEFTRIGHT) return way < 0 ? IN_SPIN_L : IN_SPIN_R;
                return menu ? m : IN_WAKE;
            }
        }
    }
    return IN_NONE;
}

#else   /* not Linux: a PC, nothing to take over */

static int key_meaning(int code)
{
    switch (code) {
    case K_UP: return IN_UP;
    case K_DOWN: return IN_DOWN;
    case K_LEFT: return IN_LEFT;
    case K_RIGHT: return IN_RIGHT;
    case K_1: return IN_START1;
    case K_2: return IN_START2;
    case K_F2: case K_TAB: return IN_MENU;
    case K_ENTER: case K_LEFTCTRL: return IN_OK;
    case K_ESC: case K_LEFTALT: return IN_BACK;
    }
    return IN_WAKE;
}

int  input_start(void) { return 0; }
int  input_next(int menu) { (void)menu; return IN_NONE; }
void input_release(void) {}

#endif
