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
    byte colors[TMS_COLORS];
    byte vram[TMS_VRAM_SIZE];

    Uint32 sdlcolors[TMS_COLORS];

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
    word vram_addr;
    byte read_buffer;

    sdlclock idclock;
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
    SDL_Texture *picture192;
    SDL_Texture *picture224;
    SDL_PixelFormat *pixelfmt;
    word *buffer16;
} tms9918a;

void tms9918a_init(tms9918a *cpn, const display *screen, video_mode vmode);
void tms9918a_free(tms9918a *cpn);

void tms9918a_writeop(tms9918a *cpn, byte val);
byte tms9918a_readstatus(tms9918a *cpn);

void tms9918a_writedata(tms9918a *cpn, byte data);
byte tms9918a_readdata(tms9918a *cpn);

byte tms9918a_getscanline(tms9918a *cpn);
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
#endif

#endif // TMS9918A_H_INCLUDED
