#ifndef CPU_LIBZ80EX_H_INCLUDED
#define CPU_LIBZ80EX_H_INCLUDED

#include "../emul.h"
#include "../misc/types.h"
#include "../misc/xml.h"

#define FLAG_CARRY      0x01
#define FLAG_ADD_SUB    0x02
#define FLAG_OVERFLOW   0x04
#define FLAG_PARITY     0x04
#define FLAG_BIT3       0x08
#define FLAG_HALF_CARRY 0x10
#define FLAG_BIT5       0x20
#define FLAG_ZERO       0x40
#define FLAG_SIGN       0x80

typedef byte (*readmemory_handler)(void* param, dword address);
typedef void (*writememory_handler)(void* param, dword address, byte data);
typedef byte (*readio_handler)(void* param, byte port);
typedef void (*writeio_handler)(void* param, byte port, byte data);

typedef struct {
    union {
        struct {
            word AF, BC, DE, HL;
        } w;
        struct {
            byte F, A, C, B, E, D, L, H;
        } b;
    };

    union {
        struct {
            word AFp, BCp, DEp, HLp;
        } wp;
        struct {
            byte Fp, Ap, Cp, Bp, Ep, Dp, Lp, Hp;
        } bp;
    };

    word IX, IY, SP, PC;
    byte I, R, R7, IM, IFF1, IFF2;
    byte halted;

    int cycles;

    void *param;
    readmemory_handler readmem;
    writememory_handler writemem;
    readio_handler readio;
    writeio_handler writeio;
} cpuZ80;


cpuZ80 *cpuZ80_create(void *sms, readmemory_handler readmem, writememory_handler writemem, readio_handler readio, writeio_handler writeio);
void cpuZ80_free(cpuZ80 *cpu);
void cpuZ80_reset(cpuZ80 *cpu);
int cpuZ80_step(cpuZ80 *cpu);
int cpuZ80_int(cpuZ80 *cpu);
int cpuZ80_nmi(cpuZ80 *cpu);

word cpuZ80_getAF(cpuZ80 *cpu);
word cpuZ80_getBC(cpuZ80 *cpu);
word cpuZ80_getDE(cpuZ80 *cpu);
word cpuZ80_getHL(cpuZ80 *cpu);
word cpuZ80_getAF_(cpuZ80 *cpu);
word cpuZ80_getBC_(cpuZ80 *cpu);
word cpuZ80_getDE_(cpuZ80 *cpu);
word cpuZ80_getHL_(cpuZ80 *cpu);
word cpuZ80_getIX(cpuZ80 *cpu);
word cpuZ80_getIY(cpuZ80 *cpu);
word cpuZ80_getPC(cpuZ80 *cpu);
word cpuZ80_getSP(cpuZ80 *cpu);
word cpuZ80_getI(cpuZ80 *cpu);
word cpuZ80_getR(cpuZ80 *cpu);
word cpuZ80_getR7(cpuZ80 *cpu);
word cpuZ80_getIM(cpuZ80 *cpu);
word cpuZ80_getIFF1(cpuZ80 *cpu);
word cpuZ80_getIFF2(cpuZ80 *cpu);

void cpuZ80_setAF(cpuZ80 *cpu, dword value);
void cpuZ80_setBC(cpuZ80 *cpu, dword value);
void cpuZ80_setDE(cpuZ80 *cpu, dword value);
void cpuZ80_setHL(cpuZ80 *cpu, dword value);
void cpuZ80_setAF_(cpuZ80 *cpu, dword value);
void cpuZ80_setBC_(cpuZ80 *cpu, dword value);
void cpuZ80_setDE_(cpuZ80 *cpu, dword value);
void cpuZ80_setHL_(cpuZ80 *cpu, dword value);
void cpuZ80_setIX(cpuZ80 *cpu, dword value);
void cpuZ80_setIY(cpuZ80 *cpu, dword value);
void cpuZ80_setPC(cpuZ80 *cpu, dword value);
void cpuZ80_setSP(cpuZ80 *cpu, dword value);
void cpuZ80_setI(cpuZ80 *cpu, dword value);
void cpuZ80_setR(cpuZ80 *cpu, dword value);
void cpuZ80_setR7(cpuZ80 *cpu, dword value);
void cpuZ80_setIM(cpuZ80 *cpu, dword value);
void cpuZ80_setIFF1(cpuZ80 *cpu, dword value);
void cpuZ80_setIFF2(cpuZ80 *cpu, dword value);

int cpuZ80_is_halted(cpuZ80 *cpu);
int cpuZ80_int_accepted(cpuZ80 *cpu);

#ifdef DEBUG
#define cpuZ80_dasm_pc(cpu,buffer,len) cpuZ80_dasm(cpu,cpuZ80_getPC(cpu),buffer,len)
void cpuZ80_dasm(cpuZ80 *cpu, word addr, char *buffer, int len);
#define DASM(x) (x)
#else
#define DASM(x) (NULL)
#endif

void cpuZ80_takesnapshot(cpuZ80 *cpu, xmlTextWriterPtr writer);
void cpuZ80_loadsnapshot(cpuZ80 *cpu, xmlNode *cpunode);

typedef void (*opcode_function)(cpuZ80 *cpu);
typedef struct {
    opcode_function func;
    char *dasm;
} opcode;

#endif // CPU_LIBZ80EX_H_INCLUDED
