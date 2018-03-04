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

#include <assert.h>
#include <SDL.h>

#include "clock.h"
#include "misc/list.h"
#include "misc/exit.h"

typedef struct {
    int *lkptable;
    int frequency;
    int index;
    Uint32 ticks;
} sclock;

Uint32 _pause;
glist *_clocks;

static void clock_free()
{
    assert(lcount(_clocks)==0);
    ldelete(_clocks);
}

void clock_init()
{
    _clocks = lcreate();
    pushexit(clock_free);
}

sdlclock clock_new(int frequency)
{
    sclock *c = malloc(sizeof(sclock));

    c->lkptable = malloc(sizeof(int) * frequency);
    c->frequency = frequency;
    c->index = 0;
    c->ticks = SDL_GetTicks();

    int i;
    for(i=0;i<frequency;i++)
        c->lkptable[i] = ((1000*(i+1)) / frequency) - ((1000*i) / frequency);

#ifdef DEBUG
    int s = 0;
    for(i=0;i<frequency;i++) s += c->lkptable[i];
    assert(s==1000);
#endif

    ladd(_clocks, c);

    return (sdlclock)c;
}


void clock_release(sdlclock c)
{
    if(c==NULL) return;

    sclock *_c = (sclock*)c;

    lremove(_clocks, _c);

    free(_c->lkptable);
    free(_c);
}


void clock_step(sdlclock c)
{
    sclock *_c = (sclock*)c;

    _c->ticks += _c->lkptable[_c->index];
    if(++_c->index>=_c->frequency) _c->index = 0;
}


int clock_elapsed(sdlclock c)
{
    return SDL_GetTicks()>=((sclock*)c)->ticks;
}


#define DELAY   2

void clock_wait(sdlclock c)
{
    sclock *_c = (sclock*)c;

    Uint32 t = SDL_GetTicks();
    if(_c->ticks>(t+DELAY)) {
        SDL_Delay(_c->ticks-(t+DELAY));
    }

    while(!clock_elapsed(c));
}


void clock_reset()
{
    Uint32 i, t = SDL_GetTicks();
    int count;

    count = lcount(_clocks);
    for(i=0;i<count;i++)
        ((sclock*)lget(_clocks, i))->ticks = t;
}


void clock_pause(int enable)
{
    Uint32 i, t;
    int count;

    if(enable) {
        _pause = SDL_GetTicks();
    } else {
        t = SDL_GetTicks() - _pause;
        count = lcount(_clocks);
        for(i=0;i<count;i++)
            ((sclock*)lget(_clocks, i))->ticks += t;
    }
}

