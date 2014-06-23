#ifndef TMS9918A_H_INCLUDED
#define TMS9918A_H_INCLUDED

#include <SDL.h>

#include "emul.h"
#include "clock.h"
#include "video.h"
#include "misc/string.h"
#include "misc/xml.h"

#define TMS_REGISTERS   16
#define TMS_COLORS      32

#define TMS_CRAM_SIZE   64 /* SMS:32 GG:64 */

#define TMS_VRAM_SIZE   0x4000
#define TMS_VRAM_TILES_OFS      0x0000

typedef enum {
    TMS_READ_RAM = 0x0000,
    TMS_WRITE_RAM = 0x4000,
    TMS_WRITE_REGISTER = 0x8000,
    TMS_WRITE_COLOR = 0xC000
} tms9918a_op;

typedef struct _tms9918a {
    byte registers[TMS_REGISTERS];
    byte cram[TMS_CRAM_SIZE]; // 32 bytes for SMS / 64 bytes for GG
    byte vram[TMS_VRAM_SIZE];

    Uint32 sdlcolors[TMS_COLORS];

    gameconsole gconsole;   // SMS or GG

    dword *tiles[512];
    dword tiles_desc[512][(8*8)/4];

    tms9918a_op curoperation;

    union {
        tms9918a_op newoperation;
        struct {
            byte lsb;
            byte msb;
        } bytes;
    } op;

    dword flagsetop;

    int color_index;
    int crammask;
    int cramsize;
    word vram_addr;
    byte read_buffer;

    sdlclock idclock;
    video_mode vmode;
    int framerate;
    int internalscanlines;
    int frame;
    int scanline;
    int slcounter;
    int screenheight;

    int r0_vert_scroll_inhibit;
    int r0_do_not_display_leftmost_col;
    int r0_hsync_int;
    int r0_top2rows_no_hscroll;
    int r0_mode2;
    int r0_mode4;

    int r1_display;
    int r1_vsync_int;
    int r1_8x16sprite;
    int r1_mode3;
    int r1_mode1;
    int r1_zoomedsprites;

    int r2_bgbaseaddr;

    int r5_spattrbaseaddr;

    int r6_sprite_base_tiles;
    int r7_bordercolor;
    int r8_scrollx;
    int r9_scrolly;
    int r10_value;

    byte status;
    byte vsyncint, hsyncint;

    const display *screen;
    const SDL_Rect *ggrect;
    SDL_Texture *picture;
    SDL_PixelFormat *pixelfmt;
    word *buffer16;

#ifdef DEBUG
    int showpalette;
#endif
} tms9918a;

void tms9918a_init(tms9918a *cpn, gameconsole gconsole, const display *screen, video_mode vmode);
void tms9918a_free(tms9918a *cpn);

void tms9918a_writeop(tms9918a *cpn, byte port, byte data);
byte tms9918a_readstatus(tms9918a *cpn, byte port);

void tms9918a_writedata(tms9918a *cpn, byte port, byte data);
byte tms9918a_readdata(tms9918a *cpn, byte port);

byte tms9918a_getscanline(tms9918a *cpn, byte port);
int tms9918a_getmonitorscanlines(tms9918a *cpn);
int tms9918a_getslpersecond(tms9918a *cpn);

void tms9918a_waitnextframe(tms9918a *cpn);
void tms9918a_execute(tms9918a *cpn);

int tms9918a_int_pending(tms9918a *cpn);
void tms9918a_reset_int(tms9918a *cpn);

SDL_Surface *tms9918a_takescreenshot(const tms9918a *cpn);

void tms9918a_takesnapshot(tms9918a *cpn, xmlTextWriterPtr writer);
void tms9918a_loadsnapshot(tms9918a *cpn, xmlNode *vdpnode);

#ifdef DEBUG
void tms9918a_save_tiles(tms9918a *cpn, const string filename);
void tms9918a_toggledisplaypalette(tms9918a *cpn);
#endif

#endif // TMS9918A_H_INCLUDED
