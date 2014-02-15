#ifndef Z80_COMMON_H_INCLUDED
#define Z80_COMMON_H_INCLUDED

#include "z80.h"

static const byte lkp_f_sz[256] = {
#define M1(x) (((x)==0 ? FLAG_ZERO : 0) | ((x)&0x80 ? FLAG_SIGN : 0))
    M256(0)
#undef M1
};

// Parity hack from
// http://www-graphics.stanford.edu/~seander/bithacks.html#ParityWith64Bits
static const byte lkp_f_szp[256] = {
#define M1(x) (((x)==0 ? FLAG_ZERO : 0) | ((x)&0x80 ? FLAG_SIGN : 0) | \
               ((((((x) * 0x0101010101010101ULL) & 0x8040201008040201ULL) % 0x1FF) & 1) ? 0 : FLAG_PARITY))
    M256(0)
#undef M1
};

#define flags_sz(x)     lkp_f_sz[(x)]
#define flags_szp(x)    lkp_f_szp[(x)]
#define flags_p(x)      (lkp_f_szp[(x)] & FLAG_PARITY)

#define PUSH(x) \
{\
    cpu->SP -= 2;\
    write16(cpu, cpu->SP, (x));\
}

#define POP(x) \
{\
    (x) = read16(cpu, cpu->SP);\
    cpu->SP += 2;\
}

#define RST(x) \
{\
    cpu->cycles += 11;\
    PUSH(cpu->PC);\
    cpu->PC = (x);\
}

#define sread8(a,b) ((int8_t)read8(a,b))

byte read8(cpuZ80 *cpu, word addr);
word read16(cpuZ80 *cpu, word addr);
void write8(cpuZ80 *cpu, word addr, byte value);
void write16(cpuZ80 *cpu, word addr, word value);

byte readio(cpuZ80 *cpu, byte port);
void writeio(cpuZ80 *cpu, byte port, byte value);

byte inc8(cpuZ80 *cpu, byte value);
byte dec8(cpuZ80 *cpu, byte value);
byte add8(cpuZ80 *cpu, byte v1, byte v2);
byte adc8(cpuZ80 *cpu, byte v1, byte v2);
byte sub8(cpuZ80 *cpu, byte v1, byte v2);
byte sbc8(cpuZ80 *cpu, byte v1, byte v2);

byte and8(cpuZ80 *cpu, byte v1, byte v2);
byte xor8(cpuZ80 *cpu, byte v1, byte v2);
byte or8(cpuZ80 *cpu, byte v1, byte v2);

void bit8(cpuZ80 *cpu, int b, byte value);
byte set8(cpuZ80 *cpu, int b, byte value);
byte res8(cpuZ80 *cpu, int b, byte value);

byte rlc8(cpuZ80 *cpu, byte value);
byte rrc8(cpuZ80 *cpu, byte value);
byte srl8(cpuZ80 *cpu, byte value);
byte sla8(cpuZ80 *cpu, byte value);
byte sll8(cpuZ80 *cpu, byte value);
byte sra8(cpuZ80 *cpu, byte value);
byte rl8(cpuZ80 *cpu, byte value);
byte rr8(cpuZ80 *cpu, byte value);

word add16(cpuZ80 *cpu, word v1, word v2);
word sub16(cpuZ80 *cpu, word v1, word v2);
word adc16(cpuZ80 *cpu, word v1, word v2);
word sbc16(cpuZ80 *cpu, word v1, word v2);

byte in8(cpuZ80 *cpu, byte port);

#ifdef DEBUG
void dasmopcode(cpuZ80 *cpu, const char *dasm, word addr, char *buffer, size_t len);
#endif /* DEBUG */

#endif // Z80_COMMON_H_INCLUDED
