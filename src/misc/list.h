#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

struct _glist;
typedef struct _glist glist;

glist *lcreate();
void ldelete(glist *l);

void lpush(glist *l, void *item);
void *lpop(glist *l);

void ladd(glist *l, void *item);
void lremove(glist *l, void *item);

int lcount(const glist *l);
void *lget(const glist *l, int index);

#endif // LIST_H_INCLUDED
