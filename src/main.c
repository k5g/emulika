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

#include <getopt.h>
#include <zip.h>

#include "environ.h"
#include "emul.h"
#include "sms.h"
#include "string.h"
#include "icon.h"
#include "input.h"
#include "rom.h"
#include "snapshot.h"
#include "screenshot.h"
#include "misc/log4me.h"
#include "misc/exit.h"

#if defined(HAVE_ENDIAN_H)
    #include <endian.h>
    #if __BYTE_ORDER != __LITTLE_ENDIAN
        #error "Only little endian processor is supported :("
    #endif
#endif /* HAVE_ENDIAN_H */

#define DEFAULT_SCALE   2

typedef struct {
    string basedir;
    string snapshots;
    string backup;
    string screenshots;
#ifdef DEBUG
    string debug;
#endif
} appenv;

static appenv *getappenv(void);
void savetiles(mastersystem *sms, const string debugdir);
void readoptions(int argc, char **argv, char **romfilename, int *fullscreen, tmachine *machine, video_mode *vmode, int *nosound, float *scale, int *codemasters);

void initmodules(const string basedir)
{
	log4me_init(LOG_EMU_NONE);
#ifdef WIN32
	string redirect = pathcombc(strdups(basedir), "output.txt");
	log4me_redirectoutput(CSTR(redirect));
	strfree(redirect);
#endif

    LIBXML_TEST_VERSION;

    clock_init();

    // initialize SDL video
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)<0) {
        log4me_error(LOG_EMU_SDL, "Unable to init SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    // make sure SDL cleans up before exit
    pushexit(SDL_Quit);

    video_init();
    input_init();

    seticon(getcurrentwindow());
}

int main ( int argc, char** argv )
{
    int done, pause, bookmark;
    int joypads[NJOYSTICKS];

    mastersystem *sms = NULL;
    appenv *environment = NULL;
    romspecs *rspecs = NULL;

    display screen;
    SDL_version sdlvers;

    char *romfilename=NULL;
    int nosound = 0;
    int codemasters = 0;

    tmachine machine = UNDEFINED;
    video_mode vmode = UNDEFINED;

    screen.fullscreen = 0;
    screen.scale = DEFAULT_SCALE;
    screen.minscale = (float)224.0 / 192;

#ifndef DEBUG
    assert(0);
#endif

    environment = getappenv();
    initmodules(environment->basedir);

    log4me_print("-==| %s version %s |==-\n", PACKAGE, VERSION);
#ifdef DEBUG
    log4me_print("  => DEBUG version\n");
#endif
    readoptions(argc, argv, &romfilename, &screen.fullscreen, &machine, &vmode, &nosound, &screen.scale, &codemasters);

    SDL_GetVersion(&sdlvers);
    log4me_print("SDL : %d.%d.%d\n", sdlvers.major, sdlvers.minor, sdlvers.patch);
    log4me_print("SDL : Video driver (%s)\n", getcurrentvideodriver());
    log4me_print("SDL : Rendered driver (%s)\n", getcurrentrendererdriver());
    log4me_print("SDL : Audio driver (%s)\n", SDL_GetCurrentAudioDriver());

    // Init joypads
    int i;
    for(i=0;i<NJOYSTICKS;i++) joypads[i] = input_new_pad();

    // Display joypads informations
    if(input_pad_detected()) {
        padinfos infos;
        for(i=0;i<NJOYSTICKS;i++) {
            if(joypads[i]==NO_JOYPAD) continue;
            input_pad_getinfos(joypads[i], &infos);
            log4me_print("SDL : Player %d joystick detected => %s\n", i+1, infos.name);
            log4me_print("\tButtons : %d, Axis : %d, Hats : %d\n", infos.buttons, infos.axis, infos.hats);
        }
    } else
        log4me_print("SDL : No joystick detected\n");

    rspecs = getromspecs(romfilename, machine, vmode, codemasters);
    machine = getrommachine(rspecs);
    vmode = getromvideomode(rspecs);

    assert((machine==JAPAN) || (machine==EXPORT));
    assert((vmode==VM_NTSC) || (vmode==VM_PAL));
    log4me_print("SMS : Use %s machine with %s video mode\n", machine==EXPORT ? "Export" : "Japan", vmode==VM_PAL ? "PAL" : "NTSC");

    switch(getromgameconsole(rspecs)) {
        case GC_SMS:
            screen.width = 256;
            screen.height = 192;
            screen.margin = DEFAULT_MARGIN;
            break;
        case GC_GG:
            screen.width = 160;
            screen.height = 144;
            screen.margin = 0;
            break;
        default:
            assert(0);
            break;
    }
    setvideomode(&screen);

    sms = ms_init(&screen, rspecs, nosound ? SND_OFF : SND_ON, joypads[0], joypads[1], environment->backup);
    if(sms==NULL) {
        log4me_error(LOG_EMU_MAIN, "Unable to allocate and initialize the SMS emulator.\n");
        exit(EXIT_FAILURE);
    }

    SDL_SetWindowTitle(screen.window, CSTR(sms->romname));

    done = pause = bookmark = 0;

    ms_start(sms);
    while (!done)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            input_process_event(&event);
            done |= (event.type==SDL_QUIT);
        }

        done |= input_key_pressed(SDL_SCANCODE_ESCAPE);

        if(input_key_down(SDL_SCANCODE_PAUSE)) {
            pause ^= 1;
            ms_pause(sms, pause);
        }

        if(input_key_down(SDL_SCANCODE_F9))
            takesnapshot(sms, environment->snapshots);

        if(input_key_down(SDL_SCANCODE_F10))
            takescreenshot(sms, environment->screenshots);

        if(input_key_down(SDL_SCANCODE_F2)) {
            ms_pause(sms, 1);
            screen.scale -= 0.2;
            setvideomode(&screen);
            ms_pause(sms, 0);
        }

        if(input_key_down(SDL_SCANCODE_F3)) {
            ms_pause(sms, 1);
            screen.scale += 0.2;
            setvideomode(&screen);
            ms_pause(sms, 0);
        }

        if(input_key_down(SDL_SCANCODE_F4)) {
            ms_pause(sms, 1);
            screen.fullscreen ^= 1;
            setvideomode(&screen);
            ms_pause(sms, 0);
        }

#ifdef DEBUG
        if(input_key_down(SDL_SCANCODE_B))
            log4me_print("[BKM] %d\n", bookmark++);

        if(input_key_down(SDL_SCANCODE_F5))
            savetiles(sms, environment->debug);

        if(input_key_down(SDL_SCANCODE_F6))
            tms9918a_toggledisplaypalette(&sms->vdp);
#endif

        if(!ms_ispaused(sms)) ms_execute(sms);
    }

    releaseobject(sms);
    releaseobject(rspecs);
    for(i=0;i<NJOYSTICKS;i++) input_release_pad(joypads[i]);

    releaseobject(environment);

    return 0;
}

void printusage()
{
    log4me_print("Usage: "PACKAGE" [OPTIONS] ROM\n");
}

void printhelp()
{
    printusage();
    log4me_print("\nOptions:\n");
    log4me_print("  --fullscreen\t: Set fullscreen mode\n");
    log4me_print("  --machine MCH\t: Set machine type [japan/export] (default=japan)\n");
    log4me_print("  --mode TYPE\t: Set video mode NTSC/PAL [ntsc/pal] (default=ntsc)\n");
    log4me_print("  --nosound\t: Set sound off\n");
    log4me_print("  --scale NUM\t: Increase the size of the screen by NUM (default=%d)\n", DEFAULT_SCALE);
    log4me_print("  --codemasters\t: Force the compatibility of Codemasters games\n");
}

void readoptions(int argc, char **argv, char **romfilename, int *fullscreen, tmachine *machine, video_mode *vmode, int *nosound, float *scale, int *codemasters)
{
    int c;
    int option_index;

    struct option long_options[] = {
        {"fullscreen", no_argument, fullscreen, 1},
        {"machine", required_argument, NULL, 't'},
        {"mode", required_argument, NULL, 'm'},
        {"nosound", no_argument, nosound, 1},
        {"scale", required_argument, NULL, 's'},
        {"codemasters", no_argument, codemasters, 1},
        {"help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0}
    };

    if(argc<=1) {
        printusage();
        log4me_print("Try '"PACKAGE" --help' for more information.\n");
    }

    while((c = getopt_long(argc, argv, "", long_options, &option_index))!=-1) {
        //printf("-> %c %d (%d)\n", c, (int)c, option_index);
        switch(c) {
            case 's':
                *scale = atof(optarg);
                if(*scale<1) {
                    log4me_print("wrong scale parameter, set to default value\n");
                    *scale = DEFAULT_SCALE;
                }
                break;
            case 't':
                *machine = strcasecmp(optarg, "export")==0 ? EXPORT : JAPAN;
                break;
            case 'm':
                *vmode = strcasecmp(optarg, "pal")==0 ? VM_PAL : VM_NTSC;
                break;
            case 'h':
                printhelp();
                exit(EXIT_SUCCESS);
                break;
            case '?':
                printhelp();
                exit(EXIT_FAILURE);
                break;
        }
    }

    if(optind>=argc) {
        log4me_print("ROM filename missing.\n");
        exit(EXIT_FAILURE);
    }

    *romfilename = argv[optind];
}

static void freeappenv(appenv *app)
{
    strfree(app->basedir);
    strfree(app->snapshots);
    strfree(app->backup);
    strfree(app->screenshots);
#ifdef DEBUG
    strfree(app->debug);
#endif /* DEBUG */
    free(app);
}

static appenv *getappenv()
{
    #define SNAPSHOTS "snapshots"
    #define BACKUP "backup"
    #define SCREENSHOTS "screenshots"

    appenv *app = malloc(sizeof(appenv));
    pushobject(app, freeappenv);

    app->basedir = pathcombc(gethomedir(), "." PACKAGE);
    createdirectory(app->basedir);

    app->snapshots = pathcombc(strdups(app->basedir), SNAPSHOTS);
    createdirectory(app->snapshots);

    app->backup = pathcombc(strdups(app->basedir), BACKUP);
    createdirectory(app->backup);

    app->screenshots = pathcombc(strdups(app->basedir), SCREENSHOTS);
    createdirectory(app->screenshots);

#ifdef DEBUG
    app->debug = pathcombc(strdups(app->basedir), "debug");
    createdirectory(app->debug);
#endif

    return app;
}

#ifdef DEBUG
void savetiles(mastersystem *sms, const string debugdir)
{
    string filename = gettimedfilename(CSTR(sms->romname), "bmp");

    string tilesfilename = strdups(debugdir);
    pathcombs(tilesfilename, filename);

    tms9918a_save_tiles(&sms->vdp, tilesfilename);
    strfree(filename);
    strfree(tilesfilename);
}
#endif
