#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED

#include "emul.h"

typedef struct {
    int fullscreen;
    int marginx, marginy;
    float scale;
    SDL_Window *window;
    SDL_Renderer *renderer;
} display;

void video_init(void);
void setvideomode(display *screen);

#endif // VIDEO_H_INCLUDED
