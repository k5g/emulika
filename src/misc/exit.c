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

#include <stdio.h>
#include <stdlib.h>

#include "exit.h"
#include "list.h"
#include "../emul.h"

static void initexit (void) __attribute__((constructor));
//void end (void) __attribute__((destructor));

typedef struct {
    void (*callback1)(void);
    void (*callback2)(void*);
    void *object;
} cbexit;

glist *_exitlist = NULL;

static void finalexit()
{
    while(lcount(_exitlist)) popexit();
    ldelete(_exitlist);
}

static void initexit()
{
    _exitlist = lcreate();
    atexit(finalexit);
}

static void callback_and_free(cbexit *el)
{
    assert((el->callback1!=NULL) ^ (el->callback2!=NULL));
    if(el->callback1)
        el->callback1();
    else if(el->callback2)
            el->callback2(el->object);
    free(el);
}

void pushexit(void (*callback)(void))
{
    assert(_exitlist!=NULL);

    cbexit *el = calloc(1, sizeof(cbexit));
    el->callback1 = callback;
    lpush(_exitlist, el);
}

void popexit()
{
    cbexit *el = (cbexit*)lpop(_exitlist); assert(el!=NULL);
    callback_and_free(el);
}

void _pushobject(void *object, void (*callback)(void*))
{
    assert(_exitlist!=NULL);
    assert(object!=NULL);

    cbexit *el = calloc(1, sizeof(cbexit));
    el->callback2 = callback;
    el->object = object;
    lpush(_exitlist, el);
}

void releaseobject(void *object)
{
    if(object==NULL) return;

    int i;
    int count = lcount(_exitlist);

    for(i=0;i<count;i++) {
        cbexit *el = (cbexit*)lget(_exitlist, i);
        if(el->object==object) {
            lremove(_exitlist, el);
            callback_and_free(el);
            return;
        }
    }
}
