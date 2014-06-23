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

#define DEFAULT_DMODE_W 640
#define DEFAULT_DMODE_H 480

SDL_DisplayMode _desktopmode;
SDL_Window *_window = NULL;
SDL_Renderer *_renderer = NULL;
SDL_RendererInfo _rendererinfo;
int _fixedres = 0;
byte _vfullscreen = 0;
float _vscale = 1;

static void video_free()
{
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
}

void video_init()
{
    int flags = 0;

    pushexit(video_free);

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#ifdef RASPBERRYPI
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2"); // Force opengles2 on the raspberry pi plateform
#endif

    if(SDL_GetDesktopDisplayMode(0, &_desktopmode)<0) {
        log4me_error(LOG_EMU_SDL, "Unable to get desktop display mode : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    /*int i;
    SDL_DisplayMode mode;
    for(i=0;i<SDL_GetNumDisplayModes(0); i++) {
        SDL_GetDisplayMode(0, i, &mode);
        log4me_info("Resolution %dx%d %dbits %dHz\n", mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);
    }*/

    flags |= SDL_WINDOW_HIDDEN;
    _window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_DMODE_W, DEFAULT_DMODE_H, flags);

    if(!_window) {
        log4me_error(LOG_EMU_SDL, "Unable to create window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_DisplayMode windowmode;
    if(SDL_GetWindowDisplayMode(_window, &windowmode)==0) {
        _fixedres = (windowmode.w!=DEFAULT_DMODE_W) || (windowmode.h!=DEFAULT_DMODE_H);
        if(_fixedres)
            _desktopmode = windowmode;
    }

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_PRESENTVSYNC);

    if(!_renderer) {
        log4me_error(LOG_EMU_SDL, "Unable to create renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    memset(&_rendererinfo, 0, sizeof(_rendererinfo));
    SDL_GetRendererInfo(_renderer, &_rendererinfo);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
    SDL_RenderClear(_renderer);
    SDL_RenderPresent(_renderer);

    _vfullscreen = 0;
    _vscale = 1;
}

void setvideomode(display *screen)
{
    int width, height;
    float fsscale, mscale = screen->minscale<1.0 ? 1.0 : screen->minscale;

    screen->window = _window;
    screen->renderer = _renderer;

    if(screen->fullscreen!=_vfullscreen) {
        if(screen->fullscreen)
            _vscale = screen->scale;
        else
            screen->scale = _vscale;
        _vfullscreen = screen->fullscreen;
        if(!_fixedres)
            SDL_SetWindowFullscreen(screen->window, screen->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    } else {
        if(screen->scale<mscale) screen->scale = mscale;
        if(screen->scale>=MIN(_desktopmode.w/screen->width, _desktopmode.h/screen->height)) screen->scale = MIN(_desktopmode.w/screen->width, _desktopmode.h/screen->height);
    }

    switch(screen->fullscreen) {
        case 0:
            width = (screen->width + 2*screen->margin) * screen->scale;
            height = (screen->height + 2*screen->margin) * screen->scale;
            if(_fixedres) {
                screen->marginx = (_desktopmode.w - screen->width*screen->scale) / 2;
                screen->marginy = (_desktopmode.h - screen->height*screen->scale) / 2;
            } else {
                screen->marginx = screen->margin * screen->scale;
                screen->marginy = screen->margin * screen->scale;
                SDL_SetWindowSize(screen->window, width, height);
            }
            break;
        case 1:
            width = _desktopmode.w;
            height = _desktopmode.h;
            fsscale = MIN((float)width/screen->width, (float)height/screen->height);
            screen->marginx = (width - screen->width*fsscale) / 2;
            screen->marginy = (height - screen->height*fsscale) / 2;
            screen->scale = fsscale;
            break;
    }

    SDL_ShowCursor(screen->fullscreen || _fixedres ? 0 : 1);
    SDL_ShowWindow(screen->window);
}

SDL_Window *getcurrentwindow()
{
    return _window;
}

const char *getcurrentrendererdriver()
{
    return _rendererinfo.name;
}

const char *getcurrentvideodriver()
{
    return SDL_GetCurrentVideoDriver();
}

