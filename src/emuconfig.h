#ifndef EMUCONFIG_H_INCLUDED
#define EMUCONFIG_H_INCLUDED

#include "input.h"
#include "misc/string.h"

typedef struct {
    /* video */
    int fullscreen;
    int noaspectratio;
    float scale;
    string overlayfilename;
    string bezelfilename;
    /* audio */
    int volume;
    int nosound;
} emuconfig;

void initconfig(emuconfig *config);
void readconfig(const string configfilename, emuconfig *config, iprofile **player1, iprofile **player2);
void freeconfig(emuconfig *config);

#endif // EMUCONFIG_H_INCLUDED
