#ifndef YM2413_H_INCLUDED
#define YM2413_H_INCLUDED

#include "emul.h"

typedef struct {
    int data;
} ym2413;

void ym2413_init(ym2413 *cpn);
void ym2413_free();

void ym2413_write(ym2413 *cpn, byte port, byte data);

#endif // YM2413_H_INCLUDED
