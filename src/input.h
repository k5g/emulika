#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <SDL2/SDL.h>

#include "emul.h"

#define NO_JOYPAD   (-1)

typedef enum {
    AX_LEFT,
    AX_RIGHT,
    AX_UP,
    AX_DOWN
} t_axis;

typedef struct {
    const char *name;
    int buttons;
    int axis;
    int hats;
} padinfos;

void input_init(void);

void input_process_event(const SDL_Event *event);

int input_key_pressed(SDL_Scancode k);
int input_key_down(SDL_Scancode k);
int input_key_up(SDL_Scancode k);

int input_pad_detected(void);
int input_new_pad(void);
void input_release_pad(int pad);
int input_pad_button_pressed(int pad, int button);
int input_pad_axis_pressed(int pad, t_axis axis);

void input_pad_getinfos(int pad, padinfos *infos);


#endif // INPUT_H_INCLUDED
