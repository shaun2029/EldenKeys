/*
    Elden Keys - Elden Ring Linux keyboard macros
    Copyright Shaun Simpson 2022
    Released under GPLv2 license.

    Building:
        gcc EldenKeys.c -lX11 -lc -lXmu -o EldenKeys
    
    Elden Ring Key Binds
        1 = ESC
        2 = E+UpArrow
        3 = E+RightArrow
        4 = E+DownArrow
        5 = E+LeftArrow
        CAPS = LeftShift
        
    Launch in terminal like:
        ./EldenKeys /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_056E38673132-event-kbd        
*/
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>    // `apt-get install libxmu-dev`
#include <string.h>
#include <linux/input.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>

#define SKIP_KEY_TIME 2

char targetWindowName[] = "ELDEN RING™";

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

int FakeKeyEventSync(int fd)
{
    struct input_event eventEnd = {0};

    eventEnd.type = EV_SYN;
    eventEnd.code = SYN_REPORT;
    eventEnd.value = 0;
    gettimeofday(&eventEnd.time, NULL);
    size_t stat = write(fd, &eventEnd, sizeof(struct input_event));
    if (stat < 0) {
        printf("Errror write kbd:%s\n", strerror(errno));
        return -1;
    }
    else {
        return 0;
    }
}

/* code = keycode, value = 1 (key down) 2 (key repeat) 0 (key up) */
int FakeKeyEvent(int fd, unsigned short code, unsigned int value)
{
    struct input_event event = {0};

    event.value = value; // key down
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = code; //E
    size_t stat = write(fd, &event, sizeof(struct input_event));
    if (stat < 0) {
        printf("Errror write kbd:%s\n", strerror(errno));
        return -1;
    }
    else {
        return FakeKeyEventSync(fd);
    }
}

int SkipKey(unsigned short code, unsigned short lastCode, 
    struct timeval *lastCodeTime)
{
    struct timeval codeTime, timeDiff;
    int skipCode = 0;

    if (code == lastCode) {
        struct timeval codeTime, timeDiff;

        gettimeofday(&codeTime, NULL);
        timersub(&codeTime, lastCodeTime, &timeDiff); 
        
        if (timeDiff.tv_sec < SKIP_KEY_TIME) {
            skipCode = 1;
        }
    }
    
    return skipCode;
}

/* Returns 1 if key was found. */
int PressOneKey(int fd, unsigned short searchCode, int eventValue,
  unsigned short keyCode1,
  unsigned short inputCode, unsigned short lastCode) 
{
    int skipCode;

    /* Has key been pressed? */
    if ((inputCode == searchCode) && (eventValue == 1)) {
        printf("key detected: %s\n", qwerty_map[searchCode]);

        printf("key press: %s\n", qwerty_map[keyCode1]);

        /* Press key 1 */
        FakeKeyEvent(fd, keyCode1, 1);

        return 1;
    }

    return 0;
}


/* Returns 1 if key was found. */
int ReleaseOneKey(int fd, unsigned short searchCode, int eventValue,
  unsigned short keyCode1,
  unsigned short inputCode, unsigned short lastCode) 
{
    int skipCode;

    /* Has key been released? */
    if ((inputCode == searchCode) && (eventValue == 0)) {
        printf("key detected: %s\n", qwerty_map[searchCode]);

        printf("key released: %s\n", qwerty_map[keyCode1]);

        /* Release key 1 */
        FakeKeyEvent(fd, keyCode1, 0);

        return 1;
    }
    
    return 0;
}

/* Returns 1 if key was found. */
int SendOneKey(int fd, unsigned short searchCode, int eventValue,
  unsigned short keyCode1,
  unsigned short inputCode, unsigned short lastCode, struct timeval *lastCodeTime) 
{
    int skipCode = 0;

    /* Has key been pressed? */
    if ((inputCode == searchCode) && (eventValue == 1)) {
        printf("key detected: %s\n", qwerty_map[searchCode]);

        if (lastCodeTime != NULL) {
            skipCode = SkipKey(inputCode, lastCode, lastCodeTime);
        }

        if (!skipCode) {
            gettimeofday(lastCodeTime, NULL);
            printf("key press: %s\n", qwerty_map[keyCode1]);

            /* Press key 1 */
            FakeKeyEvent(fd, keyCode1, 1);
            usleep(100000);

            /* Release key 1 */
            FakeKeyEvent(fd, keyCode1, 0);
        }
        else {
          printf("key press: skipped, time out %ds\n", SKIP_KEY_TIME); 
        }
        
        return 1;
    }
    
    return 0;
}

/* Returns 1 if key was found. */
int SendTwoKeys(int fd, unsigned short searchCode, int eventValue,
  unsigned short keyCode1, unsigned short keyCode2, 
  unsigned short inputCode, unsigned short lastCode, struct timeval *lastCodeTime) 
{
    int skipCode = 0;

//     printf("key search: %s, value: %d\n", qwerty_map[searchCode], eventValue);

    /* Has key been pressed? */
    if ((inputCode == searchCode) && (eventValue == 1)) {
        printf("key detected: %s\n", qwerty_map[searchCode]);

        if (lastCodeTime != NULL) {
            skipCode = SkipKey(inputCode, lastCode, lastCodeTime);
        }

        if (!skipCode) {
            gettimeofday(lastCodeTime, NULL);
            printf("key press: %s + %s\n", qwerty_map[keyCode1], qwerty_map[keyCode2]);

            /* Press key 1 */
            FakeKeyEvent(fd, keyCode1, 1);
            usleep(300000);


            /* Press key 2 */
            FakeKeyEvent(fd, keyCode2, 1);
            usleep(100000);

            /* Release key 2 */
            FakeKeyEvent(fd, keyCode2, 0);
            usleep(100000);

            /* Release key 1 */
            FakeKeyEvent(fd, keyCode1, 0);
        }
        else {
          printf("key press: skipped, time out %ds\n", SKIP_KEY_TIME); 
        }
        return 1;
    }
    
    return 0;
}

// (XFetchName cannot get a name with multi-byte chars)
int TargetWindowFocused(Display* d, Window w){
  XTextProperty prop;
  Status s;
  int found = 0;

  //printf("window name:\n");

  s = XGetWMName(d, w, &prop); // see man
  if(s){
    int count = 0, result;
    char **list = NULL;
    result = XmbTextPropertyToTextList(d, &prop, &list, &count); // see man
    if(result == Success){
        //printf("\t%s\n", list[0]);
        if (strcmp(list[0], targetWindowName) == 0) {
            found = 1;        
            //printf("Target Focused: %s\n", targetWindowName);
        }
    }else{
        //printf("ERROR: XmbTextPropertyToTextList\n");
    }
  }else{
        //printf("ERROR: XGetWMName\n");
  }
  
  return found;
}

Window get_top_window(Display* d, Window start){
  Window w = start;
  Window parent = start;
  Window root = None;
  Window *children;
  unsigned int nchildren;
  Status s;

  //printf("getting top window ... \n");
  while (parent != root) {
    w = parent;
    s = XQueryTree(d, w, &root, &parent, &children, &nchildren); // see man

    if (s)
      XFree(children);

    //printf("  get parent (window: %d)\n", (int)w);
  }

  //printf("success (window: %d)\n", (int)w);

  return w;
}

Window get_focus_window(Display* d){
  Window w;
  int revert_to;
  int targetFocused = 0;
  //printf("getting input focus window ... ");
  XGetInputFocus(d, &w, &revert_to); // see man
  
  if(w == None){
    //printf("no focus window\n");
  }else{
    //printf("success (window: %d)\n", (int)w);
    w = get_top_window(d, w);
    targetFocused = TargetWindowFocused(d, w);
  }

  return targetFocused;
}

int main(int argc, char **argv)
{
    Display *display;
    Window root_window;
    struct input_event event;

    printf("EldenKeys v1.0\n");

    if (argc == 2) {
      printf("Using input event device: %s\n", argv[1]);
    }
    else {
      printf("Usage: %s dev_keyboard_event]\n", argv[0]);
      printf("e.g.: %s /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_056E38673132-event-kbd\n", argv[0]);
      return 0;
    }

    display = XOpenDisplay(0);
    if (!display) {
        printf("Errror opening X11 display!\n");
        return -2;
    }
    
    root_window = DefaultRootWindow(display);
    
    // for XmbTextPropertyToTextList
    setlocale(LC_ALL, ""); // see man locale    

    int fd = open(argv[1], O_RDWR);
    size_t stat;
    if (fd < 0) {
        printf("Errro open mouse:%s\n", strerror(errno));
        return -1;
    }
  
    unsigned short last_code = 0;
    struct timeval last_code_time, lastTargetTime, targetTime, timeDiff;
    
    int targetFocused = get_focus_window(display);
    gettimeofday(&lastTargetTime, NULL);
    
    while (1) {
        int bytes = read(fd, &event, sizeof(struct input_event));
        
        gettimeofday(&targetTime, NULL);
        timersub(&targetTime, &lastTargetTime, &timeDiff); 
        
        if (timeDiff.tv_sec > 2) {
            targetFocused = get_focus_window(display);
            gettimeofday(&lastTargetTime, NULL);
        }

        if (bytes == sizeof(struct input_event) && (event.type == EV_KEY)) {
            if (targetFocused) { 
                //printf("key: %s, code: %d, type: %d, value: %d\n", qwerty_map[event.code], event.code, event.type, event.value);//xcb_key_symbols_get_keysym(symbols, event.code, 0)));
#ifdef SHAUN            
                /* One released - Press ESC */
                if (SendTwoKeys(fd, 2, event.value, 18, 10, event.code, last_code, &last_code_time)) {
                    last_code = event.code; 
                }
                /* Two released - press E + 7 */
                else if (SendTwoKeys(fd, 3, event.value, 18, 8, event.code, last_code, &last_code_time)) {
                    last_code = event.code; 
                }
                /* Three released - press E + DOWN */
                else if (SendTwoKeys(fd, 4, event.value, 18, 108, event.code, last_code, &last_code_time)) {
                    last_code = event.code; 
                }
                /* Four released - press E + LEFT */
                else if (SendTwoKeys(fd, 5, event.value, 18, 105, event.code, last_code, &last_code_time)) {
                    last_code = event.code; 
                }
#else
                /* One released - Press ESC */
                if (SendOneKey(fd, 2, event.value, 1, event.code, last_code, NULL)) {
                    last_code = event.code; 
                }
                /* Two released - press E + UP */
                else if (SendTwoKeys(fd, 3, event.value, 18, 103, event.code, last_code, &last_code_time)) {
                    last_code = event.code; 
                }
                /* Three released - press E + RIGHT */
                else if (SendTwoKeys(fd, 4, event.value, 18, 106, event.code, last_code, &last_code_time)) {
                    last_code = event.code; 
                }
                /* Four released - press E + DOWN */
                else if (SendTwoKeys(fd, 5, event.value, 18, 108, event.code, last_code, &last_code_time)) {
                    last_code = event.code; 
                }
                /* Five released - press E + LEFT */
                else if (SendTwoKeys(fd, 6, event.value, 18, 105, event.code, last_code, &last_code_time)) {
                    last_code = event.code; 
                }
                /* CAPS pressed - Press LSHIFT */
                else if (PressOneKey(fd, 58, event.value, 42, event.code, last_code)) {
                    last_code = event.code; 
                }
                /* CAPS pressed - Press LSHIFT */
                else if (ReleaseOneKey(fd, 58, event.value, 42, event.code, last_code)) {
                    last_code = event.code; 
                }
                /* Eight released - Press UP */
                else if (SendOneKey(fd, 8, event.value, 103, event.code, last_code, NULL)) {
                    last_code = event.code; 
                    gettimeofday(&last_code_time, NULL);
                }
                /* Nine released - Press RIGHT */
                else if (SendOneKey(fd, 10, event.value, 106, event.code, last_code, NULL)) {
                    last_code = event.code; 
                }
#endif
            }
        }
        else {
            usleep(32000);
        }
    }
}

