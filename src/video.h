#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED

#include "emul.h"

#define DEFAULT_MARGIN  10

typedef struct {
    int fullscreen;
    int width, height;
    int margin, marginx, marginy;
    float scale;
    float minscale;
    SDL_Window *window;
    SDL_Renderer *renderer;
} display;

void video_init(void);
void setvideomode(display *screen);
SDL_Window *getcurrentwindow(void);
const char *getcurrentrendererdriver(void);
const char *getcurrentvideodriver(void);

#endif // VIDEO_H_INCLUDED
