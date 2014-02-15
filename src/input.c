/************************************************************************

    Copyright 2013-2014 Xavier PINEAU

    This file is part of Emulika.

    Emulika is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Emulika is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Emulika.  If not, see <http://www.gnu.org/licenses/>.

************************************************************************/

#include "input.h"
#include "misc/exit.h"

typedef struct {
    byte pressed : 1;
    byte down : 1;
    byte up : 1;
} kstat;

typedef struct {
    SDL_Joystick *joy;
    int available;
    sword *axis;
    byte *buttons;
} jstat;

kstat *statuskeys = NULL;
jstat *statuspads = NULL;

void input_free()
{
    free(statuskeys);

    int i;
    for(i=SDL_NumJoysticks()-1; i>=0; i--) {
        free(statuspads[i].buttons);
        free(statuspads[i].axis);
        if(statuspads[i].joy!=NULL)
            SDL_JoystickClose(statuspads[i].joy);
    }
    free(statuspads);
}

void input_init()
{
    assert(SDL_WasInit(SDL_INIT_JOYSTICK)==SDL_INIT_JOYSTICK);

    statuskeys = calloc(SDL_NUM_SCANCODES, sizeof(kstat));

    int i, njoysticks;

    if((njoysticks=SDL_NumJoysticks())==0) return;
    SDL_JoystickEventState(SDL_ENABLE);

    statuspads = calloc(njoysticks, sizeof(jstat));
    for(i=0; i<njoysticks; i++) {
        SDL_Joystick *j = SDL_JoystickOpen(i);
        if((SDL_JoystickNumAxes(j)<8) && (SDL_JoystickNumButtons(j)<20)) {
            statuspads[i].joy = j;
            statuspads[i].available = 1;
            statuspads[i].axis = calloc(SDL_JoystickNumAxes(j), sizeof(sword));
            statuspads[i].buttons = calloc(SDL_JoystickNumButtons(j), sizeof(byte));
        } else {
            SDL_JoystickClose(j);
            assert(statuspads[i].available==0);
        }
    }

    pushexit(input_free);
}

void input_process_event(const SDL_Event *event)
{
    switch(event->type) {
        case SDL_KEYDOWN:
            assert(event->key.keysym.scancode<SDL_NUM_SCANCODES);
            if(!event->key.repeat) {
                statuskeys[event->key.keysym.scancode].pressed = 1;
                statuskeys[event->key.keysym.scancode].down = 1;
                statuskeys[event->key.keysym.scancode].up = 0;
                //log4me_warning("[KEY] %d press => pressed : %d / down : %d / up : %d\n", event->key.keysym.scancode, statuskeys[event->key.keysym.scancode].pressed, statuskeys[event->key.keysym.scancode].down, statuskeys[event->key.keysym.scancode].up);
            }
            break;

        case SDL_KEYUP:
            assert(event->key.keysym.scancode<SDL_NUM_SCANCODES);
            if(!event->key.repeat) {
                statuskeys[event->key.keysym.scancode].pressed = 0;
                statuskeys[event->key.keysym.scancode].down = 0;
                statuskeys[event->key.keysym.scancode].up = 1;
                //log4me_warning("[KEY] %d release => pressed : %d / down : %d / up : %d\n", event->key.keysym.scancode, statuskeys[event->key.keysym.scancode].pressed, statuskeys[event->key.keysym.scancode].down, statuskeys[event->key.keysym.scancode].up);
            }
            break;

        case SDL_JOYBUTTONDOWN:
            statuspads[event->jbutton.which].buttons[event->jbutton.button] = 1;
            log4me_debug(LOG_EMU_JOYSTICK, "J%d button %d down\n", event->jbutton.which, event->jbutton.button);
            break;

        case SDL_JOYBUTTONUP:
            statuspads[event->jbutton.which].buttons[event->jbutton.button] = 0;
            log4me_debug(LOG_EMU_JOYSTICK, "J%d button %d up\n", event->jbutton.which, event->jbutton.button);
            break;

        case SDL_JOYAXISMOTION:
            statuspads[event->jaxis.which].axis[event->jaxis.axis] =  event->jaxis.value;
            log4me_debug(LOG_EMU_JOYSTICK, "J%d axis %d = %d\n", event->jaxis.which, event->jaxis.axis, event->jaxis.value);
            break;
    }
}

int input_key_pressed(SDL_Scancode k)
{
    assert(k<SDL_NUM_SCANCODES);
    statuskeys[k].down = 0;
    statuskeys[k].up = 0;
    //log4me_warning("[KEY] %d = %d\n", k, statuskeys[k].pressed);
    return statuskeys[k].pressed;
}

int input_key_down(SDL_Scancode k)
{
    assert(k<SDL_NUM_SCANCODES);
    int r = statuskeys[k].down;
    statuskeys[k].down = 0;
    //log4me_warning("[KEY] %d down = %d\n", k, r);
    return r;
}

int input_key_up(SDL_Scancode k)
{
    assert(k<SDL_NUM_SCANCODES);
    int r = statuskeys[k].up;
    statuskeys[k].up = 0;
    //log4me_warning("[KEY] %d up = %d\n", k, r);
    return r;
}

int input_pad_detected()
{
    int i;
    for(i=SDL_NumJoysticks()-1; i>=0; i--) {
        if(statuspads[i].joy!=NULL) return 1;
    }
    return 0;
}

int input_new_pad()
{
    int i;
    for(i=0; i<SDL_NumJoysticks(); i++) {
        if(statuspads[i].available) {
            statuspads[i].available = 0;
            return i;
        }
    }
    return NO_JOYPAD;
}

void input_release_pad(int pad)
{
    if(pad==NO_JOYPAD) return;

    assert(pad<SDL_NumJoysticks());
    assert(statuspads[pad].available==0);

    statuspads[pad].available = 1;
}

int input_pad_button_pressed(int pad, int button)
{
    assert(pad<SDL_NumJoysticks());
    assert(button<SDL_JoystickNumButtons(statuspads[pad].joy));
    return statuspads[pad].buttons[button];
}

int input_pad_axis_pressed(int pad, t_axis axis)
{
    assert(pad<SDL_NumJoysticks());
    switch(axis) {
        case AX_LEFT:
            return statuspads[pad].axis[0]<=-8192;
        case AX_RIGHT:
            return statuspads[pad].axis[0]>= 8192;
        case AX_UP:
            return statuspads[pad].axis[1]<=-8192;
        case AX_DOWN:
            return statuspads[pad].axis[1]>= 8192;
        default:
            assert(0);
            break;
    }
    return 0;
}

void input_pad_getinfos(int pad, padinfos *infos)
{
    assert(pad<SDL_NumJoysticks());
    assert(statuspads[pad].joy);

    infos->name = SDL_JoystickNameForIndex(pad);
    infos->buttons = SDL_JoystickNumButtons(statuspads[pad].joy);
    infos->axis = SDL_JoystickNumAxes(statuspads[pad].joy);
    infos->hats = SDL_JoystickNumBalls(statuspads[pad].joy);
}


