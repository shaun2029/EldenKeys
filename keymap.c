#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <string.h>
#include <linux/input.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

char *qwerty_map[] = {"", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    "0", "-", "=", "BACKSPACE", "TAB", "q", "w",
    "e", "r", "t", "y", "u", "i", "o", "p", "[",
    "]", "ENTER", "CTRL", "a", "s", "d", "f", "g",
    "h", "j", "k", "l", ";", "'", "41", "LSHIFT",
    "#", "z", "x", "c", "v", "b", "n", "m",
    ",", ".", "/", "RSHIFT", "KP_Multiply", "ALT", "SPACE",
    "CAPSLOCK","F1","F2","F3","F4","F5","F6","F7",
    "F8","F9","F10","NUM","SCROLL","KP_7","KP_8","KP_9",
    "KP_-","KP_4","KP_5","KP_6","KP_+","KP_1","KP_2","KP_3",
    "KP_0","KP_.","Last_Console","84","\\","F11","F12",
    "89","90","91","92","93","94","95","KP_ENTER", "CTRL",
    "KP_DIVIDE","99","ALTGR","BREAK","HOME","UP","PGUP",
    "LEFT", "RIGHT","END","DOWN","PGDOWN","INS","DEL","MACRO","F13",
    "F14","HELP","DO","F17","KP_MIN+","PAUSE","120","KP_.",
    "122","123","124","SUPER","126","127","127","129","130",
    "131","132","133","134","135","136","137","138","139",
};

/*
struct input_event {
        struct timeval time;
        unsigned short type;
        unsigned short code;
        unsigned int value;
};
*/

int main(int argc, char **argv)
{
    struct input_event event, event_end;

    if (argc == 2) {
      printf("Using input event device: %s\n", argv[1]);
    }
    else {
      printf("Usage: %s dev_keyboard_event]\n", argv[0]);
      printf("e.g.: %s /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_056E38673132-event-kbd\n", argv[0]);
      return 0;
    }

    int fd = open(argv[1], O_RDWR);
    size_t stat;
    if (fd < 0) {
        printf("Errro open keyboard:%s\n", strerror(errno));
        return -1;
    }
    memset(&event, 0, sizeof(event));
    memset(&event, 0, sizeof(event_end));
    event.type = EV_REL;
    event.code = REL_X;
    event.value = 100;
    event_end.type = EV_SYN;
    event_end.code = SYN_REPORT;
    event_end.value = 0;

    while (1) {
        int bytes = read(fd, &event, sizeof(struct input_event));

        if (bytes == sizeof(struct input_event)) {
            printf("key: %s, code: %d, type: %d, value: %d\n", qwerty_map[event.code], event.code, event.type, event.value);
        }
        else {
            usleep(32000);
        }
    }
}

