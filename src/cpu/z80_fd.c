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

#include "z80.h"
#include "z80_common.h"

void opcode_fdcb(cpuZ80 *cpu);

static void add_iy_bc(cpuZ80 *cpu) /* 0xFD 0x09 */
{
    cpu->cycles += 15;
    cpu->IY = add16(cpu, cpu->IY, cpu->w.BC);
}

static void add_iy_de(cpuZ80 *cpu) /* 0xFD 0x19 */
{
    cpu->cycles += 15;
    cpu->IY = add16(cpu, cpu->IY, cpu->w.DE);
}

static void ld_iy_nn(cpuZ80 *cpu) /* 0xFD 0x21 */
{
    cpu->cycles += 14;
    cpu->IY = read16(cpu, cpu->PC);
    cpu->PC += 2;
}

static void ld_mnn_iy(cpuZ80 *cpu) /* 0xFD 0x22 */
{
    cpu->cycles += 20;
    write16(cpu, read16(cpu, cpu->PC), cpu->IY);
    cpu->PC += 2;
}

static void inc_iy(cpuZ80 *cpu) /* 0xFD 0x21 */
{
    cpu->cycles += 10;
    cpu->IY++;
}

static void inc_iyh(cpuZ80 *cpu) /* 0xFD 0x24 */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0x00FF) | ((word)inc8(cpu, cpu->IY >> 8) << 8);
}

static void dec_iyh(cpuZ80 *cpu) /* 0xFD 0x25 */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0x00FF) | ((word)dec8(cpu, cpu->IY >> 8) << 8);
}

static void ld_iyh_n(cpuZ80 *cpu) /* 0xFD 0x26 */
{
    cpu->cycles += 11;
    cpu->IY = (cpu->IY & 0x00FF) | ((word)read8(cpu, cpu->PC++) << 8);
}

static void add_iy_iy(cpuZ80 *cpu) /* 0xFD 0x29 */
{
    cpu->cycles += 15;
    cpu->IY = add16(cpu, cpu->IY, cpu->IY);
}

static void ld_iy_mnn(cpuZ80 *cpu) /* 0xFD 0x2A */
{
    cpu->cycles += 20;
    cpu->IY = read16(cpu, read16(cpu, cpu->PC));
    cpu->PC += 2;
}

static void dec_iy(cpuZ80 *cpu) /* 0xFD 0x2B */
{
    cpu->cycles += 10;
    cpu->IY--;
}

static void inc_iyl(cpuZ80 *cpu) /* 0xFD 0x2C */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0xFF00) | inc8(cpu, cpu->IY);
}

static void dec_iyl(cpuZ80 *cpu) /* 0xFD 0x2D */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0xFF00) | dec8(cpu, cpu->IY);
}

static void ld_iyl_n(cpuZ80 *cpu) /* 0xFD 0x2E */
{
    cpu->cycles += 11;
    cpu->IY = (cpu->IY & 0xFF00) | read8(cpu, cpu->PC++);
}

static void inc_miyd(cpuZ80 *cpu) /* 0xFD 0x34 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, inc8(cpu, read8(cpu, addr)));
}

static void dec_miyd(cpuZ80 *cpu) /* 0xFD 0x35 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, dec8(cpu, read8(cpu, addr)));
}

static void ld_miyd_n(cpuZ80 *cpu) /* 0xFD 0x36 */
{
    cpu->cycles += 19;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, read8(cpu, cpu->PC++));
}

static void add_iy_sp(cpuZ80 *cpu) /* 0xFD 0x39 */
{
    cpu->cycles += 15;
    cpu->IY = add16(cpu, cpu->IY, cpu->SP);
}

static void ld_b_miyd(cpuZ80 *cpu) /* 0xFD 0x46 */
{
    cpu->cycles += 19;
    cpu->b.B = read8(cpu, cpu->IY+sread8(cpu, cpu->PC++));
}

static void ld_c_miyd(cpuZ80 *cpu) /* 0xFD 0x4E */
{
    cpu->cycles += 19;
    cpu->b.C = read8(cpu, cpu->IY+sread8(cpu, cpu->PC++));
}

static void ld_d_iyh(cpuZ80 *cpu) /* 0xFD 0x54 */
{
    cpu->cycles += 8;
    cpu->b.D = ((cpu->IY & 0xFF00) >> 8);
}

static void ld_d_iyl(cpuZ80 *cpu) /* 0xFD 0x55 */
{
    cpu->cycles += 8;
    cpu->b.D = cpu->IY & 0x00FF;
}

static void ld_d_miyd(cpuZ80 *cpu) /* 0xFD 0x56 */
{
    cpu->cycles += 19;
    cpu->b.D = read8(cpu, cpu->IY+sread8(cpu, cpu->PC++));
}

static void ld_e_iyh(cpuZ80 *cpu) /* 0xFD 0x5C */
{
    cpu->cycles += 8;
    cpu->b.E = ((cpu->IY & 0xFF00) >> 8);
}

static void ld_e_iyl(cpuZ80 *cpu) /* 0xFD 0x5D */
{
    cpu->cycles += 8;
    cpu->b.E = cpu->IY & 0x00FF;
}

static void ld_e_miyd(cpuZ80 *cpu) /* 0xFD 0x5E */
{
    cpu->cycles += 19;
    cpu->b.E = read8(cpu, cpu->IY+sread8(cpu, cpu->PC++));
}

static void ld_iyh_b(cpuZ80 *cpu) /* 0xFD 0x60 */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0x00FF) | ((word)cpu->b.B << 8);
}

static void ld_iyh_c(cpuZ80 *cpu) /* 0xFD 0x61 */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0x00FF) | ((word)cpu->b.C << 8);
}

static void ld_iyh_d(cpuZ80 *cpu) /* 0xFD 0x62 */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0x00FF) | ((word)cpu->b.D << 8);
}

static void ld_iyh_e(cpuZ80 *cpu) /* 0xFD 0x63 */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0x00FF) | ((word)cpu->b.E << 8);
}

static void ld_h_miyd(cpuZ80 *cpu) /* 0xFD 0x66 */
{
    cpu->cycles += 19;
    cpu->b.H = read8(cpu, cpu->IY+sread8(cpu, cpu->PC++));
}

static void ld_iyh_a(cpuZ80 *cpu) /* 0xFD 0x67 */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0x00FF) | ((word)cpu->b.A << 8);
}

static void ld_iyl_b(cpuZ80 *cpu) /* 0xFD 0x68 */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0xFF00) | cpu->b.B;
}

static void ld_iyl_c(cpuZ80 *cpu) /* 0xFD 0x69 */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0xFF00) | cpu->b.C;
}

static void ld_iyl_d(cpuZ80 *cpu) /* 0xFD 0x6A */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0xFF00) | cpu->b.D;
}

static void ld_iyl_e(cpuZ80 *cpu) /* 0xFD 0x6B */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0xFF00) | cpu->b.E;
}

static void ld_l_miyd(cpuZ80 *cpu) /* 0xFD 0x6E */
{
    cpu->cycles += 19;
    cpu->b.L = read8(cpu, cpu->IY+sread8(cpu, cpu->PC++));
}

static void ld_iyl_a(cpuZ80 *cpu) /* 0xFD 0x6F */
{
    cpu->cycles += 8;
    cpu->IY = (cpu->IY & 0xFF00) | cpu->b.A;
}

static void ld_miyd_b(cpuZ80 *cpu) /* 0xFD 0x70 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IY+sread8(cpu, cpu->PC++), cpu->b.B);
}

static void ld_miyd_c(cpuZ80 *cpu) /* 0xFD 0x71 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IY+sread8(cpu, cpu->PC++), cpu->b.C);
}

static void ld_miyd_d(cpuZ80 *cpu) /* 0xFD 0x72 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IY+sread8(cpu, cpu->PC++), cpu->b.D);
}

static void ld_miyd_e(cpuZ80 *cpu) /* 0xFD 0x73 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IY+sread8(cpu, cpu->PC++), cpu->b.E);
}

static void ld_miyd_h(cpuZ80 *cpu) /* 0xFD 0x74 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IY+sread8(cpu, cpu->PC++), cpu->b.H);
}

static void ld_miyd_l(cpuZ80 *cpu) /* 0xFD 0x75 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IY+sread8(cpu, cpu->PC++), cpu->b.L);
}

static void ld_miyd_a(cpuZ80 *cpu) /* 0xFD 0x77 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IY+sread8(cpu, cpu->PC++), cpu->b.A);
}

static void ld_a_iyh(cpuZ80 *cpu) /* 0xFD 0x7C */
{
    cpu->cycles += 8;
    cpu->b.A = ((cpu->IY & 0xFF00) >> 8);
}

static void ld_a_iyl(cpuZ80 *cpu) /* 0xFD 0x7D */
{
    cpu->cycles += 8;
    cpu->b.A = cpu->IY & 0x00FF;
}

static void ld_a_miyd(cpuZ80 *cpu) /* 0xFD 0x7E */
{
    cpu->cycles += 19;
    cpu->b.A = read8(cpu, cpu->IY+sread8(cpu, cpu->PC++));
}

static void add_a_miyd(cpuZ80 *cpu) /* 0xFD 0x86 */
{
    cpu->cycles += 19;
    cpu->b.A = add8(cpu, cpu->b.A, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void adc_a_miyd(cpuZ80 *cpu) /* 0xFD 0x8E */
{
    cpu->cycles += 19;
    cpu->b.A = adc8(cpu, cpu->b.A, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void sub_miyd(cpuZ80 *cpu) /* 0xFD 0x96 */
{
    cpu->cycles += 19;
    cpu->b.A = sub8(cpu, cpu->b.A, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void sbc_a_miyd(cpuZ80 *cpu) /* 0xFD 0x9E */
{
    cpu->cycles += 19;
    cpu->b.A = sbc8(cpu, cpu->b.A, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void and_miyd(cpuZ80 *cpu) /* 0xFD 0xA6 */
{
    cpu->cycles += 19;
    cpu->b.A = and8(cpu, cpu->b.A, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void xor_miyd(cpuZ80 *cpu) /* 0xFD 0xAE */
{
    cpu->cycles += 19;
    cpu->b.A = xor8(cpu, cpu->b.A, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void or_miyd(cpuZ80 *cpu) /* 0xFD 0xB6 */
{
    cpu->cycles += 19;
    cpu->b.A = or8(cpu, cpu->b.A, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void cp_miyd(cpuZ80 *cpu) /* 0xFD 0xB6 */
{
    cpu->cycles += 19;
    sub8(cpu, cpu->b.A, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void pop_iy(cpuZ80 *cpu) /* 0xFD 0xE1 */
{
    cpu->cycles += 14;
    POP(cpu->IY);
}

static void ex_msp_iy(cpuZ80 *cpu) /* 0xFD 0xE3 */
{
    cpu->cycles += 23;
    word value = read16(cpu, cpu->SP);
    write16(cpu, cpu->SP, cpu->IY);
    cpu->IY = value;
}

static void push_iy(cpuZ80 *cpu) /* 0xFD 0xE5 */
{
    cpu->cycles += 15;
    PUSH(cpu->IY);
}

static void jp_iy(cpuZ80 *cpu) /* 0xFD 0xE9 */
{
    cpu->cycles += 8;
    cpu->PC = cpu->IY;
}

opcode opcodes_fd[256] = {
    { NULL,         DASM("")            /* 0xFD 0x00 */ },
    { NULL,         DASM("")            /* 0xFD 0x01 */ },
    { NULL,         DASM("")            /* 0xFD 0x02 */ },
    { NULL,         DASM("")            /* 0xFD 0x03 */ },
    { NULL,         DASM("")            /* 0xFD 0x04 */ },
    { NULL,         DASM("")            /* 0xFD 0x05 */ },
    { NULL,         DASM("")            /* 0xFD 0x06 */ },
    { NULL,         DASM("")            /* 0xFD 0x07 */ },
    { NULL,         DASM("")            /* 0xFD 0x08 */ },
    { add_iy_bc,    DASM("ADD IY,BC")   /* 0xFD 0x09 */ },
    { NULL,         DASM("")            /* 0xFD 0x0A */ },
    { NULL,         DASM("")            /* 0xFD 0x0B */ },
    { NULL,         DASM("")            /* 0xFD 0x0C */ },
    { NULL,         DASM("")            /* 0xFD 0x0D */ },
    { NULL,         DASM("")            /* 0xFD 0x0E */ },
    { NULL,         DASM("")            /* 0xFD 0x0F */ },
    { NULL,         DASM("")            /* 0xFD 0x10 */ },
    { NULL,         DASM("")            /* 0xFD 0x11 */ },
    { NULL,         DASM("")            /* 0xFD 0x12 */ },
    { NULL,         DASM("")            /* 0xFD 0x13 */ },
    { NULL,         DASM("")            /* 0xFD 0x14 */ },
    { NULL,         DASM("")            /* 0xFD 0x15 */ },
    { NULL,         DASM("")            /* 0xFD 0x16 */ },
    { NULL,         DASM("")            /* 0xFD 0x17 */ },
    { NULL,         DASM("")            /* 0xFD 0x18 */ },
    { add_iy_de,    DASM("ADD IY,DE")   /* 0xFD 0x19 */ },
    { NULL,         DASM("")            /* 0xFD 0x1A */ },
    { NULL,         DASM("")            /* 0xFD 0x1B */ },
    { NULL,         DASM("")            /* 0xFD 0x1C */ },
    { NULL,         DASM("")            /* 0xFD 0x1D */ },
    { NULL,         DASM("")            /* 0xFD 0x1E */ },
    { NULL,         DASM("")            /* 0xFD 0x1F */ },
    { NULL,         DASM("")            /* 0xFD 0x20 */ },
    { ld_iy_nn,     DASM("LD IY,#nn")   /* 0xFD 0x21 */ },
    { ld_mnn_iy,    DASM("LD (#nn),IY") /* 0xFD 0x22 */ },
    { inc_iy,       DASM("INC IY")      /* 0xFD 0x23 */ },
    { inc_iyh,      DASM("INC IYh")     /* 0xFD 0x24 */ },
    { dec_iyh,      DASM("DEC IYh")     /* 0xFD 0x25 */ },
    { ld_iyh_n,     DASM("LD IYh,#n")   /* 0xFD 0x26 */ },
    { NULL,         DASM("")            /* 0xFD 0x27 */ },
    { NULL,         DASM("")            /* 0xFD 0x28 */ },
    { add_iy_iy,    DASM("ADD IY,IY")   /* 0xFD 0x29 */ },
    { ld_iy_mnn,    DASM("LD IY,(#nn)") /* 0xFD 0x2A */ },
    { dec_iy,       DASM("DEC IY")      /* 0xFD 0x2B */ },
    { inc_iyl,      DASM("INC IYl")     /* 0xFD 0x2C */ },
    { dec_iyl,      DASM("DEC IYl")     /* 0xFD 0x2D */ },
    { ld_iyl_n,     DASM("LD IYl,#n")   /* 0xFD 0x2E */ },
    { NULL,         DASM("")            /* 0xFD 0x2F */ },
    { NULL,         DASM("")            /* 0xFD 0x30 */ },
    { NULL,         DASM("")            /* 0xFD 0x31 */ },
    { NULL,         DASM("")            /* 0xFD 0x32 */ },
    { NULL,         DASM("")            /* 0xFD 0x33 */ },
    { inc_miyd,     DASM("INC (IY+#d)") /* 0xFD 0x34 */ },
    { dec_miyd,     DASM("DEC (IY+#d)") /* 0xFD 0x35 */ },
    { ld_miyd_n,    DASM("LD (IY+#d),#n")/* 0xFD 0x36 */ },
    { NULL,         DASM("")            /* 0xFD 0x37 */ },
    { NULL,         DASM("")            /* 0xFD 0x38 */ },
    { add_iy_sp,    DASM("ADD IY,SP")   /* 0xFD 0x39 */ },
    { NULL,         DASM("")            /* 0xFD 0x3A */ },
    { NULL,         DASM("")            /* 0xFD 0x3B */ },
    { NULL,         DASM("")            /* 0xFD 0x3C */ },
    { NULL,         DASM("")            /* 0xFD 0x3D */ },
    { NULL,         DASM("")            /* 0xFD 0x3E */ },
    { NULL,         DASM("")            /* 0xFD 0x3F */ },
    { NULL,         DASM("")            /* 0xFD 0x40 */ },
    { NULL,         DASM("")            /* 0xFD 0x41 */ },
    { NULL,         DASM("")            /* 0xFD 0x42 */ },
    { NULL,         DASM("")            /* 0xFD 0x43 */ },
    { NULL,         DASM("")            /* 0xFD 0x44 */ },
    { NULL,         DASM("")            /* 0xFD 0x45 */ },
    { ld_b_miyd,    DASM("LD B,(IY+#d)")/* 0xFD 0x46 */ },
    { NULL,         DASM("")            /* 0xFD 0x47 */ },
    { NULL,         DASM("")            /* 0xFD 0x48 */ },
    { NULL,         DASM("")            /* 0xFD 0x49 */ },
    { NULL,         DASM("")            /* 0xFD 0x4A */ },
    { NULL,         DASM("")            /* 0xFD 0x4B */ },
    { NULL,         DASM("")            /* 0xFD 0x4C */ },
    { NULL,         DASM("")            /* 0xFD 0x4D */ },
    { ld_c_miyd,    DASM("LD C,(IY+#d)")/* 0xFD 0x4E */ },
    { NULL,         DASM("")            /* 0xFD 0x4F */ },
    { NULL,         DASM("")            /* 0xFD 0x50 */ },
    { NULL,         DASM("")            /* 0xFD 0x51 */ },
    { NULL,         DASM("")            /* 0xFD 0x52 */ },
    { NULL,         DASM("")            /* 0xFD 0x53 */ },
    { ld_d_iyh,     DASM("LD D,IYh")    /* 0xFD 0x54 */ },
    { ld_d_iyl,     DASM("LD D,IYl")    /* 0xFD 0x55 */ },
    { ld_d_miyd,    DASM("LD D,(IY+#d)")/* 0xFD 0x56 */ },
    { NULL,         DASM("")            /* 0xFD 0x57 */ },
    { NULL,         DASM("")            /* 0xFD 0x58 */ },
    { NULL,         DASM("")            /* 0xFD 0x59 */ },
    { NULL,         DASM("")            /* 0xFD 0x5A */ },
    { NULL,         DASM("")            /* 0xFD 0x5B */ },
    { ld_e_iyh,     DASM("LD E,IYh")    /* 0xFD 0x5C */ },
    { ld_e_iyl,     DASM("LD E,IYl")    /* 0xFD 0x5D */ },
    { ld_e_miyd,    DASM("LD E,(IY+#d)")/* 0xFD 0x5E */ },
    { NULL,         DASM("")            /* 0xFD 0x5F */ },
    { ld_iyh_b,     DASM("LD IYh,B")    /* 0xFD 0x60 */ },
    { ld_iyh_c,     DASM("LD IYh,C")    /* 0xFD 0x61 */ },
    { ld_iyh_d,     DASM("LD IYh,D")    /* 0xFD 0x62 */ },
    { ld_iyh_e,     DASM("LD IYh,E")    /* 0xFD 0x63 */ },
    { NULL,         DASM("")            /* 0xFD 0x64 */ },
    { NULL,         DASM("")            /* 0xFD 0x65 */ },
    { ld_h_miyd,    DASM("LD H,(IY+#d)")/* 0xFD 0x66 */ },
    { ld_iyh_a,     DASM("LD IYh,A")    /* 0xFD 0x67 */ },
    { ld_iyl_b,     DASM("LD IYl,B")    /* 0xFD 0x68 */ },
    { ld_iyl_c,     DASM("LD IYl,C")    /* 0xFD 0x69 */ },
    { ld_iyl_d,     DASM("LD IYl,D")    /* 0xFD 0x6A */ },
    { ld_iyl_e,     DASM("LD IYl,E")    /* 0xFD 0x6B */ },
    { NULL,         DASM("")            /* 0xFD 0x6C */ },
    { NULL,         DASM("")            /* 0xFD 0x6D */ },
    { ld_l_miyd,    DASM("LD L,(IY+#d)")/* 0xFD 0x6E */ },
    { ld_iyl_a,     DASM("LD IYl,A")    /* 0xFD 0x6F */ },
    { ld_miyd_b,    DASM("LD (IY+#d),B")/* 0xFD 0x70 */ },
    { ld_miyd_c,    DASM("LD (IY+#d),C")/* 0xFD 0x71 */ },
    { ld_miyd_d,    DASM("LD (IY+#d),D")/* 0xFD 0x72 */ },
    { ld_miyd_e,    DASM("LD (IY+#d),E")/* 0xFD 0x73 */ },
    { ld_miyd_h,    DASM("LD (IY+#d),H")/* 0xFD 0x74 */ },
    { ld_miyd_l,    DASM("LD (IY+#d),L")/* 0xFD 0x75 */ },
    { NULL,         DASM("")            /* 0xFD 0x76 */ },
    { ld_miyd_a,    DASM("LD (IY+#d),A")/* 0xFD 0x77 */ },
    { NULL,         DASM("")            /* 0xFD 0x78 */ },
    { NULL,         DASM("")            /* 0xFD 0x79 */ },
    { NULL,         DASM("")            /* 0xFD 0x7A */ },
    { NULL,         DASM("")            /* 0xFD 0x7B */ },
    { ld_a_iyh,     DASM("LD A,IYh")    /* 0xFD 0x7C */ },
    { ld_a_iyl,     DASM("LD A,IYl")    /* 0xFD 0x7D */ },
    { ld_a_miyd,    DASM("LD A,(IY+#d)")/* 0xFD 0x7E */ },
    { NULL,         DASM("")            /* 0xFD 0x7F */ },
    { NULL,         DASM("")            /* 0xFD 0x80 */ },
    { NULL,         DASM("")            /* 0xFD 0x81 */ },
    { NULL,         DASM("")            /* 0xFD 0x82 */ },
    { NULL,         DASM("")            /* 0xFD 0x83 */ },
    { NULL,         DASM("")            /* 0xFD 0x84 */ },
    { NULL,         DASM("")            /* 0xFD 0x85 */ },
    { add_a_miyd,   DASM("ADD A,(IY+#d)")/* 0xFD 0x86 */ },
    { NULL,         DASM("")            /* 0xFD 0x87 */ },
    { NULL,         DASM("")            /* 0xFD 0x88 */ },
    { NULL,         DASM("")            /* 0xFD 0x89 */ },
    { NULL,         DASM("")            /* 0xFD 0x8A */ },
    { NULL,         DASM("")            /* 0xFD 0x8B */ },
    { NULL,         DASM("")            /* 0xFD 0x8C */ },
    { NULL,         DASM("")            /* 0xFD 0x8D */ },
    { adc_a_miyd,   DASM("ADC A,(IY+#d)")/* 0xFD 0x8E */ },
    { NULL,         DASM("")            /* 0xFD 0x8F */ },
    { NULL,         DASM("")            /* 0xFD 0x90 */ },
    { NULL,         DASM("")            /* 0xFD 0x91 */ },
    { NULL,         DASM("")            /* 0xFD 0x92 */ },
    { NULL,         DASM("")            /* 0xFD 0x93 */ },
    { NULL,         DASM("")            /* 0xFD 0x94 */ },
    { NULL,         DASM("")            /* 0xFD 0x95 */ },
    { sub_miyd,     DASM("SUB (IY+#d)") /* 0xFD 0x96 */ },
    { NULL,         DASM("")            /* 0xFD 0x97 */ },
    { NULL,         DASM("")            /* 0xFD 0x98 */ },
    { NULL,         DASM("")            /* 0xFD 0x99 */ },
    { NULL,         DASM("")            /* 0xFD 0x9A */ },
    { NULL,         DASM("")            /* 0xFD 0x9B */ },
    { NULL,         DASM("")            /* 0xFD 0x9C */ },
    { NULL,         DASM("")            /* 0xFD 0x9D */ },
    { sbc_a_miyd,   DASM("SBC A,(IY+#d)")/* 0xFD 0x9E */ },
    { NULL,         DASM("")            /* 0xFD 0x9F */ },
    { NULL,         DASM("")            /* 0xFD 0xA0 */ },
    { NULL,         DASM("")            /* 0xFD 0xA1 */ },
    { NULL,         DASM("")            /* 0xFD 0xA2 */ },
    { NULL,         DASM("")            /* 0xFD 0xA3 */ },
    { NULL,         DASM("")            /* 0xFD 0xA4 */ },
    { NULL,         DASM("")            /* 0xFD 0xA5 */ },
    { and_miyd,     DASM("AND (IY+#d)") /* 0xFD 0xA6 */ },
    { NULL,         DASM("")            /* 0xFD 0xA7 */ },
    { NULL,         DASM("")            /* 0xFD 0xA8 */ },
    { NULL,         DASM("")            /* 0xFD 0xA9 */ },
    { NULL,         DASM("")            /* 0xFD 0xAA */ },
    { NULL,         DASM("")            /* 0xFD 0xAB */ },
    { NULL,         DASM("")            /* 0xFD 0xAC */ },
    { NULL,         DASM("")            /* 0xFD 0xAD */ },
    { xor_miyd,     DASM("XOR (IY+#d)") /* 0xFD 0xAE */ },
    { NULL,         DASM("")            /* 0xFD 0xAF */ },
    { NULL,         DASM("")            /* 0xFD 0xB0 */ },
    { NULL,         DASM("")            /* 0xFD 0xB1 */ },
    { NULL,         DASM("")            /* 0xFD 0xB2 */ },
    { NULL,         DASM("")            /* 0xFD 0xB3 */ },
    { NULL,         DASM("")            /* 0xFD 0xB4 */ },
    { NULL,         DASM("")            /* 0xFD 0xB5 */ },
    { or_miyd,      DASM("OR (IY+#d)")  /* 0xFD 0xB6 */ },
    { NULL,         DASM("")            /* 0xFD 0xB7 */ },
    { NULL,         DASM("")            /* 0xFD 0xB8 */ },
    { NULL,         DASM("")            /* 0xFD 0xB9 */ },
    { NULL,         DASM("")            /* 0xFD 0xBA */ },
    { NULL,         DASM("")            /* 0xFD 0xBB */ },
    { NULL,         DASM("")            /* 0xFD 0xBC */ },
    { NULL,         DASM("")            /* 0xFD 0xBD */ },
    { cp_miyd,      DASM("CP (IY+#d)")  /* 0xFD 0xBE */ },
    { NULL,         DASM("")            /* 0xFD 0xBF */ },
    { NULL,         DASM("")            /* 0xFD 0xC0 */ },
    { NULL,         DASM("")            /* 0xFD 0xC1 */ },
    { NULL,         DASM("")            /* 0xFD 0xC2 */ },
    { NULL,         DASM("")            /* 0xFD 0xC3 */ },
    { NULL,         DASM("")            /* 0xFD 0xC4 */ },
    { NULL,         DASM("")            /* 0xFD 0xC5 */ },
    { NULL,         DASM("")            /* 0xFD 0xC6 */ },
    { NULL,         DASM("")            /* 0xFD 0xC7 */ },
    { NULL,         DASM("")            /* 0xFD 0xC8 */ },
    { NULL,         DASM("")            /* 0xFD 0xC9 */ },
    { NULL,         DASM("")            /* 0xFD 0xCA */ },
    { opcode_fdcb,  NULL                /* 0xFD 0xCB */ },
    { NULL,         DASM("")            /* 0xFD 0xCC */ },
    { NULL,         DASM("")            /* 0xFD 0xCD */ },
    { NULL,         DASM("")            /* 0xFD 0xCE */ },
    { NULL,         DASM("")            /* 0xFD 0xCF */ },
    { NULL,         DASM("")            /* 0xFD 0xD0 */ },
    { NULL,         DASM("")            /* 0xFD 0xD1 */ },
    { NULL,         DASM("")            /* 0xFD 0xD2 */ },
    { NULL,         DASM("")            /* 0xFD 0xD3 */ },
    { NULL,         DASM("")            /* 0xFD 0xD4 */ },
    { NULL,         DASM("")            /* 0xFD 0xD5 */ },
    { NULL,         DASM("")            /* 0xFD 0xD6 */ },
    { NULL,         DASM("")            /* 0xFD 0xD7 */ },
    { NULL,         DASM("")            /* 0xFD 0xD8 */ },
    { NULL,         DASM("")            /* 0xFD 0xD9 */ },
    { NULL,         DASM("")            /* 0xFD 0xDA */ },
    { NULL,         DASM("")            /* 0xFD 0xDB */ },
    { NULL,         DASM("")            /* 0xFD 0xDC */ },
    { NULL,         DASM("")            /* 0xFD 0xDD */ },
    { NULL,         DASM("")            /* 0xFD 0xDE */ },
    { NULL,         DASM("")            /* 0xFD 0xDF */ },
    { NULL,         DASM("")            /* 0xFD 0xE0 */ },
    { pop_iy,       DASM("POP IY")      /* 0xFD 0xE1 */ },
    { NULL,         DASM("")            /* 0xFD 0xE2 */ },
    { ex_msp_iy,    DASM("EX (SP),IY")  /* 0xFD 0xE3 */ },
    { NULL,         DASM("")            /* 0xFD 0xE4 */ },
    { push_iy,      DASM("PUSH IY")     /* 0xFD 0xE5 */ },
    { NULL,         DASM("")            /* 0xFD 0xE6 */ },
    { NULL,         DASM("")            /* 0xFD 0xE7 */ },
    { NULL,         DASM("")            /* 0xFD 0xE8 */ },
    { jp_iy,        DASM("JP IY")       /* 0xFD 0xE9 */ },
    { NULL,         DASM("")            /* 0xFD 0xEA */ },
    { NULL,         DASM("")            /* 0xFD 0xEB */ },
    { NULL,         DASM("")            /* 0xFD 0xEC */ },
    { NULL,         DASM("")            /* 0xFD 0xED */ },
    { NULL,         DASM("")            /* 0xFD 0xEE */ },
    { NULL,         DASM("")            /* 0xFD 0xEF */ },
    { NULL,         DASM("")            /* 0xFD 0xF0 */ },
    { NULL,         DASM("")            /* 0xFD 0xF1 */ },
    { NULL,         DASM("")            /* 0xFD 0xF2 */ },
    { NULL,         DASM("")            /* 0xFD 0xF3 */ },
    { NULL,         DASM("")            /* 0xFD 0xF4 */ },
    { NULL,         DASM("")            /* 0xFD 0xF5 */ },
    { NULL,         DASM("")            /* 0xFD 0xF6 */ },
    { NULL,         DASM("")            /* 0xFD 0xF7 */ },
    { NULL,         DASM("")            /* 0xFD 0xF8 */ },
    { NULL,         DASM("")            /* 0xFD 0xF9 */ },
    { NULL,         DASM("")            /* 0xFD 0xFA */ },
    { NULL,         DASM("")            /* 0xFD 0xFB */ },
    { NULL,         DASM("")            /* 0xFD 0xFC */ },
    { NULL,         DASM("")            /* 0xFD 0xFD */ },
    { NULL,         DASM("")            /* 0xFD 0xFE */ },
    { NULL,         DASM("")            /* 0xFD 0xFF */ }
};

void opcode_fd(cpuZ80 *cpu)
{
    word pc = cpu->PC-1;

    int op = (int)read8(cpu, cpu->PC++);

    if(opcodes_fd[op].func==NULL) {
        log4me_error(LOG_EMU_Z80, "0x%04X : opcode = 0xFD 0x%02X not implemented\n", pc, op);
        exit(EXIT_FAILURE);
    }

    cpu->R++;
    opcodes_fd[op].func(cpu);
}

#ifdef DEBUG
void dasmopcode_fdcb(cpuZ80 *cpu, word addr, char *buffer, int len);

void dasmopcode_fd(cpuZ80 *cpu, word addr, char *buffer, int len)
{
    int op = (int)read8(cpu, addr);

    if(opcodes_fd[op].func==opcode_fdcb)
        dasmopcode_fdcb(cpu, addr+1, buffer, len);
    else
        dasmopcode(cpu, opcodes_fd[op].dasm, addr+1, buffer, len);
}
#endif /* DEBUG */
