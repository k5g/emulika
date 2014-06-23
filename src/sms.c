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

#include <stdlib.h>

#include "sms.h"
#include "clock.h"
#include "emul.h"
#include "environ.h"
#include "input.h"
#include "misc/log4me.h"
#include "misc/exit.h"
#include "misc/xml.h"

#define CPU_CLOCK_NTSC  3579545 // Hz for NTSC systems
#define CPU_CLOCK_PAL   3546893 // Hz for PAL/SECAM

#define SEGA_MS_TAG    "TMR SEGA"

#define JOYPAD1_DC_TR      0x20
#define JOYPAD1_DC_TL      0x10
#define JOYPAD1_DC_RIGHT   0x08
#define JOYPAD1_DC_LEFT    0x04
#define JOYPAD1_DC_DOWN    0x02
#define JOYPAD1_DC_UP      0x01
#define JOYPAD1_DC_INIT    0xFF

#define JOYPAD2_DD_TR      0x08
#define JOYPAD2_DD_TL      0x04
#define JOYPAD2_DD_RIGHT   0x02
#define JOYPAD2_DD_LEFT    0x01
#define JOYPAD2_DC_DOWN    0x80
#define JOYPAD2_DC_UP      0x40
#define JOYPAD2_DD_INIT    0xFF

#define JOYPAD2_DD_RESET   0x10

#define GG_START_PAUSE     0x80

// => SMS/GG hardware notes by Charles MacDonald
// D7 : Expansion slot enable (1= disabled, 0= enabled)
// D6 : Cartridge slot enable (1= disabled, 0= enabled)
// D5 : Card slot disabled (1= disabled, 0= enabled)
// D4 : Work RAM disabled (1= disabled, 0= enabled)
// D3 : BIOS ROM disabled (1= disabled, 0= enabled)
// D2 : I/O chip disabled (1= disabled, 0= enabled)
// D1 : Unknown
// D0 : Unknown
#define iochip_disable(s)   (bit2_is_set(s->port_3e))

typedef byte (*readiofunc)(void *obj, byte port);
typedef void (*writeiofunc)(void *obj, byte port, byte data);

struct _iomap {
    void *rdobj;
    readiofunc readio;
    void *wrobj;
    writeiofunc writeio;
};

static void sms_updatejoypads(mastersystem *sms);
static string sms_getbackupfilename(mastersystem *sms);
static string sms_geteepromfilename(mastersystem *sms);
static void sms_loadrom(mastersystem *sms);
static void sms_loadsnapshot(mastersystem *sms, xmlDocPtr doc);

static byte ms_readmemory(void* param, dword address)
{
    mastersystem *sms = (mastersystem*)param;

    if(address<0x0400) return sms->rom[address];
    if(address>=0xFFFC) return sms->mregisters[address&0x03];

    return sms->rdmap[address>>13][address&0x1FFF];
}

static void ms_writemregister(mastersystem *sms, int reg, byte data)
{
    assert(sms->rombanks>0);

    sms->mregisters[reg] = data;

    switch(reg) {
        case 0 :
            switch(getrommemorymapper(sms->rspecs)) {
                case MM_SEGA:
                    if(bit3_is_set(data)) {
                        log4me_debug(LOG_EMU_SMS, "load page 2 = Cartridge RAM page %d\n", bit2_is_set(data) ? 1 : 0);
                        sms->rdmap[4] = sms->cartridgeram + (bit2_is_set(data) ? MS_PAGE_SIZE : 0); // 8ko
                        sms->rdmap[5] = sms->rdmap[4] + 8192; // 8ko
                        sms->wrmap[4] = sms->rdmap[4];
                        sms->wrmap[5] = sms->rdmap[5];
                        sms->cartridgeram_detected = 1;
                    } else {
                        log4me_debug(LOG_EMU_SMS, "disable on-board cartridge ram\n");
                        sms->wrmap[4] = sms->mnull;
                        sms->wrmap[5] = sms->mnull;
                        ms_writemregister(sms, 3, sms->mregisters[3]);
                    }
                    break;

                case MM_SEGA_EEPROM:
                    log4me_debug(LOG_EMU_SMS, "EEPROM %s\n", bit3_is_set(data) ? "enable" : "disable");
                    if(bit7_is_set(data))
                        seeprom_init(sms->mc93c46);
                    break;

                default:
                    assert((getrommemorymapper(sms->rspecs)!=MM_SEGA) && (getrommemorymapper(sms->rspecs)!=MM_SEGA_EEPROM));
                    break;
            }
            break;

        case 1 :
            log4me_debug(LOG_EMU_SMS, "load page 0 = ROM bank %d (0x%02X)\n", data % sms->rombanks, data);
            sms->rdmap[0] = sms->rom + ((data % sms->rombanks) << 14); // 8ko
            sms->rdmap[1] = sms->rdmap[0] + 8192; // 8ko
            break;

        case 2 :
            log4me_debug(LOG_EMU_SMS, "load page 1 = ROM bank %d (0x%02X)\n", data % sms->rombanks, data);
            sms->rdmap[2] = sms->rom + ((data % sms->rombanks) << 14); // 8ko
            sms->rdmap[3] = sms->rdmap[2] + 8192; // 8ko
            break;

        case 3 :
            if(!bit3_is_set(sms->mregisters[0])) { // Terminator
                log4me_debug(LOG_EMU_SMS, "load page 2 = ROM bank %d (0x%02X)\n", data % sms->rombanks, data);
                sms->rdmap[4] = sms->rom + ((data % sms->rombanks) << 14); // 8ko
                sms->rdmap[5] = sms->rdmap[4] + 8192; // 8ko
            } else
                log4me_warning(LOG_EMU_SMS, "cartridge ram selected, load page 2 not possible\n");
            break;

        default:
            log4me_error(LOG_EMU_SMS, "write wrong mem register : %d\n", reg);
            break;
    }
}

static void ms_writememory(void* param, dword address, byte data)
{
    mastersystem *sms = (mastersystem*)param;

    sms->wrmap[address>>13][address&0x1FFF] = data;
    if(address>=0xFFFC)
        ms_writemregister(sms, address & 0x03, data);

#ifdef DEBUG
    if((address<0x8000) || ((address<0xC000) && !bit3_is_set(sms->mregisters[0])))
        log4me_warning(LOG_EMU_SMS, "write wrong address 0x%04X = 0x%02X\n", address, data);
#endif
}

static byte ms_readmemory_segaeeprom(void* param, dword address)
{
    mastersystem *sms = (mastersystem*)param;

    if(address<0x0400) return sms->rom[address];
    if(address>=0xFFFC) return sms->mregisters[address&0x03];

    if((address==0x8000) && bit3_is_set(sms->mregisters[0])) {
        byte cs, dout, v;
        seeprom_getlines(sms->mc93c46, &cs, NULL, &dout);
        v = (cs << 2) | dout;
        v |= 0x02; // ??
        return v;
    }

    return sms->rdmap[address>>13][address&0x1FFF];
}

static void ms_writememory_segaeeprom(void* param, dword address, byte data)
{
    mastersystem *sms = (mastersystem*)param;

    if((address==0x8000) && bit3_is_set(sms->mregisters[0])) {
        seeprom_setlines(sms->mc93c46, bit2_is_set(data)/*cs*/, bit1_is_set(data)/*clk*/, bit0_is_set(data)/*din*/);
        return;
    }

    sms->wrmap[address>>13][address&0x1FFF] = data;
    if(address>=0xFFFC)
        ms_writemregister(sms, address & 0x03, data);

#ifdef DEBUG
    if(address<0xC000)
        log4me_warning(LOG_EMU_SMS, "write wrong address 0x%04X = 0x%02X\n", address, data);
#endif
}

static byte sms_readmemorycodemasters(void* param, dword address)
{
    return ((mastersystem*)param)->rdmap[address>>13][address&0x1FFF];
}

static void sms_writememorycodemasters(void* param, dword address, byte data)
{
    mastersystem *sms = (mastersystem*)param;

    sms->wrmap[address>>13][address&0x1FFF] = data;
    if(address==0x8000) {
        sms->cmregister = data;
        sms->rdmap[4] = sms->rom + ((data % sms->rombanks) << 14); // 8ko
        sms->rdmap[5] = sms->rdmap[4] + 8192; // 8ko
        return;
    }

    if((address==0x4000) && (data!=1)) {
        log4me_error(LOG_EMU_SMS, "write wrong address 0x%04X = 0x%02X\n", address, data);
        exit(EXIT_FAILURE);
    }

    if((address==0x0000) && (data!=0)) {
        log4me_error(LOG_EMU_SMS, "write wrong address 0x%04X = 0x%02X\n", address, data);
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    if(address<0xC000)
        log4me_warning(LOG_EMU_SMS, "write wrong address 0x%04X = 0x%02X\n", address, data);
#endif
}

static byte ms_readio(void* param, byte port)
{
    mastersystem *sms = (mastersystem*)param;
    iomap *iom = sms->iomapper + port;

    if(iom->readio!=NULL)
        return iom->readio(iom->rdobj, port);

    log4me_warning(LOG_EMU_SMS, "read wrong port : 0x%02X (%d)\n", port, port);
    //exit(EXIT_FAILURE);
    return 0xFF;
}

static byte sms_readioggstartpause(mastersystem *sms, byte port)
{
    sms_updatejoypads(sms);
    return sms->ports[0];
}

static byte sms_readiogg(mastersystem *sms, byte port)
{
    log4me_debug(LOG_EMU_SMS, "read I/O port %d = %02X (%d)\n", port, sms->ports[port], sms->ports[port]);
    return sms->ports[port];
}

static byte sms_readiojoypad1(mastersystem *sms, byte port)
{
    /* => SMS/GG hardware notes by Charles MacDonald
    D7 : Port B DOWN pin input
    D6 : Port B UP pin input
    D5 : Port A TR pin input
    D4 : Port A TL pin input
    D3 : Port A RIGHT pin input
    D2 : Port A LEFT pin input
    D1 : Port A DOWN pin input
    D0 : Port A UP pin input    */
    sms_updatejoypads(sms);
    //log4me_debug(LOG_SMS_CORE, "[SMS] read joystick 1 port 0x%02X = 0x%02X (%d)\n", port, sms->port_dc, sms->port_dc);
    return sms->port_dc;
}

static byte sms_readiojoypad2(mastersystem *sms, byte port)
{
    /* => SMS/GG hardware notes by Charles MacDonald
    D7 : Port B TH pin input
    D6 : Port A TH pin input
    D5 : Unused
    D4 : RESET button (1= not pressed, 0= pressed)
    D3 : Port B TR pin input
    D2 : Port B TL pin input
    D1 : Port B RIGHT pin input
    D0 : Port B LEFT pin input

    Bit 5 returns 0 on a Genesis and 1 on an SMS, SMS 2 and GG.
    Bit 4 always returns 1 on a Genesis and GG which have no RESET button. */
    sms_updatejoypads(sms);
    //log4me_debug(LOG_SMS_CORE, "[SMS] read joystick 2 port 0x%02X = 0x%02X (%d)\n", port, sms->port_dd, sms->port_dd);
    return sms->port_dd;
}

static byte sms_readioF2(mastersystem *sms, byte port)
{
    // YM2413 available only on japan machine
    assert(iochip_disable(sms));
    // => SMS/GG hardware notes by Charles MacDonald
    // Port $F2 : Bit 0 can be read and written to detect if YM2413 is available.
    // The SMS normally assigns reads from $C0-$FF to the I/O chip. You must
    // disable the I/O chip by setting bit 2 of port $3E if you want to read
    // port $F2 to perform YM2413 detection.
    byte data = ~sms->port_f2; // YM2413 not implemented return false value
    log4me_debug(LOG_EMU_SMS, "Read I/O port F2 = %02X (%d) => Try to detect YM2413\n", data, data);
    return data;
    // return sms->machine==JAPAN ? sms->port_f2 : 0xFF;

}

static void ms_writeio(void* param, byte port, byte data)
{
    mastersystem *sms = (mastersystem*)param;
    iomap *iom = sms->iomapper + port;

    if(iom->writeio!=NULL) {
        iom->writeio(iom->wrobj, port, data);
        return;
    }

    log4me_warning(LOG_EMU_SMS, "write wrong port : 0x%02X (%d) - data : 0x%02X (%d)\n", port, port, data, data);
    //exit(EXIT_FAILURE);
}

static void sms_writeionull(mastersystem *sms, byte port, byte data)
{
}

static void sms_writeio3E(mastersystem *sms, byte port, byte data)
{
    sms->port_3e = data;
    log4me_debug(LOG_EMU_SMS, "I/O chip=%d - BIOS ROM=%d - Work RAM=%d - Card slot=%d - Cartridge slot=%d Expansion slot=%d\n",
                   bit2_is_set(data), bit3_is_set(data), bit4_is_set(data), bit5_is_set(data), bit6_is_set(data), bit7_is_set(data));
}

static void sms_writeio3F(mastersystem *sms, byte port, byte data)
{
    sms->port_3f = data;
    assert((bit0_is_set(sms->port_3f) ^ bit2_is_set(sms->port_3f))==0);

    if(bit0_is_set(sms->port_3f) && bit2_is_set(sms->port_3f)) {
        // => SEGA MASTER SYSTEM TECHNICAL INFORMATION by Richard Talbot-Watkins
        // The value actually written to the nationalisation bits depends on the country
        // of origin.  British machines directly copy the values in bits 5 and 7 of port
        // $3F to the nationalisation bits.  Japanese machines appear to complement one
        // or more of these bits, although these details are uncertain.
        switch(getrommachine(sms->rspecs)) {
            case JAPAN:
                if(bit5_is_set(sms->port_3f)) sms->port_dd &= 0xBF; else sms->port_dd |= 0x40;
                if(bit7_is_set(sms->port_3f)) sms->port_dd &= 0x7F; else sms->port_dd |= 0x80;
                break;
            case EXPORT:
                if(bit5_is_set(sms->port_3f)) sms->port_dd |= 0x40; else sms->port_dd &= 0xBF;
                if(bit7_is_set(sms->port_3f)) sms->port_dd |= 0x80; else sms->port_dd &= 0x7F;
                break;
        }
    }

    if(bit0_not_set(sms->port_3f) && bit2_not_set(sms->port_3f)) sms->port_dd |= 0xC0;

    log4me_debug(LOG_EMU_SMS, "write port 0x3F = 0x%02X (%d)\n", data, data);
}

static void sms_writeiogg(mastersystem *sms, byte port, byte data)
{
    log4me_debug(LOG_EMU_SMS, "write port %d = 0x%02X (%d)\n", port, data, data);
    sms->ports[port] = data;
}

static void sms_writeioF2(mastersystem *sms, byte port, byte data)
{
    sms->port_f2 = data;
    log4me_debug(LOG_EMU_SMS, "Write %02X (%d) to port F2 => Try to detect YM2413\n", data, data);
}

static void sms_initiomapper(mastersystem *sms)
{
#define INIT_IO_MAP(p,ro,r,wo,w)  {   \
    sms->iomapper[p].rdobj = ro;  \
    sms->iomapper[p].readio = (readiofunc)r;  \
    sms->iomapper[p].wrobj = wo;  \
    sms->iomapper[p].writeio = (writeiofunc)w; \
}

if(sms->gconsole==GC_GG) {
    INIT_IO_MAP(0x00, sms           , sms_readioggstartpause, NULL          , NULL);
    INIT_IO_MAP(0x01, sms           , sms_readiogg          , sms           , sms_writeiogg);
    INIT_IO_MAP(0x02, sms           , sms_readiogg          , sms           , sms_writeiogg);
    INIT_IO_MAP(0x03, sms           , sms_readiogg          , sms           , sms_writeiogg)
    INIT_IO_MAP(0x04, sms           , sms_readiogg          , NULL          , NULL);
    INIT_IO_MAP(0x05, sms           , sms_readiogg          , sms           , sms_writeiogg);
    INIT_IO_MAP(0x06, NULL          , NULL                  , &sms->snd     , sn76489_writestereo);
}

    INIT_IO_MAP(0x3E, NULL          , NULL                  , sms           , sms_writeio3E);
    INIT_IO_MAP(0x3F, NULL          , NULL                  , sms           , sms_writeio3F);

    INIT_IO_MAP(0x7E, &sms->vdp     , tms9918a_getscanline  , &sms->snd     , sn76489_write);
    INIT_IO_MAP(0x7F, &sms->vdp     , tms9918a_getscanline  , &sms->snd     , sn76489_write); // Read H Counter not implemented

    INIT_IO_MAP(0xBD, NULL          , NULL                  , &sms->vdp     , tms9918a_writeop);
    INIT_IO_MAP(0xBE, &sms->vdp     , tms9918a_readdata     , &sms->vdp     , tms9918a_writedata);
    INIT_IO_MAP(0xBF, &sms->vdp     , tms9918a_readstatus   , &sms->vdp     , tms9918a_writeop);

    INIT_IO_MAP(0xC0, sms           , sms_readiojoypad1     , NULL          , NULL);
    INIT_IO_MAP(0xC1, sms           , sms_readiojoypad2     , NULL          , NULL);

    INIT_IO_MAP(0xDC, sms           , sms_readiojoypad1     , NULL          , NULL);
    INIT_IO_MAP(0xDD, sms           , sms_readiojoypad2     , NULL          , NULL);
    INIT_IO_MAP(0xDE, sms           , sms_readiojoypad1     , sms           , sms_writeionull);
    INIT_IO_MAP(0xDF, sms           , sms_readiojoypad2     , sms           , sms_writeionull);

    INIT_IO_MAP(0xF0, NULL          , NULL                  , &sms->fmsnd   , ym2413_write);
    INIT_IO_MAP(0xF1, NULL          , NULL                  , &sms->fmsnd   , ym2413_write);
    INIT_IO_MAP(0xF2, sms           , sms_readioF2          , sms           , sms_writeioF2);
}

void ms_free(mastersystem *sms)
{
    string bkpfilename;
    FILE *f;

    if(sms->cartridgeram_detected) {
        bkpfilename = sms_getbackupfilename(sms);

        if((f = fopen(CSTR(bkpfilename), "wb"))==NULL) {
            log4me_error(LOG_EMU_SMS, "Save cartridge RAM failed !\n");
            return;
        }
        fwrite(sms->cartridgeram, sizeof(byte), MS_CARTRIDGE_RAM, f);
        fclose(f) ;
        strfree(bkpfilename);
    }

    if(sms->mc93c46) {
        bkpfilename = sms_geteepromfilename(sms);
        seeprom_savedata(sms->mc93c46, bkpfilename);
        strfree(bkpfilename);
    }

    seeprom_free(sms->mc93c46);
    ym2413_free(&sms->fmsnd);
    sn76489_free(&sms->snd);
    tms9918a_free(&sms->vdp);
    clock_release(sms->idclock1s);
    strfree(sms->romname);
    if(sms->iomapper) free(sms->iomapper);
    if(sms->rom) free(sms->rom);
    if(sms->lkptsps) free(sms->lkptsps);
    if(sms->z80) cpuZ80_free(sms->z80);
    free(sms);
}

mastersystem* ms_init(const display *screen, const romspecs *rspecs, sound_onoff playsound, int joypad1, int joypad2, string backupdir)
{
    int i;
    assert((getromgameconsole(rspecs)==GC_SMS) || (getromgameconsole(rspecs)==GC_GG));

    memorymapper mmrom = getrommemorymapper(rspecs);
    mastersystem *sms = calloc(1, sizeof(mastersystem));
    if(sms==NULL) {
        log4me_error(LOG_EMU_SMS, "Unable to allocate the memory block.\n");
        exit(EXIT_FAILURE);
    }
    pushobject(sms, ms_free);

    switch(mmrom) {
        case MM_SEGA:
            sms->z80 = cpuZ80_create(sms, ms_readmemory, ms_writememory, ms_readio, ms_writeio);
            break;
        case MM_SEGA_EEPROM:
            sms->z80 = cpuZ80_create(sms, ms_readmemory_segaeeprom, ms_writememory_segaeeprom, ms_readio, ms_writeio);
            sms->mc93c46 = seeprom_create();
            break;
        case MM_CODEMASTERS:
             sms->z80 = cpuZ80_create(sms, sms_readmemorycodemasters, sms_writememorycodemasters, ms_readio, ms_writeio);
            break;
    }

    if(sms->z80==NULL) {
        log4me_error(LOG_EMU_SMS, "Unable to allocate and initialize the Z80 cpu emulator.\n");
        exit(EXIT_FAILURE);
    }

    sms->rspecs = rspecs;
    sms->gconsole = getromgameconsole(rspecs);

    sms->clock = getromvideomode(sms->rspecs)==VM_NTSC ? CPU_CLOCK_NTSC : CPU_CLOCK_PAL;
    sms->cpu = 0;

    cpuZ80_reset(sms->z80);
    cpuZ80_setIM(sms->z80, 1); // Force IM 1

    //memset(sms->mem, 0, MS_MEM_SIZE);
    sms->rom = NULL;
    sms->romname = NULL;
    sms->romlen = 0;
    sms->rombanks = MS_PAGE_SIZE*2; // 32k rom size

    sms->wrmap[0] = sms->mnull;
    sms->wrmap[1] = sms->mnull;
    sms->wrmap[2] = sms->mnull;
    sms->wrmap[3] = sms->mnull;
    sms->wrmap[4] = sms->mnull;
    sms->wrmap[5] = sms->mnull;
    sms->rdmap[6] = sms->wrmap[6] = sms->mem;
    sms->rdmap[7] = sms->wrmap[7] = sms->mem;

    // => Software Reference Manual for the SEGA Mark III Console
    // When power is applied, the following data is set by the system ROM program:
    // $FFFF = 2    $FFFE = 1   $FFFD = 0   $FFFC = 0
    //ms_writememory(sms, 0xFFFF, 2);
    //ms_writememory(sms, 0xFFFE, 1);
    //ms_writememory(sms, 0xFFFD, 0);
    //ms_writememory(sms, 0xFFFC, 0);

    //memset(sms->cartridgeram, 0, MS_CARTRIDGE_RAM);
    sms->cartridgeram_detected = 0;

    // Game Gear only
    sms->ports[0] = (getrommachine(sms->rspecs)!=JAPAN ? 0x40 : 0) | 0x80;
    sms->ports[1] = 0x7F;
    sms->ports[2] = 0xFF;
    sms->ports[3] = 0x00;
    sms->ports[4] = 0xFF;
    sms->ports[5] = 0x00;
    assert(MS_GG_NUM_PORTS==6);

    sms->port_3f = 0;
    sms->port_dc = JOYPAD1_DC_INIT;
    sms->port_dd = JOYPAD2_DD_INIT;
    sms->port_3e = 0xE0; // => Software Reference Manual for the SEGA Mark III Console (page 44)
    sms->port_f2 = 0;

    sms_writeio3E(sms, 0x3E, sms->port_3e); // force init iochip_disable, etc...

    sms->joystick1 = joypad1;
    sms->joystick2 = joypad2;

    sms->pause = 0;

    tms9918a_init(&sms->vdp, sms->gconsole, screen, getromvideomode(rspecs));
    sms->scanlinespersecond = sms->vdp.framerate * sms->vdp.internalscanlines;
    sms->curscanlineps = 0;

    sms->tstates = 0;

    sn76489_init(&sms->snd, sms->clock, sms->scanlinespersecond, sms->gconsole==GC_GG ? SC_STEREO : SC_MONO, playsound);
    ym2413_init(&sms->fmsnd);

    sms->backupdir = backupdir;
    sms->idclock1s = clock_new(1);

    sms->iomapper = calloc(256, sizeof(iomap));
    sms_initiomapper(sms);

    sms->lkptsps = malloc(sizeof(int)*sms->scanlinespersecond);
    if(sms->lkptsps==NULL) {
        log4me_error(LOG_EMU_SMS, "Unable to allocate the memory block.\n");
        exit(EXIT_FAILURE);
    }

    for(i=0;i<sms->scanlinespersecond;i++)
        sms->lkptsps[i] = (((uint64_t)sms->clock*(i+1)) / sms->scanlinespersecond) - (((uint64_t)sms->clock*i) / sms->scanlinespersecond);

    sms_loadrom(sms);

    if(romhavesnapshot(rspecs))
        sms_loadsnapshot(sms, getromsnapshot(rspecs));

    return sms;
}

static void sms_loadrom(mastersystem *sms)
{
    string bkpfilename;
    FILE *fbkp;

    log4me_print("************ Loading ROM ************\n");

    sms->romname = getromname(sms->rspecs);
    log4me_print("%s\n", CSTR(sms->romname));
    log4me_print("Digest    : %s\n", CSTR(getromdigest(sms->rspecs)));
    log4me_print("Plateform : %s\n", sms->gconsole==GC_SMS ? "Sega Master Sytem" : "Sega Game Gear");

    sms->romlen = getromlength(sms->rspecs);
    log4me_print("Length    : %d ko\n", sms->romlen/1024);

    sms->rombanks = (sms->romlen / MS_PAGE_SIZE);
    log4me_print("Banks     : %d\n", sms->rombanks);

    sms->rom = malloc(sms->romlen);
    if(readromdata(sms->rspecs, 0, sms->rom, sms->romlen)!=sms->romlen)
        goto error;

    //log4me_print("Tag       : %s\n", memcmp(sms->rom + 0x7FF0, SEGA_MS_TAG, strlen(SEGA_MS_TAG))!=0 ? "Not found" : SEGA_MS_TAG);

    int i;
    switch(getrommemorymapper(sms->rspecs)) {
        case MM_SEGA:
        case MM_SEGA_EEPROM:
            ms_writememory(sms, 0xFFFC, 0);
            ms_writememory(sms, 0xFFFD, 0);
            ms_writememory(sms, 0xFFFE, 1);
            if(sms->romlen>0x8000)
            ms_writememory(sms, 0xFFFF, 2);
            break;

        case MM_CODEMASTERS:
            for(i=0;i<6;i++) sms->rdmap[i] = sms->rom + 8192 * i;
            sms->cmregister = 2;
            break;
    }

    sms->port_3e = sms->romlen<=0x8000 ? 0xC8 : 0xA8; // => Software Reference Manual for the SEGA Mark III Console (page 44)

    // Load backup cartridge RAM
    bkpfilename = sms_getbackupfilename(sms);
    if((fbkp = fopen(CSTR(bkpfilename), "rb"))!=NULL) {
        sms->cartridgeram_detected = fread(sms->cartridgeram, sizeof(byte), MS_CARTRIDGE_RAM, fbkp) == MS_CARTRIDGE_RAM;
        if(sms->cartridgeram_detected)
            log4me_print("RAM       : loaded\n");
        else
            memset(sms->cartridgeram, 0, MS_CARTRIDGE_RAM);
        fclose(fbkp) ;
    }
    strfree(bkpfilename);

    // Load EEPROM data
    bkpfilename = sms_geteepromfilename(sms);
    if(sms->mc93c46 && fileexists(bkpfilename)) {
        seeprom_loaddata(sms->mc93c46, bkpfilename);
        log4me_print("EEPROM    : loaded\n");
    }
    strfree(bkpfilename);

    log4me_print("************ Let's GO ! *************\n\n");
    return;

error:
    log4me_error(LOG_EMU_SMS, "Read ROM failed.\n");
    exit(EXIT_FAILURE);
}

void ms_start(mastersystem *sms)
{
    sn76489_pause(&sms->snd, 0);

    clock_reset();
    sms->vdp.frame = 0;
    sms->snd.samplerate = 0;
    sms->cpu = 0;
    clock_step(sms->idclock1s);
}

static inline int sms_cpustep(mastersystem *sms)
{
#ifdef DEBUG
    char dasm[80];

    if(log4me_enabled(LOG_EMU_Z80_MASK) && !sms->z80->halted) {
        cpuZ80_dasm_pc(sms->z80, dasm, 80);
        log4me_debug(LOG_EMU_Z80, "0x%04X : %s\n", cpuZ80_getPC(sms->z80), dasm);
    }
#endif
    return cpuZ80_step(sms->z80);
}

void ms_execute(mastersystem *sms)
{
#define interrupt() { \
    if(tms9918a_int_pending(&sms->vdp)) { \
        tstates_op = sms_cpustep(sms); \
        if(cpuZ80_int_accepted(sms->z80)) tstates_op += cpuZ80_int(sms->z80); \
        sms->tstates += tstates_op; \
        sms->cpu += tstates_op; \
    } \
}
    int monitor_scanlines;
    int tstates_per_scanline, tstates_op;

    assert(sms->vdp.scanline==0);
    assert(sms->pause==0);

    monitor_scanlines = tms9918a_getmonitorscanlines(&sms->vdp);

    // wait next frame
    tms9918a_waitnextframe(&sms->vdp);

    while(monitor_scanlines>0) {
        tstates_per_scanline = sms->lkptsps[sms->curscanlineps];

        do {
            interrupt();

            tstates_op = sms_cpustep(sms);
            sms->tstates += tstates_op;
            sms->cpu += tstates_op;
        } while(sms->tstates<tstates_per_scanline);
        sms->tstates -= tstates_per_scanline;

        if((sms->gconsole==GC_SMS) && input_key_down(SDL_SCANCODE_S))
            cpuZ80_nmi(sms->z80);

        sn76489_execute(&sms->snd);

        tms9918a_execute(&sms->vdp);

        interrupt();

        if(++sms->curscanlineps>=sms->scanlinespersecond) sms->curscanlineps = 0;
        monitor_scanlines--;

        if(clock_elapsed(sms->idclock1s)) {
            //log4me_info("[DBG] FPS : %d - Sound : %d - CPU : %d - Sound buffer : %d\n", sms->vdp.frame, sms->snd.samplerate, sms->cpu, sms->snd.curpos);
            sms->vdp.frame = 0;
            sms->snd.samplerate = 0;
            sms->cpu = 0;
            clock_step(sms->idclock1s);
        }
    }
}

void ms_pause(mastersystem *sms, int pause)
{
    if(pause) {
        sms->pause += 1;
        if(sms->pause==1) {
            clock_pause(1);
            sn76489_pause(&sms->snd, 1);
        }
    } else {
        sms->pause -= 1;
        if(sms->pause==0) {
            clock_pause(0);
            sn76489_pause(&sms->snd, 0);
        }
    }
}

int ms_ispaused(mastersystem *sms)
{
    return (sms->pause>0);
}

#define SET_RES_PAD(value, port, flag) {\
    if((value)) port &= ~flag; else port |= flag; \
}

#define KEY_PAD(key, port, flag) {\
    SET_RES_PAD(input_key_pressed(key), port, flag); \
}

#define BUT_PAD(pad, button, port, flag) {\
    SET_RES_PAD(input_pad_button_pressed((pad),(button)), port, flag); \
}

#define AXS_PAD(pad, axis, port, flag) {\
    SET_RES_PAD(input_pad_axis_pressed((pad),(axis)), port, flag); \
}

static void sms_updatejoypads(mastersystem *sms)
{
    int nevents;
    SDL_Event events[10];

    SDL_PumpEvents();

    // Keyboard
    nevents = SDL_PeepEvents(events, 10, SDL_GETEVENT, SDL_KEYDOWN, SDL_KEYUP);
    while(--nevents>=0)
        input_process_event(&events[nevents]);

    // Joysticks
    nevents = SDL_PeepEvents(events, 10, SDL_GETEVENT, SDL_JOYAXISMOTION, SDL_JOYBUTTONUP);
    while(--nevents>=0)
        input_process_event(&events[nevents]);

    // Joypad 1
    if(sms->joystick1==NO_JOYPAD) {
        KEY_PAD(SDL_SCANCODE_UP   , sms->port_dc, JOYPAD1_DC_UP);
        KEY_PAD(SDL_SCANCODE_DOWN , sms->port_dc, JOYPAD1_DC_DOWN);
        KEY_PAD(SDL_SCANCODE_RIGHT, sms->port_dc, JOYPAD1_DC_RIGHT);
        KEY_PAD(SDL_SCANCODE_LEFT , sms->port_dc, JOYPAD1_DC_LEFT);
        KEY_PAD(SDL_SCANCODE_Z    , sms->port_dc, JOYPAD1_DC_TL);
        KEY_PAD(SDL_SCANCODE_X    , sms->port_dc, JOYPAD1_DC_TR);
    } else {
        AXS_PAD(sms->joystick1, AX_UP   , sms->port_dc, JOYPAD1_DC_UP);
        AXS_PAD(sms->joystick1, AX_DOWN , sms->port_dc, JOYPAD1_DC_DOWN);
        AXS_PAD(sms->joystick1, AX_RIGHT, sms->port_dc, JOYPAD1_DC_RIGHT);
        AXS_PAD(sms->joystick1, AX_LEFT , sms->port_dc, JOYPAD1_DC_LEFT);
        BUT_PAD(sms->joystick1, 0, sms->port_dc, JOYPAD1_DC_TL);
        BUT_PAD(sms->joystick1, 1, sms->port_dc, JOYPAD1_DC_TR);
    }

    // Joypad 2
    if(sms->joystick2==NO_JOYPAD) {
        KEY_PAD(SDL_SCANCODE_KP_8     , sms->port_dc, JOYPAD2_DC_UP);
        KEY_PAD(SDL_SCANCODE_KP_5     , sms->port_dc, JOYPAD2_DC_DOWN);
        KEY_PAD(SDL_SCANCODE_KP_6     , sms->port_dd, JOYPAD2_DD_RIGHT);
        KEY_PAD(SDL_SCANCODE_KP_4     , sms->port_dd, JOYPAD2_DD_LEFT);
        KEY_PAD(SDL_SCANCODE_KP_0     , sms->port_dd, JOYPAD2_DD_TL);
        KEY_PAD(SDL_SCANCODE_KP_PERIOD, sms->port_dd, JOYPAD2_DD_TR);
    } else {
        AXS_PAD(sms->joystick2, AX_UP   , sms->port_dc, JOYPAD2_DC_UP);
        AXS_PAD(sms->joystick2, AX_DOWN , sms->port_dc, JOYPAD2_DC_DOWN);
        AXS_PAD(sms->joystick2, AX_RIGHT, sms->port_dd, JOYPAD2_DD_RIGHT);
        AXS_PAD(sms->joystick2, AX_LEFT , sms->port_dd, JOYPAD2_DD_LEFT);
        BUT_PAD(sms->joystick2, 0, sms->port_dd, JOYPAD2_DD_TL);
        BUT_PAD(sms->joystick2, 1, sms->port_dd, JOYPAD2_DD_TR);
    }

    if(sms->gconsole==GC_GG) {
        KEY_PAD(SDL_SCANCODE_S, sms->ports[0], GG_START_PAUSE);
    }

    // Others
    if(sms->gconsole==GC_SMS) {
        if( input_key_down(SDL_SCANCODE_R))
            log4me_print("Software reset\n");
        KEY_PAD(SDL_SCANCODE_R, sms->port_dd, JOYPAD2_DD_RESET);
    }
}

static string sms_getbackupfilename(mastersystem *sms)
{
    string bkpfilename = strdups(sms->backupdir);

    straddc(bkpfilename, "/");
    stradds(bkpfilename, sms->romname);
    straddc(bkpfilename, ".bkp");
    return bkpfilename;
}

static string sms_geteepromfilename(mastersystem *sms)
{
    string bkpfilename = strdups(sms->backupdir);

    straddc(bkpfilename, "/");
    stradds(bkpfilename, sms->romname);
    straddc(bkpfilename, ".eeprom");
    return bkpfilename;
}

void sms_takesnapshot(mastersystem *sms, xmlTextWriterPtr writer)
{
    int i;

    xmlTextWriterStartElement(writer, BAD_CAST "machine");
    xmlTextWriterWriteAttribute(writer, BAD_CAST "id", BAD_CAST "sms");
    xmlTextWriterWriteAttribute(writer, BAD_CAST "version", BAD_CAST VERSION);

        xmlTextWriterStartElement(writer, BAD_CAST "rom");
        xmlTextWriterWriteFormatString(writer, "%s", CSTR(sms->romname));
        xmlTextWriterEndElement(writer); /* rom */

        xmlTextWriterStartElement(writer, BAD_CAST "romfilename");
        xmlTextWriterWriteFormatString(writer, "%s", CSTR(getrommainfilename(sms->rspecs)));
        xmlTextWriterEndElement(writer); /* romfilename */

        xmlTextWriterStartElement(writer, BAD_CAST "digest");
        xmlTextWriterWriteFormatString(writer, "%s", CSTR(getromdigest(sms->rspecs)));
        xmlTextWriterEndElement(writer); /* digest */

        xmlTextWriterStartElement(writer, BAD_CAST "country");
        xmlTextWriterWriteFormatString(writer, "%s", getrommachine(sms->rspecs)==JAPAN ? "japan" : "export");
        xmlTextWriterEndElement(writer); /* country */

        xmlTextWriterStartElement(writer, BAD_CAST "videomode");
        xmlTextWriterWriteFormatString(writer, "%s", getromvideomode(sms->rspecs)==VM_NTSC ? "ntsc" : "pal");
        xmlTextWriterEndElement(writer); /* videmode */

        switch(getrommemorymapper(sms->rspecs)) {
            case MM_SEGA:
            case MM_SEGA_EEPROM:
                xmlTextWriterStartElement(writer, BAD_CAST "memregisters");
                    for(i=0;i<MS_REGISTERS;i++) {
                        xmlTextWriterStartElement(writer, BAD_CAST "register");
                        xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "id", "%d", i);
                        xmlTextWriterWriteFormatString(writer, "%02X", sms->mregisters[i]);
                        xmlTextWriterEndElement(writer); /* register */
                }
                xmlTextWriterEndElement(writer); /* memregisters */
                break;
            case MM_CODEMASTERS:
                xmlTextWriterStartElement(writer, BAD_CAST "cmregister");
                xmlTextWriterWriteFormatString(writer, "%02X", sms->cmregister);
                xmlTextWriterEndElement(writer); /* cmregister */
                break;
        }

        xmlTextWriterStartElement(writer, BAD_CAST "ports");
            if(sms->gconsole==GC_GG) {
                for(i=0;i<MS_GG_NUM_PORTS;i++) {
                    xmlTextWriterStartElement(writer, BAD_CAST "port");
                    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "id", "%02X", i);
                    xmlTextWriterWriteFormatString(writer, "%02X", sms->ports[i]);
                    xmlTextWriterEndElement(writer); /* port */
                }
            }

            xmlTextWriterStartElement(writer, BAD_CAST "port");
            xmlTextWriterWriteAttribute(writer, BAD_CAST "id", BAD_CAST "3E");
            xmlTextWriterWriteFormatString(writer, "%02X", sms->port_3e);
            xmlTextWriterEndElement(writer); /* port */

            xmlTextWriterStartElement(writer, BAD_CAST "port");
            xmlTextWriterWriteAttribute(writer, BAD_CAST "id", BAD_CAST "3F");
            xmlTextWriterWriteFormatString(writer, "%02X", sms->port_3f);
            xmlTextWriterEndElement(writer); /* port */

            xmlTextWriterStartElement(writer, BAD_CAST "port");
            xmlTextWriterWriteAttribute(writer, BAD_CAST "id", BAD_CAST "DC");
            xmlTextWriterWriteFormatString(writer, "%02X", sms->port_dc);
            xmlTextWriterEndElement(writer); /* port */

            xmlTextWriterStartElement(writer, BAD_CAST "port");
            xmlTextWriterWriteAttribute(writer, BAD_CAST "id", BAD_CAST "DD");
            xmlTextWriterWriteFormatString(writer, "%02X", sms->port_dd);
            xmlTextWriterEndElement(writer); /* port */

            xmlTextWriterStartElement(writer, BAD_CAST "port");
            xmlTextWriterWriteAttribute(writer, BAD_CAST "id", BAD_CAST "F2");
            xmlTextWriterWriteFormatString(writer, "%02X", sms->port_f2);
            xmlTextWriterEndElement(writer); /* port */
        xmlTextWriterEndElement(writer); /* ports */

        xmlTextWriterWriteArray(writer, "memory", sms->mem, MS_MEM_SIZE);

        if(sms->cartridgeram_detected)
            xmlTextWriterWriteArray(writer, "cartridgeram", sms->cartridgeram, MS_CARTRIDGE_RAM);

        if(sms->mc93c46)
            seeprom_takesnapshot(sms->mc93c46, writer);

        xmlTextWriterStartElement(writer, BAD_CAST "cpu");
            cpuZ80_takesnapshot(sms->z80, writer);
        xmlTextWriterEndElement(writer); /* cpu */

        xmlTextWriterStartElement(writer, BAD_CAST "video");
            tms9918a_takesnapshot(&sms->vdp, writer);
        xmlTextWriterEndElement(writer); /* video */

        xmlTextWriterStartElement(writer, BAD_CAST "sound");
            sn76489_takesnapshot(&sms->snd, writer);
        xmlTextWriterEndElement(writer); /* sound */

    xmlTextWriterEndElement(writer); /* machine */
}

static void sms_loadsnapshot(mastersystem *sms, xmlDocPtr doc)
{
    xmlNode *rootnode = xmlDocGetRootElement(doc);
    assert(strcasecmp("machine", (const char*)rootnode->name)==0);
    assert(rootnode->next==NULL);

    XML_ENUM_CHILD(rootnode, node,
        XML_ELEMENT_ENUM_CHILD("memregisters", node, child,
            XML_ELEMENT_CONTENT("register", child,
                ms_writemregister(sms, atoi((const char*)xmlGetProp(child, BAD_CAST "id")), (byte)strtoul((const char*)content, NULL, 16));
            )
        )

        XML_ELEMENT_CONTENT("cmregister", node,
            sms_writememorycodemasters(sms, 0x8000, (byte)strtoul((const char*)content, NULL, 16));
        )

        XML_ELEMENT_ENUM_CHILD("ports", node, child,
            XML_ELEMENT_CONTENT("port", child,
                xmlChar *port = xmlGetProp(child, BAD_CAST "id");
                if(xmlStrCaseCmp(port, "00"))
                    sms->ports[0] = (byte)strtoul((const char *)content, NULL, 16);
                else
                if(xmlStrCaseCmp(port, "01"))
                    sms->ports[1] = (byte)strtoul((const char *)content, NULL, 16);
                else
                if(xmlStrCaseCmp(port, "02"))
                    sms->ports[2] = (byte)strtoul((const char *)content, NULL, 16);
                else
                if(xmlStrCaseCmp(port, "03"))
                    sms->ports[3] = (byte)strtoul((const char *)content, NULL, 16);
                else
                if(xmlStrCaseCmp(port, "04"))
                    sms->ports[4] = (byte)strtoul((const char *)content, NULL, 16);
                else
                if(xmlStrCaseCmp(port, "05"))
                    sms->ports[5] = (byte)strtoul((const char *)content, NULL, 16);
                else
                if(xmlStrCaseCmp(port, "3E"))
                    sms->port_3e = (byte)strtoul((const char *)content, NULL, 16);
                else
                if(xmlStrCaseCmp(port, "3F"))
                    sms->port_3f = (byte)strtoul((const char *)content, NULL, 16);
                else
                if(xmlStrCaseCmp(port, "DC"))
                    sms->port_dc = (byte)strtoul((const char *)content, NULL, 16);
                else
                if(xmlStrCaseCmp(port, "DD"))
                    sms->port_dd = (byte)strtoul((const char *)content, NULL, 16);
                else
                if(xmlStrCaseCmp(port, "F2"))
                    sms->port_f2 = (byte)strtoul((const char *)content, NULL, 16);
            )
        )

        XML_ELEMENT("memory", node,
            xmlNodeReadArray(node, sms->mem, MS_MEM_SIZE);
        )

        XML_ELEMENT("cartridgeram", node,
            xmlNodeReadArray(node, sms->cartridgeram, MS_CARTRIDGE_RAM);
            sms->cartridgeram_detected = 1;
        )

        XML_ELEMENT("eeprom", node,
            if(sms->mc93c46)
                seeprom_loadsnapshot(sms->mc93c46, node);
        )

        XML_ELEMENT_ENUM_CHILD("cpu", node, cpu,
            XML_ELEMENT("z80", cpu,
                cpuZ80_loadsnapshot(sms->z80, cpu);
            )
        )

        XML_ELEMENT_ENUM_CHILD("video", node, video,
            XML_ELEMENT("tms99xx", video,
                tms9918a_loadsnapshot(&sms->vdp, video);
            )
        )

        XML_ELEMENT_ENUM_CHILD("sound", node, sound,
            XML_ELEMENT("sn76489", sound,
                sn76489_loadsnapshot(&sms->snd, sound);
            )
        )
    )
}
