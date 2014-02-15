#ifndef EXIT_H_INCLUDED
#define EXIT_H_INCLUDED

#define pushobject(o,cb)    pushvobject(o,(void (*)(void*))cb)

void pushexit(void (*callback)(void));
void popexit(void);

void pushvobject(void *object, void (*callback)(void*));
void releaseobject(void *object);

#endif // EXIT_H_INCLUDED
