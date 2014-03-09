#ifndef SMS_H_INCLUDED
#define SMS_H_INCLUDED

#include <SDL.h>

#include "cpu/z80.h"
#include "tms9918a.h"
#include "sn76489.h"
#include "ym2413.h"

#include "rom.h"
#include "misc/string.h"

#define MS_MEM_SIZE         0x2000    /*  8K */
#define MS_PAGE_SIZE        0x4000    /* 16K */
#define MS_CARTRIDGE_RAM    0x8000    /* 32K */
#define MS_REGISTERS        4

#define MS_PAGEROM_START    0x0000
#define MS_PAGEROM_END      0xBFFF
#define MS_RAM_START        0xC000
#define MS_RAM_END          0xDFFF

#define MS_ADDR_RAM_SEL_REG 0xFFFC
#define MS_ADDR_SET_PAGE2   0xFFFF


struct _iomap;
typedef struct _iomap iomap;

typedef struct _mastersystem {
    cpuZ80 *z80;

    byte *rdmap[8]; // 8*8Ko = 64Ko
    byte *wrmap[8]; // 8*8Ko = 64Ko
    byte mem[MS_MEM_SIZE];
    byte mnull[8192];
    byte mregisters[MS_REGISTERS]; // 0xFFFC-0xFFFF
    byte cmregister;
    byte cartridgeram[MS_CARTRIDGE_RAM]; // On-board cartridge RAM
    iomap *iomapper;

    byte *rom;
    char *romname;
    long romlen;
    int rombanks;
    const romspecs *rspecs;

    byte port_3e; // controls the memory enables
    byte port_3f; // automatic nationalisation
    byte port_dc; // joypad port 1
    byte port_dd; // joypad port 2
    byte port_f2; // detect ym2413

    int cartridgeram_detected;

    tms9918a vdp;
    sn76489 snd;
    ym2413  fmsnd;

    int clock;
    int cpu;

    int *lkptsps; // Lookup table tstates per scanline for 1 second
    int scanlinespersecond;

    int joystick1;
    int joystick2;
    int pause;

    string backupdir;
    sdlclock idclock1s;

} mastersystem;


mastersystem* ms_init(const display *screen, const romspecs *rspecs, sound_onoff playsound, int joypad1, int joypad2, string backupdir);

void ms_start(mastersystem *sms);
void ms_execute(mastersystem *sms);
void ms_pause(mastersystem *sms, int pause);
int ms_ispaused(mastersystem *sms);

void sms_takesnapshot(mastersystem *sms, xmlTextWriterPtr writer);

#endif // SMS_H_INCLUDED
