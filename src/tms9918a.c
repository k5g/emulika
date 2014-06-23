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

#include "tms9918a.h"
#include "misc/log4me.h"

#define TILES_WIDTH_SCREEN      32
#define TILES_HEIGHT_SCREEN     28
#define TILES                   (TILES_WIDTH_SCREEN * TILES_HEIGHT_SCREEN)

#define TMS_STATUS_VSYNC            0x80
#define TMS_STATUS_SPRITES_OVERFLOW 0x40
#define TMS_STATUS_SPRITES_COLLIDE  0x20

#define TMS_STATUS_RESET_FLAGS      (~(TMS_STATUS_VSYNC | TMS_STATUS_SPRITES_OVERFLOW | TMS_STATUS_SPRITES_COLLIDE))

#define TMS_BG_ATTR_FLIPH       0x0200
#define TMS_BG_ATTR_FLIPV       0x0400
#define TMS_BG_ATTR_SPRITE_PAL  0x0800

#define mode224selected(cpn)    ((cpn)->r0_mode4 && !(cpn)->r1_mode3 && (cpn)->r0_mode2 &&  (cpn)->r1_mode1)
#define mode240selected(cpn)    ((cpn)->r0_mode4 &&  (cpn)->r1_mode3 && (cpn)->r0_mode2 && !(cpn)->r1_mode1)

typedef enum {
    TMS_DRAW_ALL,
    TMS_DRAW_BG_ONLY,
    TMS_DRAW_FRONT_ONLY
} tms9918a_drawbg;

typedef struct {
    int x, offset;
    word desc;
} tms99818a_fgtile;

static const int lkp_mod224[512] = {
#define M1(x) ((x) % 224)
    M512(0)
#undef M1
};

static const int lkp_mod256[512] = {
#define M1(x) ((x) & 0xFF)
    M512(0)
#undef M1
};

#define GG_SCREEN_WIDTH     (20 * 8)    /* 160 pixels */
#define GG_SCREEN_HEIGHT    (18 * 8)    /* 144 pixels */
static const SDL_Rect ggrect192 = { 6*8, 3*8, GG_SCREEN_WIDTH, GG_SCREEN_HEIGHT };
static const SDL_Rect ggrect224 = { 6*8, 5*8, GG_SCREEN_WIDTH, GG_SCREEN_HEIGHT };

// => Sega Master System VDP documentation by Charles MacDonald
// See chapter 11.) Display timing
static const byte lkp_vcounter_ntsc192[262] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xD5,0xD6,0xD7,0xD8,0xD9,
0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,
0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,
0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};
static const byte lkp_vcounter_ntsc224[262] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xE5,0xE6,0xE7,0xE8,0xE9,
0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,
0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};
static const byte lkp_vcounter_pal192[313] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
0xF0,0xF1,0xF2,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,
0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,
0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,
0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,
0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};
static const byte lkp_vcounter_pal224[313] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,
0x00,0x01,0x02,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,
0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,
0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,
0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

static void tms9918a_setregister(tms9918a *cpn, int reg, byte data);

void tms9918a_init(tms9918a *cpn, gameconsole gconsole, const display *screen, video_mode vmode)
{
    memset(cpn->registers, 0, TMS_REGISTERS);
    memset(cpn->cram, 0, TMS_CRAM_SIZE);
    memset(cpn->vram, 0, TMS_VRAM_SIZE);
    memset(cpn->sdlcolors, 0, sizeof(cpn->sdlcolors));
    memset(cpn->tiles, 0, sizeof(cpn->tiles));
    memset(cpn->tiles_desc, 0, sizeof(cpn->tiles_desc));

    assert((gconsole==GC_SMS) || (gconsole==GC_GG));
    cpn->gconsole = gconsole;

    cpn->curoperation = TMS_READ_RAM;
    cpn->op.newoperation = TMS_READ_RAM;
    cpn->flagsetop = 0;

    cpn->color_index = 0;
    cpn->crammask = gconsole==GC_GG ? 0x3F : 0x1F;
    cpn->cramsize = gconsole==GC_GG ? TMS_CRAM_SIZE : TMS_CRAM_SIZE / 2;
    cpn->vram_addr = 0;
    cpn->read_buffer = 0;

    cpn->screen = screen;
    cpn->ggrect = &ggrect192;
    cpn->picture = gconsole==GC_GG ?
                        SDL_CreateTexture(screen->renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, GG_SCREEN_WIDTH, GG_SCREEN_HEIGHT) :
                        SDL_CreateTexture(screen->renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, 256, 192);

    cpn->pixelfmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGB565);
    cpn->buffer16 = calloc(256*224, sizeof(word));
    // TODO: cpn->buffer16==NULL;

    // => Sega Master System VDP documentation by Charles MacDonald
    // A NTSC machine displays 60 frames per second, each frame has 262 scanlines.
    // A PAL machine displays 50 frames per second, each frame has 313 scanlines.
    cpn->vmode = vmode;
    cpn->framerate = vmode==VM_NTSC ? 60 : 50;
    cpn->internalscanlines = vmode==VM_NTSC ? 262 : 313;

    cpn->idclock = clock_new(cpn->framerate);
    cpn->frame = 0;
    cpn->scanline = 0;
    cpn->slcounter = 0xFF; // reg 10
    cpn->screenheight = 192;

    cpn->status = 0;
    cpn->vsyncint = cpn->hsyncint = 0;

    // => Software Reference Manual for the SEGA Mark III Console
    // The VDP registers are initialized at power on to the following values
    tms9918a_setregister(cpn, 0x0, 0x36);
    tms9918a_setregister(cpn, 0x1, 0xA0);
    tms9918a_setregister(cpn, 0x2, 0xFF);
    tms9918a_setregister(cpn, 0x3, 0xFF);
    tms9918a_setregister(cpn, 0x4, 0xFF);
    tms9918a_setregister(cpn, 0x5, 0xFF);
    tms9918a_setregister(cpn, 0x6, 0xFB);
    tms9918a_setregister(cpn, 0x7, 0x00);
    tms9918a_setregister(cpn, 0x8, 0x00);
    tms9918a_setregister(cpn, 0x9, 0x00);
    tms9918a_setregister(cpn, 0xA, 0xFF);

#ifdef DEBUG
    cpn->showpalette = 0;
#endif
}

void tms9918a_free(tms9918a *cpn)
{
    free(cpn->buffer16);
    if(cpn->pixelfmt)
        SDL_FreeFormat(cpn->pixelfmt);
    if(cpn->picture)
        SDL_DestroyTexture(cpn->picture);
    clock_release(cpn->idclock);
}

static int tms9918a_getbgbaseaddr(tms9918a *cpn)
{
    return mode224selected(cpn) ? (((cpn->registers[2] & 0xC) << 10) + 0x0700) : ((cpn->registers[2] & 0xE) << 10);
}

static void tms9918a_setregister(tms9918a *cpn, int reg, byte data)
{
    cpn->registers[reg] = data;

    switch(reg) {
        case 0:
            if(bit0_is_set(data) || bit3_is_set(data)) {
                log4me_warning(LOG_EMU_SMSVDP, "write register 0 : 0x%02X\n", data);
                exit(EXIT_FAILURE);
            }

            cpn->r0_vert_scroll_inhibit = bit7_is_set(data);
            cpn->r0_top2rows_no_hscroll = bit6_is_set(data);
            cpn->r0_do_not_display_leftmost_col = bit5_is_set(data);
            cpn->r0_hsync_int = bit4_is_set(data);
            cpn->r0_mode4 = bit2_is_set(data);
            cpn->r0_mode2 = bit1_is_set(data);
            log4me_debug(LOG_EMU_SMSVDP, "write register 0 : hsyncint=%d, do_not_display_leftmost_col=%d, top2rows_no_hscroll=%d, mode4=%d, mode2=%d : 0x%02X\n", cpn->r0_hsync_int, cpn->r0_do_not_display_leftmost_col, cpn->r0_top2rows_no_hscroll, cpn->r0_mode4, cpn->r0_mode2, data);
            if(!cpn->r0_hsync_int) cpn->hsyncint = 0;
#ifdef DEBUG
            if(cpn->r2_bgbaseaddr!=tms9918a_getbgbaseaddr(cpn))
                log4me_warning(LOG_EMU_SMSVDP, "Force background base address (0x%04X)\n", tms9918a_getbgbaseaddr(cpn));
#endif
            cpn->r2_bgbaseaddr = tms9918a_getbgbaseaddr(cpn);
            break;

        case 1:
            cpn->r1_display = bit6_is_set(data);
            cpn->r1_vsync_int = bit5_is_set(data);
            cpn->r1_mode1 = bit4_is_set(data);
            cpn->r1_mode3 = bit3_is_set(data);
            cpn->r1_8x16sprite = bit1_is_set(data);
            cpn->r1_zoomedsprites = bit0_is_set(data);
            log4me_debug(LOG_EMU_SMSVDP, "write register 1 : display=%d, vsyncint=%d, mode1=%d, mode3=%d, 8x16sprite=%d zoomedsprites=%d : 0x%02X\n", cpn->r1_display, cpn->r1_vsync_int, cpn->r1_mode1, cpn->r1_mode3, cpn->r1_8x16sprite, cpn->r1_zoomedsprites, data);
            if(cpn->r1_vsync_int) {
                cpn->vsyncint = ((cpn->status & TMS_STATUS_VSYNC)!=0);
                if(cpn->vsyncint)
                    log4me_debug(LOG_EMU_SMSVDP, "vsync int\n");
            } else
                cpn->vsyncint = 0;
#ifdef DEBUG
            if(cpn->r2_bgbaseaddr!=tms9918a_getbgbaseaddr(cpn))
                log4me_warning(LOG_EMU_SMSVDP, "Force background base address (0x%04X)\n", tms9918a_getbgbaseaddr(cpn));
#endif
            cpn->r2_bgbaseaddr = tms9918a_getbgbaseaddr(cpn);
            break;

        case 2:
            cpn->r2_bgbaseaddr = tms9918a_getbgbaseaddr(cpn);
            log4me_debug(LOG_EMU_SMSVDP, "write register 2 set background base address (0x%04X)\n", cpn->r2_bgbaseaddr);
            break;

        case 3: case 4:
            if(data!=0xFF) {
                log4me_warning(LOG_EMU_SMSVDP, "write register %d other than 0xFF not implemented (0x%02X)\n", reg, data);
                //exit(EXIT_FAILURE);
            }
            log4me_debug(LOG_EMU_SMSVDP, "write register %d = 0x%02X (%d)\n", reg, data, data);
            break;

        case 5:
            cpn->r5_spattrbaseaddr = ((word)(data & 0x7E)) << 7;
            log4me_debug(LOG_EMU_SMSVDP, "write register 5 set sprites attributes base address (0x%04X)\n", cpn->r5_spattrbaseaddr);
            break;

        case 6:
            //if((data!=0xFF) && (data!=0xFB))
            //    log4me_warning(LOG_EMU_SMSVDP, "write register %d other than 0xFF or 0xFB not implemented (0x%02X)\n", reg, data);
            cpn->r6_sprite_base_tiles = bit2_is_set(data) ? 256 : 0;
            log4me_debug(LOG_EMU_SMSVDP, "write register 6 set base tiles = 0x%03X (%d)\n", cpn->r6_sprite_base_tiles, cpn->r6_sprite_base_tiles);
            break;

        case 7:
            cpn->r7_bordercolor = 16 + (data & 0x0F);
            log4me_debug(LOG_EMU_SMSVDP, "write register 7 set border color %d\n", cpn->r7_bordercolor);
            break;

        case 8:
            cpn->r8_scrollx = data;
            log4me_debug(LOG_EMU_SMSVDP, "write register 8 set x scrolling : 0x%02X\n", data);
            break;

        case 9:
            cpn->r9_scrolly = data;
            log4me_debug(LOG_EMU_SMSVDP, "write register 9 set y scrolling : 0x%02X\n", data);
            break;

        case 10:
            cpn->r10_value = data;
            log4me_debug(LOG_EMU_SMSVDP, "write register 10 set hsync int 0x%02X (%d)\n", data, data);
            break;

        default:
            log4me_warning(LOG_EMU_SMSVDP, "write register %d not implemented : 0x%02X\n", reg, data);
            break;
    }

    if(mode240selected(cpn)) {
        log4me_error(LOG_EMU_SMSVDP, "Unable to set video mode 4 with 240 lines => not implemented\n");
        exit(EXIT_FAILURE);
    }
}

void tms9918a_setcolor(tms9918a *cpn, int clr, byte value)
{
    word ggcolor;
#ifdef DEBUG
    SDL_Color color;
#endif

    cpn->cram[clr] = value;

    switch(cpn->gconsole) {
        case GC_SMS:
            assert(clr<32);
#ifdef DEBUG
            color.r =  (value & 0x03)       * 85;
            color.g = ((value & 0x0C) >> 2) * 85;
            color.b = ((value & 0x30) >> 4) * 85;
            log4me_debug(LOG_EMU_SMSVDP, "set color %d = 0x%02X (r:%d, g:%d b:%d)\n", clr, value, color.r, color.g, color.b);
#endif
            cpn->sdlcolors[clr] = SDL_MapRGB(cpn->pixelfmt, (value & 0x03) * 85/*r*/, ((value & 0x0C) >> 2) * 85/*g*/, ((value & 0x30) >> 4) * 85/*b*/);
            break;
        case GC_GG:
            assert(clr<64);
            ggcolor = *(((word*)cpn->cram) + (clr >> 1));
#ifdef DEBUG
            color.r =  (value & 0x000F)       * 17;
            color.g = ((value & 0x00F0) >> 4) * 17;
            color.b = ((value & 0x0F00) >> 8) * 17;
            log4me_debug(LOG_EMU_SMSVDP, "set color %d = (r:%d, g:%d b:%d)\n", clr>>1, color.r, color.g, color.b);
#endif
            cpn->sdlcolors[clr>>1] = SDL_MapRGB(cpn->pixelfmt, (ggcolor & 0x000F) * 17/*r*/, ((ggcolor & 0x00F0) >> 4) * 17/*g*/, ((ggcolor & 0x0F00) >> 8) * 17/*b*/);
            break;
        default:
            assert((cpn->gconsole==GC_SMS) || (cpn->gconsole==GC_GG));
            break;
    }
}

void tms9918a_writeop(tms9918a *cpn, byte port, byte data)
{
    if(cpn->flagsetop & 1) {
        cpn->op.bytes.msb = data;
        cpn->curoperation = cpn->op.newoperation & 0xC000;

        if(cpn->curoperation==TMS_WRITE_REGISTER) {
            tms9918a_setregister(cpn, cpn->op.bytes.msb & 0x0F, cpn->op.bytes.lsb);
        } else {
            switch(cpn->curoperation) {
                case TMS_WRITE_COLOR:
                    cpn->color_index = cpn->op.bytes.lsb & cpn->crammask;
                    log4me_debug(LOG_EMU_SMSVDP, "set color index : %d - set cram index : %d\n", cpn->gconsole==GC_GG ? cpn->op.bytes.lsb >> 1 : cpn->op.bytes.lsb, cpn->op.bytes.lsb);
                    break;
                case TMS_READ_RAM:
                    // => Texas Instruments TMS9918A VDP / Almost complete description including undocumented features By Sean Young (sean@msxnet.org)
                    // What happens internally in the VDP is interesting. This depends on what you set the R/W bit to:
                    //
                    // Write (1):	Simply set the r/w address to the A* value specified.
                    // Read (0):	Read the contents of the A* address from the VRAM and put it
                    //              in the read-ahead buffer. Set the r/w address to A* + 1.
                    //
                    // Now that the r/w address is set, we can start reading from or writing to it.
                    // After each access it is increased. By reading port #0 we can read from the
                    // address, by writing to it we write to the address. What happens internally is:
                    //
                    // Read:	Return value in read-ahead buffer. Read next value from the r/w
                    //          address, put it in the read-ahead buffer. Increase the r/w address.
                    // Write:	Write back the value to the r/w address, then increase the r/w address.
                    //          Interestingly, the value written is also stored in the read-ahead buffer.
                    //
                    // After 3fffh (16kB limit) the r/w address wraps to zero.
                    cpn->vram_addr = cpn->op.newoperation & 0x3FFF;
                    log4me_debug(LOG_EMU_SMSVDP, "read data at : 0x%04X\n", cpn->vram_addr);
                    cpn->read_buffer = cpn->vram[cpn->vram_addr++];
                    cpn->vram_addr &= 0x3FFF;
                    break;
                case TMS_WRITE_RAM:
                    cpn->vram_addr = cpn->op.newoperation & 0x3FFF;
                    log4me_debug(LOG_EMU_SMSVDP, "write data at : 0x%04X\n", cpn->vram_addr);
                    break;
                default:
                    log4me_error(LOG_EMU_SMSVDP, "operation error\n");
                    exit(EXIT_FAILURE);
                    break;
            }
        }
    } else
        cpn->op.bytes.lsb = data;

    cpn->flagsetop++;
}

byte tms9918a_readstatus(tms9918a *cpn, byte port)
{
    byte curstatus = cpn->status;
    if(bit7_is_set(cpn->status))
        cpn->status &= TMS_STATUS_RESET_FLAGS; // Reset bits only on vsync int
    cpn->flagsetop = 0;
    log4me_debug(LOG_EMU_SMSVDP, "read status : 0x%02X\n", curstatus);
    // => Software Reference Manual for the SEGA Mark III Console
    // Reading the I/O part at $BF does two things. First, it clears the interrupt request line from the VDP chip...
    cpn->vsyncint = cpn->hsyncint = 0;
    return curstatus;
}

void tms9918a_writedata(tms9918a *cpn, byte port, byte data)
{
    cpn->flagsetop = 0;

    if(cpn->curoperation==TMS_WRITE_COLOR) {
        assert((cpn->crammask==0x1F) || (cpn->crammask==0x3F));
        tms9918a_setcolor(cpn, cpn->color_index, data);
        cpn->color_index = (cpn->color_index + 1) & cpn->crammask;
        return;
    }

#ifdef DEBUG
    if(cpn->curoperation!=TMS_WRITE_RAM)
        log4me_warning(LOG_EMU_SMSVDP, "write vram mode not selected (vram = %04X <= %02X)\n", cpn->vram_addr, data);
#endif

    assert(cpn->vram_addr<TMS_VRAM_SIZE);

    assert((cpn->vram_addr >> 5)<512);
    cpn->tiles[cpn->vram_addr >> 5] = NULL; // Clear pre-calculate tiles

    cpn->read_buffer = data;
    cpn->vram[cpn->vram_addr++] = data;
    cpn->vram_addr &= 0x3FFF;
}

byte tms9918a_readdata(tms9918a *cpn, byte port)
{
    cpn->flagsetop = 0;
    log4me_debug(LOG_EMU_SMSVDP, "read data\n");

#ifdef DEBUG
    if(cpn->curoperation!=TMS_READ_RAM)
        log4me_warning(LOG_EMU_SMSVDP, "read vram mode not selected\n");
#endif

    assert(cpn->vram_addr<TMS_VRAM_SIZE);

    register byte ret = cpn->read_buffer;
    cpn->read_buffer = cpn->vram[cpn->vram_addr++];
    cpn->vram_addr &= 0x3FFF;
    return ret;
}

byte tms9918a_getscanline(tms9918a *cpn, byte port)
{
    const byte *lkp_vcounter = NULL;
    byte sl;

    assert((cpn->screenheight==192) || (cpn->screenheight==224));
    switch(cpn->vmode) {
        case VM_NTSC:
            if(cpn->screenheight==192)
                lkp_vcounter = lkp_vcounter_ntsc192;
            else
                lkp_vcounter = lkp_vcounter_ntsc224;
            break;
        case VM_PAL:
            if(cpn->screenheight==192)
                lkp_vcounter = lkp_vcounter_pal192;
            else
                lkp_vcounter = lkp_vcounter_pal224;
            break;
    }

    assert(lkp_vcounter);
    sl = cpn->scanline==0 ? lkp_vcounter[cpn->internalscanlines-1] : lkp_vcounter[cpn->scanline-1];
    log4me_debug(LOG_EMU_SMSVDP, "read scanline %d = %d (0x%02X)\n", cpn->scanline, sl, sl);
    return sl;
}

int tms9918a_getmonitorscanlines(tms9918a *cpn)
{
    return cpn->internalscanlines;
}

int tms9918a_getslpersecond(tms9918a *cpn)
{
    return tms9918a_getmonitorscanlines(cpn) * cpn->framerate;
}

static void tms9918a_decode_tile(tms9918a *cpn, int tile)
{
    int i, j;
    byte data0, data1, data2, data3, mask, color;
    byte *tile_data, *writeb;

    tile_data = cpn->vram + TMS_VRAM_TILES_OFS + (tile << 5) /* x32 */;
    writeb = (byte*)cpn->tiles_desc[tile];
    for(j=0;j<8;j++,tile_data+=4) {
        data0 = tile_data[0];
        data1 = tile_data[1];
        data2 = tile_data[2];
        data3 = tile_data[3];
        for(i=0,mask=0x80;i<8;i++,mask>>=1) {
            color = ((data0 & mask)!=0) | (((data1 & mask)!=0) << 1) | (((data2 & mask)!=0) << 2) | (((data3 & mask)!=0) << 3);
            *writeb++ = color;
        }
    }
}

static byte *tms9918a_gettilepixels(tms9918a *cpn, int tile)
{
    assert(tile<512);
    byte *pixels = (byte*)cpn->tiles[tile];
    if(pixels==NULL) {
        tms9918a_decode_tile(cpn, tile);
        pixels = (byte*)cpn->tiles_desc[tile];
        cpn->tiles[tile] = (dword*)pixels;
    }
    return pixels;
}

static void tms9918a_drawzoomedsprites(tms9918a *cpn, word *draw)
{
    byte *tilepixels, *sprites, color, tilemask, spritescollide[256];
    Uint32 *palette;
    int i, j, sx, sy, sw, tile, spritesonscanline;

    memset(spritescollide, 0, sizeof(spritescollide));
    spritesonscanline = 0;
    sprites = cpn->vram + cpn->r5_spattrbaseaddr;
    palette = cpn->sdlcolors + 16;

    // => Sega Master System VDP documentation by Charles MacDonald
    // When bit 1 of register #1 is set, bit 0 of the pattern index is ignored.
    // ex : GG - Road Rash
    tilemask = cpn->r1_8x16sprite ? 0xFE : 0xFF;

    for(i=0;i<64;i++) {
        sy = (int)sprites[i];
        // => Sega Master System VDP documentation by Charles MacDonald
        // If the Y coordinate is set to $D0, then the sprite in question and all remaining sprites of the 64 available will not be drawn. This only works
        // in the 192-line display mode, in the 224 and 240-line modes a Y coordinate of $D0 has no special meaning.
        if((sy==0xD0) && !mode224selected(cpn)) break;
        if(sy<240) {
            // => Sega Master System VDP documentation by Charles MacDonald
            // The Y coordinate is treated as being plus one, so a value of zero would
            // place a sprite on scanline 1 and not scanline zero.
            sy++;
        } else
            sy = sy - 255;

        if((sy>cpn->scanline) || ((sy+(cpn->r1_8x16sprite?32:16))<=cpn->scanline)) continue;

        tile = cpn->r6_sprite_base_tiles + (sprites[129+i*2] & tilemask);
        if(cpn->r1_8x16sprite && ((cpn->scanline-sy)>=16)) {
            sy += 16;
            tile++;
        }

        tilepixels = tms9918a_gettilepixels(cpn, tile) + (((cpn->scanline-sy) >> 1) << 3);
        sx = sprites[128+i*2];
        sw = sx + (((sx+16)>=256) ? 256-sx : 16);

        for(j=sx;j<sw;j+=2) {
            color = *tilepixels++;

            if(color!=0) {
                if(*(word*)(spritescollide + j)) {
                    cpn->status |= TMS_STATUS_SPRITES_COLLIDE;
                } else {
                    *(word*)(spritescollide + j) = 0x0101; // spritescollide[j] = 1; spritescollide[j+1] = 1;
                    draw[j] = draw[j+1] = palette[color];
                }
            }
        }

        // This process is terminated when eight sprites have been displayed on the current scanline
        if(++spritesonscanline>8) {
            cpn->status |= TMS_STATUS_SPRITES_OVERFLOW;
            break;
        }
    }
}

static void tms9918a_drawsprites(tms9918a *cpn, word *draw)
{
    byte *tilepixels, *sprites, color, tilemask, spritescollide[256];
    Uint32 *palette;
    int i, j, sx, sy, sw, tile, spritesonscanline;

    memset(spritescollide, 0, sizeof(spritescollide));
    spritesonscanline = 0;
    sprites = cpn->vram + cpn->r5_spattrbaseaddr;
    palette = cpn->sdlcolors + 16;

    // => Sega Master System VDP documentation by Charles MacDonald
    // When bit 1 of register #1 is set, bit 0 of the pattern index is ignored.
    // ex : GG - Road Rash
    tilemask = cpn->r1_8x16sprite ? 0xFE : 0xFF;

    for(i=0;i<64;i++) {
        sy = (int)sprites[i];
        // => Sega Master System VDP documentation by Charles MacDonald
        // If the Y coordinate is set to $D0, then the sprite in question and all remaining sprites of the 64 available will not be drawn. This only works
        // in the 192-line display mode, in the 224 and 240-line modes a Y coordinate of $D0 has no special meaning.
        if((sy==0xD0) && !mode224selected(cpn)) break;
        if(sy<240) {
            // => Sega Master System VDP documentation by Charles MacDonald
            // The Y coordinate is treated as being plus one, so a value of zero would
            // place a sprite on scanline 1 and not scanline zero.
            sy++;
        } else
            sy = sy - 255;

        if((sy>cpn->scanline) || ((sy+(cpn->r1_8x16sprite?16:8))<=cpn->scanline)) continue;

        tile = cpn->r6_sprite_base_tiles + (sprites[129+i*2] & tilemask);
        if(cpn->r1_8x16sprite && ((cpn->scanline-sy)>=8)) {
            sy += 8;
            tile++;
        }

        tilepixels = tms9918a_gettilepixels(cpn, tile) + ((cpn->scanline-sy) << 3);
        sx = sprites[128+i*2];
        sw = sx + (((sx+8)>=256) ? 256-sx : 8);

        for(j=sx;j<sw;j++) {
            color = *tilepixels++;

            if(color!=0) {
                if(spritescollide[j]) {
                    cpn->status |= TMS_STATUS_SPRITES_COLLIDE;
                } else {
                    spritescollide[j] = 1;
                    draw[j] = palette[color];
                }
            }
        }

        // This process is terminated when eight sprites have been displayed on the current scanline
        if(++spritesonscanline>8) {
            cpn->status |= TMS_STATUS_SPRITES_OVERFLOW;
            break;
        }
    }
}

static void tms9918a_clearline(tms9918a *cpn)
{
    word *draw = cpn->buffer16 + (cpn->scanline << 8); // scanline*256
    Uint32 color = cpn->sdlcolors[cpn->r7_bordercolor];

    assert(cpn->r1_display==0);
    int i;
    for(i=0; i<TILES_WIDTH_SCREEN*8; i++)
        *draw++ = color;

    log4me_debug(LOG_EMU_SMSVDP, "render line %d (clear)\n", cpn->scanline);
}

static void tms9918a_renderline(tms9918a *cpn)
{
    word *tilesdesc, *draw;
    byte *tilepixels;
    Uint32 *palette;
    int i, j, x, y, ft, tile, tileoffset;
    const int *lkpmodulo;
    tms99818a_fgtile foregroundtiles[TILES_WIDTH_SCREEN];

    assert(cpn->r1_display);
    lkpmodulo = mode224selected(cpn) ? lkp_mod256 : lkp_mod224;

    x = (cpn->r0_top2rows_no_hscroll && (cpn->scanline<16)) ? 0 : cpn->r8_scrollx;
    y = lkpmodulo[cpn->scanline + cpn->r9_scrolly];

    tilesdesc = (word*)(cpn->vram + cpn->r2_bgbaseaddr) + (y >> 3)*TILES_WIDTH_SCREEN;
    tileoffset = (y & 0x7) << 3;
    draw = cpn->buffer16 + (cpn->scanline << 8); // scanline*256

    ft = 0; // no foreground tiles to draw

    log4me_debug(LOG_EMU_SMSVDP, "render line %d : scroll X : 0x%02X (%d)\n", cpn->scanline, x, x);

    // Draw background tiles
    for(i=0; i<TILES_WIDTH_SCREEN; i++,tilesdesc++) {
        if((i==TILES_WIDTH_SCREEN-8) && cpn->r0_vert_scroll_inhibit) {
            y = lkpmodulo[cpn->scanline];
            tilesdesc = (word*)(cpn->vram + cpn->r2_bgbaseaddr) + (y >> 3)*TILES_WIDTH_SCREEN + i;
            tileoffset = (y & 0x7) << 3;
        }

        tile = *tilesdesc & 0x01FF;
        palette = *tilesdesc & TMS_BG_ATTR_SPRITE_PAL ? cpn->sdlcolors + 16 : cpn->sdlcolors;

        if(*tilesdesc & 0x1000) { // It's a foreground tile ?
            foregroundtiles[ft].x = x;
            foregroundtiles[ft].offset = tileoffset;
            foregroundtiles[ft++].desc = *tilesdesc;
            for(j=0;j<8;j++,x++) draw[x&0xFF] = palette[0];
            continue;
        }

        switch(*tilesdesc & (TMS_BG_ATTR_FLIPH | TMS_BG_ATTR_FLIPV)) {
            case 0:
                tilepixels = tms9918a_gettilepixels(cpn, tile) + tileoffset;
                for(j=0;j<8;j++,x++) draw[x&0xFF] = palette[*tilepixels++];
                break;
            case TMS_BG_ATTR_FLIPH:
                tilepixels = tms9918a_gettilepixels(cpn, tile) + tileoffset + 7;
                for(j=0;j<8;j++,x++) draw[x&0xFF] = palette[*tilepixels--];
                break;
            case TMS_BG_ATTR_FLIPV:
                tilepixels = tms9918a_gettilepixels(cpn, tile) + 7*8 - tileoffset;
                for(j=0;j<8;j++,x++) draw[x&0xFF] = palette[*tilepixels++];
                break;
            case TMS_BG_ATTR_FLIPH | TMS_BG_ATTR_FLIPV:
                tilepixels = tms9918a_gettilepixels(cpn, tile) + (7*8+7) - tileoffset;
                for(j=0;j<8;j++,x++) draw[x&0xFF] = palette[*tilepixels--];
                break;
        }
    }

    // Draw sprites
    if(cpn->r1_zoomedsprites)
        tms9918a_drawzoomedsprites(cpn, draw);
    else
        tms9918a_drawsprites(cpn, draw);

    // Draw foreground tiles with transparence
    for(i=0; i<ft; i++) {
        tile = foregroundtiles[i].desc & 0x01FF;
        palette = foregroundtiles[i].desc & TMS_BG_ATTR_SPRITE_PAL ? cpn->sdlcolors + 16 : cpn->sdlcolors;

        x = foregroundtiles[i].x;
        tileoffset = foregroundtiles[i].offset;

        switch(foregroundtiles[i].desc & (TMS_BG_ATTR_FLIPH | TMS_BG_ATTR_FLIPV)) {
            case 0:
                tilepixels = tms9918a_gettilepixels(cpn, tile) + tileoffset;
                for(j=0;j<8;j++,x++) {
                    if(*tilepixels) draw[x&0xFF] = palette[*tilepixels];
                    tilepixels++;
                }
                break;
            case TMS_BG_ATTR_FLIPH:
                tilepixels = tms9918a_gettilepixels(cpn, tile) + tileoffset + 7;
                for(j=0;j<8;j++,x++) {
                    if(*tilepixels) draw[x&0xFF] = palette[*tilepixels];
                    tilepixels--;
                }
                break;
            case TMS_BG_ATTR_FLIPV:
                tilepixels = tms9918a_gettilepixels(cpn, tile) + 7*8 - tileoffset;
                for(j=0;j<8;j++,x++) {
                    if(*tilepixels) draw[x&0xFF] = palette[*tilepixels];
                    tilepixels++;
                }
                break;
            case TMS_BG_ATTR_FLIPH | TMS_BG_ATTR_FLIPV:
                tilepixels = tms9918a_gettilepixels(cpn, tile) + (7*8+7) - tileoffset;
                for(j=0;j<8;j++,x++) {
                    if(*tilepixels) draw[x&0xFF] = palette[*tilepixels];
                    tilepixels--;
                }
                break;
        }
    }

    if(cpn->r0_do_not_display_leftmost_col) {
        Uint32 c = cpn->sdlcolors[cpn->r7_bordercolor];
        for(i=0;i<8;i++) *draw++ = c;
    }

    //if(cpn->slcounter==0) draw[255] = 0xFFFF;
    /*if(cpn->slcounter==0) {
        draw = cpn->buffer16 + (cpn->scanline << 8); // scanline * 256
        for(i=0;i<256;i++) draw[i] = 0xFFFF;
    }*/
}

static void tms9918a_flip(tms9918a *cpn)
{
    Uint8 r, g, b;
    SDL_Rect dstrect;
    word *buffer = cpn->buffer16;

    dstrect.x = cpn->screen->marginx;
    dstrect.y = cpn->screen->marginy;
    dstrect.w = cpn->screen->width * cpn->screen->scale;
    dstrect.h = cpn->screen->height * cpn->screen->scale;

    if(cpn->gconsole==GC_GG)
        buffer += ((cpn->ggrect->y << 8) + cpn->ggrect->x);

    SDL_UpdateTexture(cpn->picture, NULL, buffer, 256*sizeof(word));

    SDL_GetRGB(cpn->gconsole==GC_GG ? 0 : cpn->sdlcolors[cpn->r7_bordercolor], cpn->pixelfmt, &r, &g, &b);
    SDL_SetRenderDrawColor(cpn->screen->renderer, r, g, b, 255);
    SDL_RenderClear(cpn->screen->renderer);

    SDL_RenderCopy(cpn->screen->renderer, cpn->picture, NULL, &dstrect);

#ifdef DEBUG
    if(cpn->showpalette) {
        int i;
        SDL_Rect colrect;
        colrect.y = 0;
        colrect.w = (cpn->screen->width * cpn->screen->scale) / TMS_COLORS;
        colrect.h = 8 * cpn->screen->scale;
        for(i=0;i<TMS_COLORS;i++) {
            colrect.x = (colrect.w * i) + cpn->screen->marginx;
            SDL_GetRGB(cpn->sdlcolors[i], cpn->pixelfmt, &r, &g, &b);
            SDL_SetRenderDrawColor(cpn->screen->renderer, r, g, b, 255);
            SDL_RenderFillRect(cpn->screen->renderer, &colrect);
        }
    }
#endif

    SDL_RenderPresent(cpn->screen->renderer);
}

void tms9918a_waitnextframe(tms9918a *cpn)
{
    clock_wait(cpn->idclock);
}

void tms9918a_execute(tms9918a *cpn)
{
    assert((cpn->screenheight==192) || (cpn->screenheight==224));

    if(cpn->scanline==0) {
        int oldscreenheight = cpn->screenheight;
        // Reload hsync interrupt counter
        cpn->slcounter = cpn->r10_value;
        cpn->screenheight = mode224selected(cpn) ? 224 : 192;
        if(cpn->screenheight!=oldscreenheight) {
            if(cpn->gconsole==GC_SMS) {
                SDL_DestroyTexture(cpn->picture);
                cpn->picture = SDL_CreateTexture(cpn->screen->renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, 256, cpn->screenheight);
            } else {
                assert(cpn->gconsole==GC_GG);
                cpn->ggrect = cpn->screenheight==224 ? &ggrect224 : &ggrect192;
            }
        }
        log4me_debug(LOG_EMU_SMSVDP, "start new frame (%d) - %d lines\n", cpn->frame, cpn->screenheight);
    }

    if(cpn->scanline<cpn->screenheight) {
        if(cpn->r1_display)
            tms9918a_renderline(cpn);
        else
            tms9918a_clearline(cpn);
    }

    if((cpn->scanline<=cpn->screenheight) && (--cpn->slcounter<0)) {
        cpn->slcounter = cpn->r10_value;
        if(cpn->r0_hsync_int) {
            cpn->hsyncint = 1;
            log4me_debug(LOG_EMU_SMSVDP, "hsync int, scanline=%d\n", cpn->scanline);
        }
    }

    if(cpn->scanline==cpn->screenheight+1) {
        cpn->status |= TMS_STATUS_VSYNC;
        if(cpn->r1_vsync_int) {
            cpn->vsyncint = 1;
            log4me_debug(LOG_EMU_SMSVDP, "vsync int\n");
        }
        tms9918a_flip(cpn);
        cpn->frame++;
    }

    if(++cpn->scanline==cpn->internalscanlines) {
        cpn->scanline = 0;
        clock_step(cpn->idclock);
    }
}

int tms9918a_int_pending(tms9918a *cpn)
{
    return cpn->vsyncint || cpn->hsyncint;
}

SDL_Surface *tms9918a_takescreenshot(const tms9918a *cpn)
{
    int width, height;
    word *buffer;

    switch(cpn->gconsole) {
        case GC_GG :
            width = GG_SCREEN_WIDTH;
            height = GG_SCREEN_HEIGHT;
            buffer = cpn->buffer16 + ((cpn->ggrect->y << 8) + cpn->ggrect->x);
            break;
        case GC_SMS : default :
            width = 256;
            height = cpn->screenheight;
            buffer = cpn->buffer16;
            break;
    }

    return SDL_CreateRGBSurfaceFrom(buffer, width, height, cpn->pixelfmt->BitsPerPixel, 256*sizeof(word),
                        cpn->pixelfmt->Rmask, cpn->pixelfmt->Gmask, cpn->pixelfmt->Bmask, cpn->pixelfmt->Amask);
}

void tms9918a_takesnapshot(tms9918a *cpn, xmlTextWriterPtr writer)
{
    int i;

    xmlTextWriterStartElement(writer, BAD_CAST "tms99xx");
        xmlTextWriterStartElement(writer, BAD_CAST "registers");
        for(i=0;i<TMS_REGISTERS;i++) {
            xmlTextWriterStartElement(writer, BAD_CAST "register");
            xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "id", "%d", i);
            xmlTextWriterWriteFormatString(writer, "%02X", cpn->registers[i]);
            xmlTextWriterEndElement(writer); /* register */
        }
        xmlTextWriterEndElement(writer); /* registers */

        xmlTextWriterStartElement(writer, BAD_CAST "curoperation");
        xmlTextWriterWriteFormatString(writer, "%04X", cpn->curoperation);
        xmlTextWriterEndElement(writer); /* curoperation */

        xmlTextWriterStartElement(writer, BAD_CAST "newoperation");
        xmlTextWriterWriteFormatString(writer, "%04X", cpn->op.newoperation);
        xmlTextWriterEndElement(writer); /* newoperation */

        xmlTextWriterStartElement(writer, BAD_CAST "flagsetop");
        xmlTextWriterWriteFormatString(writer, "%04X", cpn->flagsetop);
        xmlTextWriterEndElement(writer); /* flagsetop */

        xmlTextWriterStartElement(writer, BAD_CAST "color_index");
        xmlTextWriterWriteFormatString(writer, "%04X", cpn->color_index);
        xmlTextWriterEndElement(writer); /* color_index */

        xmlTextWriterStartElement(writer, BAD_CAST "vram_addr");
        xmlTextWriterWriteFormatString(writer, "%04X", cpn->vram_addr);
        xmlTextWriterEndElement(writer); /* vram_addr */

        xmlTextWriterStartElement(writer, BAD_CAST "read_buffer");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->read_buffer);
        xmlTextWriterEndElement(writer); /* read_buffer */

        xmlTextWriterStartElement(writer, BAD_CAST "status");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->status);
        xmlTextWriterEndElement(writer); /* status */

        xmlTextWriterStartElement(writer, BAD_CAST "vsyncint");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->vsyncint);
        xmlTextWriterEndElement(writer); /* vsyncint */

        xmlTextWriterStartElement(writer, BAD_CAST "hsyncint");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->hsyncint);
        xmlTextWriterEndElement(writer); /* hsyncint */

        xmlTextWriterStartElement(writer, BAD_CAST "colors");
            for(i=0;i<cpn->cramsize;i++) {
                xmlTextWriterStartElement(writer, BAD_CAST "color");
                xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "id", "%d", i);
                xmlTextWriterWriteFormatString(writer, "%02X", cpn->cram[i]);
                xmlTextWriterEndElement(writer); /* color */
            }
        xmlTextWriterEndElement(writer); /* colors */

        xmlTextWriterWriteArray(writer, "vram", cpn->vram, TMS_VRAM_SIZE);
    xmlTextWriterEndElement(writer); /* tms99xx */
}

void tms9918a_loadsnapshot(tms9918a *cpn, xmlNode *vdpnode)
{
    XML_ENUM_CHILD(vdpnode, node,
        XML_ELEMENT_ENUM_CHILD("registers", node, child,
            XML_ELEMENT_CONTENT("register", child,
                tms9918a_setregister(cpn, atoi((const char*)xmlGetProp(child, BAD_CAST "id")), (byte)strtoul((const char*)content, NULL, 16));
            )
        )

        XML_ELEMENT_CONTENT("curoperation", node,
            cpn->curoperation = (word)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("newoperation", node,
            cpn->op.newoperation = (word)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("flagsetop", node,
            cpn->flagsetop = (dword)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("color_index", node,
            cpn->color_index = (int)strtol((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("vram_addr", node,
            cpn->vram_addr = (dword)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("read_buffer", node,
            cpn->read_buffer = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("status", node,
            cpn->status = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("vsyncint", node,
            cpn->vsyncint = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("hsyncint", node,
            cpn->hsyncint = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_ENUM_CHILD("colors", node, color,
            XML_ELEMENT_CONTENT("color", color,
                tms9918a_setcolor(cpn, atoi((const char*)xmlGetProp(color, BAD_CAST "id")), (byte)strtoul((const char*)content, NULL, 16));
            )
        )

        XML_ELEMENT("vram", node,
            xmlNodeReadArray(node, cpn->vram, TMS_VRAM_SIZE);
        )
    )
}

#ifdef DEBUG
void tms9918a_save_tiles(tms9918a *cpn, const string filename)
{
    #define WIDTH   (32*(8+1) + 1)
    #define HEIGHT  (16*(8+1) + 1)

    int i, j, curtile, width;
    word *draw;
    byte *tilepixels;
    SDL_Surface *tiles = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH, 2*HEIGHT, 16, 0, 0, 0, 0);

    width = tiles->pitch / tiles->format->BytesPerPixel;

    SDL_FillRect(tiles, NULL, cpn->sdlcolors[cpn->r7_bordercolor]);

    for(curtile=0; curtile<512; curtile++) {
        draw = (word*)tiles->pixels + (curtile >> 5)*width*9 + 1*width + (curtile&0x1F)*9 + 1;
        tilepixels = tms9918a_gettilepixels(cpn, curtile);

        for(i=0; i<8; i++) {
            for(j=0; j<8; j++) {
                draw[j] = cpn->sdlcolors[*tilepixels++];
            }
            draw += width;
        }
    }

    for(curtile=0; curtile<512; curtile++) {
        draw = (word*)tiles->pixels + HEIGHT*width + (curtile >> 5)*width*9 + 1*width + (curtile&0x1F)*9 + 1;
        tilepixels = tms9918a_gettilepixels(cpn, curtile);

        for(i=0; i<8; i++) {
            for(j=0; j<8; j++) {
                draw[j] = cpn->sdlcolors[16+*tilepixels++];
            }
            draw += width;
        }
    }

    SDL_SaveBMP(tiles, CSTR(filename));
    SDL_FreeSurface(tiles);
}

void tms9918a_toggledisplaypalette(tms9918a *cpn)
{
    cpn->showpalette ^= 1;
}
#endif
