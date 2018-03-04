#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED

#include "emul.h"

#define DEFAULT_MARGIN  10

typedef struct {
    int fullscreen;
    int noaspectratio;
    int width, height;
    int margin;
    float scale;
    float minscale;
} display;

void video_init(void);
void video_event(const SDL_Event *event);
void video_setmode(display *screen);
void displaytexture(const display *screen, SDL_Texture *picture, SDL_PixelFormat *pixelfmt, Uint32 backgroundcolor);
void video_setoverlay(display *screen, const char *filename);
void video_setbezel(display *screen, const char *filename);

SDL_Window *video_getcurrentwindow(void);
SDL_Renderer *video_getrenderer(void);
const char *video_getcurrentrendererdriver(void);
const char *video_getcurrentvideodriver(void);

#endif // VIDEO_H_INCLUDED
