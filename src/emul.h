#ifndef EMUL_H_INCLUDED
#define EMUL_H_INCLUDED

#include <assert.h>

#include <SDL.h>

#include "misc/types.h"
#include "misc/log4me.h"

#define LOG_EMU_NONE            0x0000
#define LOG_EMU_MAIN_MASK       0x0001
#define LOG_EMU_MAIN            LOG_EMU_MAIN_MASK   , "EMU"
#define LOG_EMU_SDL_MASK        0x0002
#define LOG_EMU_SDL             LOG_EMU_SDL_MASK    , "SDL"
#define LOG_EMU_SMS_MASK        0x0004
#define LOG_EMU_SMS             LOG_EMU_SMS_MASK    , "SMS"
#define LOG_EMU_Z80_MASK        0x0008
#define LOG_EMU_Z80             LOG_EMU_Z80_MASK    , "Z80"
#define LOG_EMU_SMSVDP_MASK     0x0010
#define LOG_EMU_SMSVDP          LOG_EMU_SMSVDP_MASK , "VDP"
#define LOG_EMU_SN76489_MASK    0x0020
#define LOG_EMU_SN76489         LOG_EMU_SN76489_MASK, "SN7"
#define LOG_EMU_YM2413_MASK     0x0040
#define LOG_EMU_YM2413          LOG_EMU_YM2413_MASK , "YM2"
#define LOG_EMU_JOYSTICK_MASK   0x0080
#define LOG_EMU_JOYSTICK        LOG_EMU_JOYSTICK_MASK,"JOY"
#define LOG_EMU_SNAPSHOT_MASK   0x0100
#define LOG_EMU_SNAPSHOT        LOG_EMU_SNAPSHOT_MASK,"SNP"

#define LOG_EMU_ALL             0xFFFF


#define NJOYSTICKS          2


#define M2(x)       M1((x)), M1((x)+1)
#define M4(x)       M2((x)), M2((x)+2)
#define M8(x)       M4((x)), M4((x)+4)
#define M16(x)      M8((x)), M8((x)+8)
#define M32(x)      M16((x)), M16((x)+16)
#define M64(x)      M32((x)), M32((x)+32)
#define M128(x)     M64((x)), M64((x)+64)
#define M256(x)     M128((x)), M128((x)+128)
#define M512(x)     M256((x)), M256((x)+256)
#define M1024(x)    M512((x)), M512((x)+512)


#define bit7_is_set(x)  (((x) & 0x80)!=0)
#define bit6_is_set(x)  (((x) & 0x40)!=0)
#define bit5_is_set(x)  (((x) & 0x20)!=0)
#define bit4_is_set(x)  (((x) & 0x10)!=0)
#define bit3_is_set(x)  (((x) & 0x08)!=0)
#define bit2_is_set(x)  (((x) & 0x04)!=0)
#define bit1_is_set(x)  (((x) & 0x02)!=0)
#define bit0_is_set(x)  (((x) & 0x01)!=0)

#define bit7_not_set(x) (((x) & 0x80)==0)
#define bit6_not_set(x) (((x) & 0x40)==0)
#define bit5_not_set(x) (((x) & 0x20)==0)
#define bit4_not_set(x) (((x) & 0x10)==0)
#define bit3_not_set(x) (((x) & 0x08)==0)
#define bit2_not_set(x) (((x) & 0x04)==0)
#define bit1_not_set(x) (((x) & 0x02)==0)
#define bit0_not_set(x) (((x) & 0x01)==0)


#define MIN(a, b)       ((a)>(b) ? (b) : (a))
#define ABS(a)          ((a)<0 ? (-(a)) : (a))

typedef enum {
    SND_OFF,
    SND_ON
} sound_onoff;

typedef enum {
    VM_NTSC = 0,
    VM_PAL
} video_mode;

typedef enum {
    JAPAN = 0,
    EXPORT
} tmachine;

#endif // EMUL_H_INCLUDED
