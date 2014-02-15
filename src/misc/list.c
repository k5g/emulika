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

#include <stdlib.h>
#include <assert.h>

#include "list.h"

struct _gitem {
    void *item;
    struct _gitem *next;
};
typedef struct _gitem gitem;

struct _glist {
    gitem *head;
};

glist *lcreate()
{
    return (glist*)calloc(1, sizeof(glist));
}

void ldelete(glist *l)
{
    assert(l->head==NULL);
    free(l);
}

void lpush(glist *l, void *item)
{
    gitem *e = (gitem*)calloc(1, sizeof(gitem));
    e->item = item;
    e->next = l->head;
    l->head = e;
}

void *lpop(glist *l)
{
    void *item;
    gitem *e;

    assert(l->head);
    if(l->head==NULL) return NULL;

    e = l->head;
    item = e->item;
    l->head = e->next;
    free(e);

    return item;
}

void ladd(glist *l, void *item)
{
    gitem *e, *le = l->head;

    if(le) {
        while(le->next!=NULL) le = le->next;
    }

    e = (gitem*)calloc(1, sizeof(gitem));
    e->item = item;
    if(le) le->next = e; else l->head = e;
}

void lremove(glist *l, void *item)
{
    gitem *e = l->head;
    gitem *p = NULL;

    while((e!=NULL) && (e->item!=item)) {
        p = e;
        e = e->next;
    }

    if(e) {
        if(p) p->next = e->next; else l->head = e->next;
        free(e);
    }
}

int lcount(const glist *l)
{
    int i;
    const gitem *e = l->head;

    for(i=0; e!=NULL; i++) e = e->next;
    return i;
}

void *lget(const glist *l, int index)
{
    int i;
    const gitem *e = l->head;

    for(i=0;i<index;i++) {
        e = e->next;
        assert(e);
    }
    return e->item;
}
