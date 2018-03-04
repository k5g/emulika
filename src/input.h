#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <SDL2/SDL.h>

#include "emul.h"
#include "misc/string.h"

typedef enum {
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_A,
    BTN_B,
    BTN_START
} t_button;

typedef struct {
    const char *name;
    int buttons;
    int axis;
    int hats;
} padinfos;

typedef enum {
    INP_KEYBOARD,
    INP_JOYPAD
} t_inputprof;

typedef union {
    struct {
        string name;
        t_inputprof type;
    } base;
    struct {
        string name;
        t_inputprof type;
        SDL_Scancode up, down, left, right, btn_a, btn_b, btn_start;
    } kbd;
    struct {
        string name;
        t_inputprof type;
        int index;
        byte up, down, left, right, btn_a, btn_b, btn_start;
    } pad;
} iprofile;

void input_init(void);

void input_process_event(const SDL_Event *event);

int input_key_pressed(SDL_Scancode k);
int input_key_down(SDL_Scancode k);
int input_key_up(SDL_Scancode k);

int input_pad_detected(void);
int input_pad_getinfos(int pad, padinfos *infos);

int input_button_pressed(const iprofile *profile, t_button btn);
int input_button_down(const iprofile *profile, t_button btn);

void input_loaddefaultprofile(iprofile **p1, iprofile **p2);

int input_setprofile(iprofile *profile);
void input_freeprofile(iprofile *profile);

#endif // INPUT_H_INCLUDED
