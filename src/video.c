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

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "video.h"
#include "misc/exit.h"

#define MARGIN  10
#define DEFAULT_DMODE_W 640
#define DEFAULT_DMODE_H 480

SDL_DisplayMode desktopmode;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int fixedres = 0;
byte vfullscreen = 0;
float vscale = 1;

static void video_free()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void video_init()
{
    int flags = 0;

    pushexit(video_free);

#ifdef RASPBERRYPI
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2"); // Force opengles2 on the raspberry pi plateform
#endif

    if(SDL_GetDesktopDisplayMode(0, &desktopmode)<0) {
        log4me_error(LOG_EMU_SDL, "Unable to get desktop display mode : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    /*int i;
    SDL_DisplayMode mode;
    for(i=0;i<SDL_GetNumDisplayModes(0); i++) {
        SDL_GetDisplayMode(0, i, &mode);
        log4me_info("Resolution %dx%d %dbits %dHz\n", mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);
    }*/

    window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_DMODE_W, DEFAULT_DMODE_H, flags);

    if(!window) {
        log4me_error(LOG_EMU_SDL, "Unable to create window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_DisplayMode windowmode;
    if(SDL_GetWindowDisplayMode(window, &windowmode)==0) {
        fixedres = (windowmode.w!=DEFAULT_DMODE_W) || (windowmode.h!=DEFAULT_DMODE_H);
        if(fixedres)
            desktopmode = windowmode;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    if(!renderer) {
        log4me_error(LOG_EMU_SDL, "Unable to create renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    vfullscreen = 0;
    vscale = 1;
}

void setvideomode(display *screen)
{
    int mscale, width, height;

    screen->window = window;
    screen->renderer = renderer;

    if(screen->fullscreen!=vfullscreen) {
        if(screen->fullscreen)
            vscale = screen->scale;
        else
            screen->scale = vscale;
        vfullscreen = screen->fullscreen;
        if(!fixedres)
            SDL_SetWindowFullscreen(screen->window, screen->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    } else {
        if(screen->scale<1.0) screen->scale = 1;
        if(screen->scale>=MIN(desktopmode.w/256, desktopmode.h/192)) screen->scale = MIN(desktopmode.w/256, desktopmode.h/192);
    }

    switch(screen->fullscreen) {
        case 0:
            width = (256 + 2*MARGIN) * screen->scale;
            height = (192 + 2*MARGIN) * screen->scale;
            if(fixedres) {
                screen->marginx = (desktopmode.w - 256*screen->scale) / 2;
                screen->marginy = (desktopmode.h - 192*screen->scale) / 2;
            } else {
                screen->marginx = MARGIN * screen->scale;
                screen->marginy = MARGIN * screen->scale;
                SDL_SetWindowSize(screen->window, width, height);
            }
            break;
        case 1:
            width = desktopmode.w;
            height = desktopmode.h;
            mscale = MIN(width/256, height/192);
            screen->marginx = (width - 256*mscale) / 2;
            screen->marginy = (height - 192*mscale) / 2;
            screen->scale = mscale;
            break;
    }

    SDL_ShowCursor(screen->fullscreen || fixedres ? 0 : 1);
}

