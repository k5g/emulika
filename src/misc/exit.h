#ifndef EXIT_H_INCLUDED
#define EXIT_H_INCLUDED


void pushexit(void (*callback)(void));
void popexit(void);

#define pushobject(o,cb)    _pushobject(o,(void (*)(void*))cb)
void _pushobject(void *object, void (*callback)(void*));
void releaseobject(void *object);

#endif // EXIT_H_INCLUDED
