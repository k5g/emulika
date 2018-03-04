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
SDL_Texture *_overlay = NULL;
SDL_Texture *_bezel = NULL;
SDL_RendererInfo _rendererinfo;
int _fixedres = 0;
int _width = 0;
int _height = 0;
int _bzmarginx, _bzmarginy, _bzwidth, _bzheight;

static void video_free()
{
    if(_overlay)
        SDL_DestroyTexture(_overlay);
    if(_bezel)
        SDL_DestroyTexture(_bezel);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
}

void video_init()
{
    int flags = 0;

    pushexit(video_free);

#ifdef RASPBERRYPI
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2"); // Force opengles2 on the raspberry pi plateform
#else
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
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
}

void video_event(const SDL_Event *event)
{
    if((event->type==SDL_WINDOWEVENT) &&
      ((event->window.event==SDL_WINDOWEVENT_RESIZED) || (event->window.event==SDL_WINDOWEVENT_SIZE_CHANGED))) {
        _width = event->window.data1;
        _height = event->window.data2;
    }
}

void video_setmode(display *screen)
{
    int width, height;
    float mscale = screen->minscale<1.0 ? 1.0 : screen->minscale;

    switch(screen->fullscreen) {
        case 1 :
            if(!_fixedres) {
                SDL_SetWindowPosition(_window, 0, 0);
                SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            }
            break;
        case 0 :
            if(screen->scale<mscale) screen->scale = mscale;
            if(screen->scale>=MIN(_desktopmode.w/screen->width, _desktopmode.h/screen->height)) screen->scale = MIN(_desktopmode.w/screen->width, _desktopmode.h/screen->height);

            width = (screen->width + 2*screen->margin) * screen->scale;
            height = (screen->height + 2*screen->margin) * screen->scale;
            SDL_SetWindowFullscreen(_window, 0);
            SDL_SetWindowSize(_window, width, height);
            break;
    }

    SDL_ShowCursor(screen->fullscreen || _fixedres ? 0 : 1);
    SDL_ShowWindow(_window);
}

void displaytexture(const display *screen, SDL_Texture *picture, SDL_PixelFormat *pixelfmt, Uint32 backgroundcolor)
{
    Uint8 r, g, b;
    SDL_Rect srcrect, dstrect;
    float scale;

    if(screen->fullscreen && _bezel) {
        dstrect.x = _bzmarginx;
        dstrect.y = _bzmarginy;
        dstrect.w = _bzwidth;
        dstrect.h = _bzheight;
    } else {
        if(screen->fullscreen && screen->noaspectratio) {
            dstrect.x = 0;
            dstrect.y = 0;
            dstrect.w = _width;
            dstrect.h = _height;
        } else {
            scale = screen->fullscreen ? MIN((float)_width/screen->width, (float)_height/screen->height) : screen->scale;
            dstrect.x = (_width - screen->width*scale) / 2;
            dstrect.y = (_height - screen->height*scale) / 2;
            dstrect.w = screen->width * scale;
            dstrect.h = screen->height * scale;
        }
    }

    SDL_GetRGB(backgroundcolor, pixelfmt, &r, &g, &b);
    SDL_SetRenderDrawColor(_renderer, r, g, b, 255);
    SDL_RenderClear(_renderer);

    SDL_RenderCopy(_renderer, picture, NULL, &dstrect);

    if(_overlay) {
        srcrect.x = srcrect.y = 0;
        srcrect.w = dstrect.w;
        srcrect.h = dstrect.h;
        SDL_RenderCopy(_renderer, _overlay, &srcrect, &dstrect);
    }

    if(screen->fullscreen && _bezel)
        SDL_RenderCopy(_renderer, _bezel, NULL, NULL);

    SDL_RenderPresent(_renderer);
}

void video_setoverlay(display *screen, const char *filename)
{
    SDL_Surface *img = NULL;
    int error = 0;

    img = IMG_Load(filename);
    if(!img) {
        log4me_error(LOG_EMU_MAIN, "Unable to load the overlay file : %s.\n", IMG_GetError());
        error = 1;
        goto error;
    }

    _overlay = SDL_CreateTextureFromSurface(_renderer, img);

error:
    if(img) SDL_FreeSurface(img);

    if(error)
        exit(EXIT_FAILURE);
}

void video_setbezel(display *screen, const char *filename)
{
    SDL_Surface *img = NULL;
    SDL_Surface *imgscale;
    Uint8 r, g, b;
    Uint8 alpha;
    int i, j;
    int xmin, xmax, ymin, ymax;
    int error = 0;

    img = IMG_Load(filename);
    if(!img) {
        log4me_error(LOG_EMU_MAIN, "Unable to load the bezel file : %s.\n", IMG_GetError());
        error = 1;
        goto error;
    }

    // Resize the image if necessary
    if((img->w!=_desktopmode.w) || (img->h!=_desktopmode.h)) {
        imgscale = SDL_CreateRGBSurface(0, _desktopmode.w, _desktopmode.h, img->format->BitsPerPixel, img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask);
        if(imgscale==NULL) {
            log4me_error(LOG_EMU_MAIN, "Unable to load the bezel file : %s.\n", IMG_GetError());
            error = 1;
            goto error;
        }

        SDL_SetSurfaceBlendMode(img, SDL_BLENDMODE_NONE);
        SDL_BlitScaled(img, NULL, imgscale, NULL);

        SDL_FreeSurface(img);
        img = imgscale;
    }

    xmin = img->w;
    ymin = img->h;
    xmax = ymax = 0;

    //if(img->format->BytesPerPixel!=4)
    //    return;

    for(i=0; i<img->h; i++) {
        Uint32 *pixel = (Uint32*)(img->pixels + i*img->pitch);
        for(j=0; j<img->w; j++, pixel++) {
            SDL_GetRGBA(*pixel, img->format, &r, &g, &b, &alpha);
            if(alpha==255) continue;

            if(i<ymin) ymin = i;
            if(i>ymax) ymax = i;
            if(j<xmin) xmin = j;
            if(j>xmax) xmax = j;
        }
    }

    _bzmarginx = xmin;
    _bzmarginy = ymin;
    _bzwidth   = xmax - xmin + 1;
    _bzheight  = ymax - ymin + 1;

    _bezel = SDL_CreateTextureFromSurface(_renderer, img);

error:
    if(img) SDL_FreeSurface(img);

    if(error)
        exit(EXIT_FAILURE);
}

SDL_Window *video_getcurrentwindow()
{
    return _window;
}

SDL_Renderer *video_getrenderer()
{
    return _renderer;
}

const char *video_getcurrentrendererdriver()
{
    return _rendererinfo.name;
}

const char *video_getcurrentvideodriver()
{
    return SDL_GetCurrentVideoDriver();
}

