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
    sword *axis;
    kstat *buttons;
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
            statuspads[i].axis = calloc(SDL_JoystickNumAxes(j), sizeof(sword));
            statuspads[i].buttons = calloc(SDL_JoystickNumButtons(j), sizeof(byte));
        } else {
            SDL_JoystickClose(j);
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
            statuspads[event->jbutton.which].buttons[event->jbutton.button].pressed = 1;
            statuspads[event->jbutton.which].buttons[event->jbutton.button].down = 1;
            statuspads[event->jbutton.which].buttons[event->jbutton.button].up = 0;
            log4me_debug(LOG_EMU_JOYSTICK, "J%d button %d down\n", event->jbutton.which, event->jbutton.button);
            break;

        case SDL_JOYBUTTONUP:
            statuspads[event->jbutton.which].buttons[event->jbutton.button].pressed = 0;
            statuspads[event->jbutton.which].buttons[event->jbutton.button].down = 0;
            statuspads[event->jbutton.which].buttons[event->jbutton.button].up = 1;
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

int input_pad_button_pressed(int pad, int button)
{
    assert(pad<SDL_NumJoysticks());
    assert(button<SDL_JoystickNumButtons(statuspads[pad].joy));
    statuspads[pad].buttons[button].down = 0;
    statuspads[pad].buttons[button].up = 0;
    return statuspads[pad].buttons[button].pressed;
}

int input_pad_button_down(int pad, int button)
{
    assert(pad<SDL_NumJoysticks());
    assert(button<SDL_JoystickNumButtons(statuspads[pad].joy));
    int r = statuspads[pad].buttons[button].down;
    statuspads[pad].buttons[button].down = 0;
    return r;
}

int input_pad_button_up(int pad, int button)
{
    assert(pad<SDL_NumJoysticks());
    assert(button<SDL_JoystickNumButtons(statuspads[pad].joy));
    int r = statuspads[pad].buttons[button].up;
    statuspads[pad].buttons[button].up = 0;
    return r;
}

int input_pad_axis_pressed(int pad, byte axis)
{
    assert(pad<SDL_NumJoysticks());

    return (axis & 0xF0) ? (statuspads[pad].axis[axis&0x0F]<=-8192) : (statuspads[pad].axis[axis&0x0F]>=8192);
}

int input_pad_getinfos(int pad, padinfos *infos)
{
    assert(pad<SDL_NumJoysticks());

    if(statuspads[pad].joy==NULL)
        return 0;

    infos->name = SDL_JoystickNameForIndex(pad);
    infos->buttons = SDL_JoystickNumButtons(statuspads[pad].joy);
    infos->axis = SDL_JoystickNumAxes(statuspads[pad].joy);
    infos->hats = SDL_JoystickNumBalls(statuspads[pad].joy);
    return 1;
}

int input_button_pressed(const iprofile *profile, t_button btn)
{
    switch(btn) {
        case BTN_UP:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_pressed(profile->kbd.up);
                case INP_JOYPAD:
                    return input_pad_axis_pressed(profile->pad.index, profile->pad.up);
            }
            break;
        case BTN_DOWN:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_pressed(profile->kbd.down);
                case INP_JOYPAD:
                    return input_pad_axis_pressed(profile->pad.index, profile->pad.down);
            }
            break;
        case BTN_LEFT:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_pressed(profile->kbd.left);
                case INP_JOYPAD:
                    return input_pad_axis_pressed(profile->pad.index, profile->pad.left);
            }
            break;
        case BTN_RIGHT:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_pressed(profile->kbd.right);
                case INP_JOYPAD:
                    return input_pad_axis_pressed(profile->pad.index, profile->pad.right);
            }
            break;
        case BTN_A:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_pressed(profile->kbd.btn_a);
                case INP_JOYPAD:
                    return input_pad_button_pressed(profile->pad.index, profile->pad.btn_a);
            }
            break;
        case BTN_B:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_pressed(profile->kbd.btn_b);
                case INP_JOYPAD:
                    return input_pad_button_pressed(profile->pad.index, profile->pad.btn_b);
            }
            break;
        case BTN_START:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_pressed(profile->kbd.btn_start);
                case INP_JOYPAD:
                    return input_pad_button_pressed(profile->pad.index, profile->pad.btn_start);
            }
            break;
    }

    return 0;
}

int input_button_down(const iprofile *profile, t_button btn)
{
    switch(btn) {
        case BTN_UP:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_down(profile->kbd.up);
                case INP_JOYPAD:
                    return 0;
            }
            break;
        case BTN_DOWN:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_down(profile->kbd.down);
                case INP_JOYPAD:
                    return 0;
            }
            break;
        case BTN_LEFT:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_down(profile->kbd.left);
                case INP_JOYPAD:
                    return 0;
            }
            break;
        case BTN_RIGHT:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_down(profile->kbd.right);
                case INP_JOYPAD:
                    return 0;
            }
            break;
        case BTN_A:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_down(profile->kbd.btn_a);
                case INP_JOYPAD:
                    return input_pad_button_down(profile->pad.index, profile->pad.btn_a);
            }
            break;
        case BTN_B:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_down(profile->kbd.btn_b);
                case INP_JOYPAD:
                    return input_pad_button_down(profile->pad.index, profile->pad.btn_b);
            }
            break;
        case BTN_START:
            switch(profile->base.type) {
                case INP_KEYBOARD:
                    return input_key_down(profile->kbd.btn_start);
                case INP_JOYPAD:
                    return input_pad_button_down(profile->pad.index, profile->pad.btn_start);
            }
            break;
    }

    return 0;
}

void input_loaddefaultprofile(iprofile **p1, iprofile **p2)
{
    if(p1!=NULL) {
        *p1 = (iprofile*)calloc(1, sizeof(iprofile));
        (*p1)->kbd.type = INP_KEYBOARD;
        (*p1)->kbd.name = strcrec("Player 1 default keyboard");
        (*p1)->kbd.up        = SDL_SCANCODE_UP;
        (*p1)->kbd.down      = SDL_SCANCODE_DOWN;
        (*p1)->kbd.left      = SDL_SCANCODE_LEFT;
        (*p1)->kbd.right     = SDL_SCANCODE_RIGHT;
        (*p1)->kbd.btn_a     = SDL_SCANCODE_Z;
        (*p1)->kbd.btn_b     = SDL_SCANCODE_X;
        (*p1)->kbd.btn_start = SDL_SCANCODE_S;
    }

    if(p2!=NULL) {
        *p2 = (iprofile*)calloc(1, sizeof(iprofile));
        (*p2)->kbd.type = INP_KEYBOARD;
        (*p2)->kbd.name = strcrec("Player 2 default keyboard");
        (*p2)->kbd.up        = SDL_SCANCODE_KP_8;
        (*p2)->kbd.down      = SDL_SCANCODE_KP_5;
        (*p2)->kbd.left      = SDL_SCANCODE_KP_4;
        (*p2)->kbd.right     = SDL_SCANCODE_KP_6;
        (*p2)->kbd.btn_a     = SDL_SCANCODE_KP_0;
        (*p2)->kbd.btn_b     = SDL_SCANCODE_KP_PERIOD;
        (*p2)->kbd.btn_start = SDL_SCANCODE_S;
    }
}

int input_setprofile(iprofile *profile)
{
    int i;

    if(profile==NULL)
        return 0;

    switch(profile->base.type) {
        case INP_KEYBOARD:
            return 1;
        case INP_JOYPAD:
            for(i=SDL_NumJoysticks()-1; i>=0; i--) {
                if(strcmp(CSTR(profile->pad.name), SDL_JoystickName(statuspads[i].joy))==0) {
                    profile->pad.index = i;
                    return 1;
                }
            }
            break;
    }

    return 0;
}

void input_freeprofile(iprofile *profile)
{
    if(profile) {
        strfree(profile->base.name);
        free(profile);
    }
}


