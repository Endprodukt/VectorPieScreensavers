/* input.c — see input.h. */
#include "input.h"

#include <vectorpie/vp.h>

#ifdef __linux__

#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
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
} dev[MAXDEV];
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
        if (logging) fprintf(stderr, "clock input: grabbed %s (%s)\n", path, name);
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
                int m = key_meaning(ev.code);
                if (ev.value == 2 && !(menu && m >= IN_UP && m <= IN_RIGHT)) continue;   /* held: only to move */
                /* outside the menu only F2/Tab and START 2 are the clock's: the rest wakes */
                if (!menu && m != IN_MENU && m != IN_START2) m = IN_WAKE;
                if (menu && m == IN_WAKE) continue;                 /* in the menu nothing wakes */
                return m;
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
                    if (!menu) { dev[i].way[a] = way; continue; }
                } else {
                    continue;
                }
                if (way == dev[i].way[a]) continue;
                dev[i].way[a] = way;
                if (way == 0) continue;
                int x = a == ABS_HAT0X || a == ABS_X;
                int m = x ? (way < 0 ? IN_LEFT : IN_RIGHT) : (way < 0 ? IN_UP : IN_DOWN);
                return menu ? m : IN_WAKE;
            }
        }
    }
    return IN_NONE;
}

#else   /* not Linux: a PC, nothing to take over */

int  input_start(void) { return 0; }
int  input_next(int menu) { (void)menu; return IN_NONE; }
void input_release(void) {}

#endif
