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

#include "emuconfig.h"
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
void getconfigfilename(int argc, char **argv, const appenv *env, string *configfilename);
void readoptions(int argc, char **argv, char **romfilename, tmachine *machine, video_mode *vmode, int *codemasters, emuconfig *config);

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

    seticon(video_getcurrentwindow());
}

int main ( int argc, char** argv )
{
    int done, pause, bookmark;

    string configfilename  = NULL;

    mastersystem *sms = NULL;
    appenv *environment = NULL;
    romspecs *rspecs = NULL;

    emuconfig config;
    display screen;
    SDL_version sdlvers;

    char *romfilename=NULL;
    int codemasters = 0;

    tmachine machine = UNDEFINED;
    video_mode vmode = UNDEFINED;

    iprofile *player1 = NULL;
    iprofile *player2 = NULL;

    initconfig(&config);

#ifndef DEBUG
    assert(0);
#endif

    environment = getappenv();
    initmodules(environment->basedir);

    log4me_print("-==| %s version %s |==-\n", PACKAGE, VERSION);
#ifdef DEBUG
    log4me_print("  => DEBUG version\n");
#endif

    // Show SDL informations
    SDL_GetVersion(&sdlvers);
    log4me_print("SDL : %d.%d.%d\n", sdlvers.major, sdlvers.minor, sdlvers.patch);
    log4me_print("SDL : Video driver (%s)\n", video_getcurrentvideodriver());
    log4me_print("SDL : Rendered driver (%s)\n", video_getcurrentrendererdriver());
    log4me_print("SDL : Audio driver (%s)\n", SDL_GetCurrentAudioDriver());

    // Show SDL joysticks informations
    int i;
    padinfos infos;
    for(i=0;i<SDL_NumJoysticks();i++) {
        if(input_pad_getinfos(i, &infos)) {
            log4me_print("SDL : Joystick detected => %s\n", infos.name);
            log4me_print("\tButtons : %d, Axis : %d, Hats : %d\n", infos.buttons, infos.axis, infos.hats);
        }
    }

    strfree(configfilename);
    getconfigfilename(argc, argv, environment, &configfilename);

    if(fileexists(configfilename))
        readconfig(configfilename, &config, &player1, &player2);
    strfree(configfilename);

    readoptions(argc, argv, &romfilename, &machine, &vmode, &codemasters, &config);

    screen.fullscreen = config.fullscreen;
    screen.noaspectratio = config.noaspectratio;
    screen.scale = config.scale;

    // Load default input profile if necessary
    if(player1==NULL)
        input_loaddefaultprofile(&player1, player2==NULL ? &player2 : NULL);

    // Init input profile for players
    if(input_setprofile(player1)==0) {
        log4me_error(LOG_EMU_MAIN, "Unable to initialize input profile for player 1 : %s.\n", player1 ? CSTR(player1->base.name) : "undefined");
        exit(EXIT_FAILURE);
    } else
        log4me_print("P1  : Input profile = %s\n", CSTR(player1->base.name));


    if(player2) {
        if(input_setprofile(player2)==0) {
            log4me_error(LOG_EMU_MAIN, "Unable to initialize input profile for player 2 : %s.\n", player2 ? CSTR(player2->base.name) : "undefined");
            exit(EXIT_FAILURE);
        } else
            log4me_print("P2  : Input profile = %s\n", CSTR(player2->base.name));
    }

    rspecs = getromspecs(romfilename, machine, vmode, codemasters);
    machine = getrommachine(rspecs);
    vmode = getromvideomode(rspecs);

    assert((machine==JAPAN) || (machine==EXPORT));
    assert((vmode==VM_NTSC) || (vmode==VM_PAL));
    log4me_print("SMS : Use %s machine with %s video mode\n", machine==EXPORT ? "Export" : "Japan", vmode==VM_PAL ? "PAL" : "NTSC");

    screen.minscale = (float)224.0 / 192;

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
    video_setmode(&screen);

    if(config.overlayfilename)
        video_setoverlay(&screen, CSTR(config.overlayfilename));


    if(config.bezelfilename)
        video_setbezel(&screen, CSTR(config.bezelfilename));

    audio_setvolume(config.volume);

    sms = ms_init(&screen, rspecs, config.nosound ? SND_OFF : SND_ON, player1, player2, environment->backup);
    if(sms==NULL) {
        log4me_error(LOG_EMU_MAIN, "Unable to allocate and initialize the SMS emulator.\n");
        exit(EXIT_FAILURE);
    }

    SDL_SetWindowTitle(video_getcurrentwindow(), CSTR(sms->romname));

    done = pause = bookmark = 0;

    ms_start(sms);
    while (!done)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            video_event(&event);
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
            video_setmode(&screen);
            ms_pause(sms, 0);
        }

        if(input_key_down(SDL_SCANCODE_F3)) {
            ms_pause(sms, 1);
            screen.scale += 0.2;
            video_setmode(&screen);
            ms_pause(sms, 0);
        }

        if(input_key_down(SDL_SCANCODE_F4)) {
            ms_pause(sms, 1);
            screen.fullscreen ^= 1;
            video_setmode(&screen);
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

    input_freeprofile(player1);
    input_freeprofile(player2);

    releaseobject(environment);
    freeconfig(&config);

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
    log4me_print("  --fullscreen\t\t: Set the fullscreen mode\n");
    log4me_print("  --machine MCH\t\t: Set the machine type [japan/export] (default=japan)\n");
    log4me_print("  --mode TYPE\t\t: Set the video mode NTSC/PAL [ntsc/pal] (default=ntsc)\n");
    log4me_print("  --nosound\t\t: Set the sound to off\n");
    log4me_print("  --volume VOL\t\t: Set the volume [0..100] (default=100)\n");
    log4me_print("  --noaspectratio\t: Don't keep aspect ratio\n");
    log4me_print("  --scale NUM\t\t: Increase the size of the screen by NUM (default=%d)\n", DEFAULT_SCALE);
    log4me_print("  --codemasters\t\t: Force the compatibility of Codemasters games\n");
    log4me_print("  --config FILE\t\t: Set the configuration filename\n");
    log4me_print("  --overlay FILE\t: Add the overlay image to the video output\n");
    log4me_print("  --bezel FILE\t\t: Add the bezel image to the video output (only in fullscreen mode)\n");
}

void getconfigfilename(int argc, char **argv, const appenv *env, string *configfilename)
{
    #define ARGVSIZE (sizeof(char*) * argc)
    // http://askedquestionsnanswers.net/question/4832603-how-could-i-temporary-redirect-stdout-to-a-file-in-a-c-program

    int c, option_index;
    string filename = NULL;
    char **argv_copy = malloc(ARGVSIZE);

    struct option long_options[] = {
        {"fullscreen"   , no_argument       , NULL, 0   },
        {"machine"      , no_argument       , NULL, 0   },
        {"mode"         , no_argument       , NULL, 0   },
        {"volume"       , no_argument       , NULL, 0   },
        {"nosound"      , no_argument       , NULL, 0   },
        {"noaspectratio", no_argument       , NULL, 0   },
        {"scale"        , no_argument       , NULL, 0   },
        {"codemasters"  , no_argument       , NULL, 0   },
        {"config"       , required_argument , NULL, 'c' },
        {"bezel"        , no_argument       , NULL, 0   },
        {"overlay"      , no_argument       , NULL, 0   },
        {"help"         , no_argument       , NULL, 0   },
        { NULL          , 0                 , NULL, 0   }
    };

    assert(optind==1);
    memcpy(argv_copy, argv, ARGVSIZE);

    while((c = getopt_long(argc, argv_copy, "", long_options, &option_index))!=-1) {
        if(c=='c')
            filename = strcrec(optarg);
    }

    if(filename==NULL) {
        filename = strdups(env->basedir);
        pathcombc(filename, "config.xml");
    }

    *configfilename = filename;
    free(argv_copy);
    optind = 1; // reset it to 1 to restart scanning for readoptions
}

void readoptions(int argc, char **argv, char **romfilename, tmachine *machine, video_mode *vmode, int *codemasters, emuconfig *config)
{
    int c;
    int option_index;

    struct option long_options[] = {
        {"fullscreen", no_argument, &config->fullscreen, 1},
        {"machine", required_argument, NULL, 't'},
        {"mode", required_argument, NULL, 'm'},
        {"volume", required_argument, NULL, 'v'},
        {"nosound", no_argument, &config->nosound, 1},
        {"noaspectratio", no_argument, &config->noaspectratio, 1},
        {"scale", required_argument, NULL, 's'},
        {"codemasters", no_argument, codemasters, 1},
        {"config", required_argument, NULL, 'c'},
        {"bezel", required_argument, NULL, 'b'},
        {"overlay", required_argument, NULL, 'o'},
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
                config->scale = atof(optarg);
                if(config->scale<1) {
                    log4me_print("wrong scale parameter, set to default value\n");
                    config->scale = DEFAULT_SCALE;
                }
                break;
            case 'v':
                config->volume = atoi(optarg);
                break;
            case 't':
                *machine = strcasecmp(optarg, "export")==0 ? EXPORT : JAPAN;
                break;
            case 'm':
                *vmode = strcasecmp(optarg, "pal")==0 ? VM_PAL : VM_NTSC;
                break;
            case 'c':
                break;
            case 'b':
                strfree(config->bezelfilename);
                config->bezelfilename = strcrec(optarg);
                break;
            case 'o':
                strfree(config->overlayfilename);
                config->overlayfilename = strcrec(optarg);
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
