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

void opcode_ddcb(cpuZ80 *cpu);

static void add_ix_bc(cpuZ80 *cpu) /* 0xDD 0x09 */
{
    cpu->cycles += 15;
    cpu->IX = add16(cpu, cpu->IX, cpu->w.BC);
}

static void add_ix_de(cpuZ80 *cpu) /* 0xDD 0x19 */
{
    cpu->cycles += 15;
    cpu->IX = add16(cpu, cpu->IX, cpu->w.DE);
}

static void ld_ix_nn(cpuZ80 *cpu) /* 0xDD 0x21 */
{
    cpu->cycles += 14;
    cpu->IX = read16(cpu, cpu->PC);
    cpu->PC += 2;
}

static void ld_mnn_ix(cpuZ80 *cpu) /* 0xDD 0x22 */
{
    cpu->cycles += 20;
    write16(cpu, read16(cpu, cpu->PC), cpu->IX);
    cpu->PC += 2;
}

static void inc_ix(cpuZ80 *cpu) /* 0xDD 0x21 */
{
    cpu->cycles += 10;
    cpu->IX++;
}

static void inc_ixh(cpuZ80 *cpu) /* 0xDD 0x24 */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0x00FF) | ((word)inc8(cpu, cpu->IX >> 8) << 8);
}

static void dec_ixh(cpuZ80 *cpu) /* 0xDD 0x25 */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0x00FF) | ((word)dec8(cpu, cpu->IX >> 8) << 8);
}

static void ld_ixh_n(cpuZ80 *cpu) /* 0xDD 0x26 */
{
    cpu->cycles += 11;
    cpu->IX = (cpu->IX & 0x00FF) | ((word)read8(cpu, cpu->PC++) << 8);
}

static void add_ix_ix(cpuZ80 *cpu) /* 0xDD 0x29 */
{
    cpu->cycles += 15;
    cpu->IX = add16(cpu, cpu->IX, cpu->IX);
}

static void ld_ix_mnn(cpuZ80 *cpu) /* 0xDD 0x2A */
{
    cpu->cycles += 20;
    cpu->IX = read16(cpu, read16(cpu, cpu->PC));
    cpu->PC += 2;
}

static void dec_ix(cpuZ80 *cpu) /* 0xDD 0x2B */
{
    cpu->cycles += 10;
    cpu->IX--;
}

static void inc_ixl(cpuZ80 *cpu) /* 0xDD 0x2C */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0xFF00) | inc8(cpu, cpu->IX);
}

static void dec_ixl(cpuZ80 *cpu) /* 0xDD 0x2D */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0xFF00) | dec8(cpu, cpu->IX);
}

static void ld_ixl_n(cpuZ80 *cpu) /* 0xDD 0x2E */
{
    cpu->cycles += 11;
    cpu->IX = (cpu->IX & 0xFF00) | read8(cpu, cpu->PC++);
}

static void inc_mixd(cpuZ80 *cpu) /* 0xDD 0x34 */
{
    cpu->cycles += 23;
    word addr = cpu->IX + sread8(cpu, cpu->PC++);
    write8(cpu, addr, inc8(cpu, read8(cpu, addr)));
}

static void dec_mixd(cpuZ80 *cpu) /* 0xDD 0x35 */
{
    cpu->cycles += 23;
    word addr = cpu->IX + sread8(cpu, cpu->PC++);
    write8(cpu, addr, dec8(cpu, read8(cpu, addr)));
}

static void ld_mixd_n(cpuZ80 *cpu) /* 0xDD 0x36 */
{
    cpu->cycles += 19;
    word addr = cpu->IX + sread8(cpu, cpu->PC++);
    write8(cpu, addr, read8(cpu, cpu->PC++));
}

static void add_ix_sp(cpuZ80 *cpu) /* 0xDD 0x39 */
{
    cpu->cycles += 15;
    cpu->IX = add16(cpu, cpu->IX, cpu->SP);
}

static void ld_b_ixh(cpuZ80 *cpu) /* 0xDD 0x44 */
{
    cpu->cycles += 8;
    cpu->b.B = ((cpu->IX & 0xFF00) >> 8);
}

static void ld_b_ixl(cpuZ80 *cpu) /* 0xDD 0x45 */
{
    cpu->cycles += 8;
    cpu->b.B = cpu->IX & 0x00FF;
}

static void ld_b_mixd(cpuZ80 *cpu) /* 0xDD 0x46 */
{
    cpu->cycles += 19;
    cpu->b.B = read8(cpu, cpu->IX+sread8(cpu, cpu->PC++));
}

static void ld_c_ixh(cpuZ80 *cpu) /* 0xDD 0x4C */
{
    cpu->cycles += 8;
    cpu->b.C = ((cpu->IX & 0xFF00) >> 8);
}

static void ld_c_ixl(cpuZ80 *cpu) /* 0xDD 0x4D */
{
    cpu->cycles += 8;
    cpu->b.C = cpu->IX & 0x00FF;
}

static void ld_c_mixd(cpuZ80 *cpu) /* 0xDD 0x4E */
{
    cpu->cycles += 19;
    cpu->b.C = read8(cpu, cpu->IX+sread8(cpu, cpu->PC++));
}

static void ld_d_ixh(cpuZ80 *cpu) /* 0xDD 0x54 */
{
    cpu->cycles += 8;
    cpu->b.D = ((cpu->IX & 0xFF00) >> 8);
}

static void ld_d_ixl(cpuZ80 *cpu) /* 0xDD 0x55 */
{
    cpu->cycles += 8;
    cpu->b.D = cpu->IX & 0x00FF;
}

static void ld_d_mixd(cpuZ80 *cpu) /* 0xDD 0x56 */
{
    cpu->cycles += 19;
    cpu->b.D = read8(cpu, cpu->IX+sread8(cpu, cpu->PC++));
}

static void ld_e_ixh(cpuZ80 *cpu) /* 0xDD 0x5C */
{
    cpu->cycles += 8;
    cpu->b.E = ((cpu->IX & 0xFF00) >> 8);
}

static void ld_e_ixl(cpuZ80 *cpu) /* 0xDD 0x5D */
{
    cpu->cycles += 8;
    cpu->b.E = cpu->IX & 0x00FF;
}

static void ld_e_mixd(cpuZ80 *cpu) /* 0xDD 0x5E */
{
    cpu->cycles += 19;
    cpu->b.E = read8(cpu, cpu->IX+sread8(cpu, cpu->PC++));
}

static void ld_ixh_b(cpuZ80 *cpu) /* 0xDD 0x60 */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0x00FF) | ((word)cpu->b.B << 8);
}

static void ld_ixh_c(cpuZ80 *cpu) /* 0xDD 0x61 */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0x00FF) | ((word)cpu->b.C << 8);
}

static void ld_ixh_d(cpuZ80 *cpu) /* 0xDD 0x62 */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0x00FF) | ((word)cpu->b.D << 8);
}

static void ld_ixh_e(cpuZ80 *cpu) /* 0xDD 0x63 */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0x00FF) | ((word)cpu->b.E << 8);
}

static void ld_ixh_ixh(cpuZ80 *cpu) /* 0xDD 0x64 */
{
    cpu->cycles += 8;
}

static void ld_ixh_ixl(cpuZ80 *cpu) /* 0xDD 0x65 */
{
    cpu->cycles += 8;
    byte ixl = cpu->IX & 0x00FF;
    cpu->IX = (cpu->IX << 8) | ixl;
}

static void ld_h_mixd(cpuZ80 *cpu) /* 0xDD 0x66 */
{
    cpu->cycles += 19;
    cpu->b.H = read8(cpu, cpu->IX+sread8(cpu, cpu->PC++));
}

static void ld_ixh_a(cpuZ80 *cpu) /* 0xDD 0x67 */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0x00FF) | ((word)cpu->b.A << 8);
}

static void ld_ixl_b(cpuZ80 *cpu) /* 0xDD 0x68 */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0xFF00) | cpu->b.B;
}

static void ld_ixl_c(cpuZ80 *cpu) /* 0xDD 0x69 */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0xFF00) | cpu->b.C;
}

static void ld_ixl_d(cpuZ80 *cpu) /* 0xDD 0x6A */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0xFF00) | cpu->b.D;
}

static void ld_ixl_e(cpuZ80 *cpu) /* 0xDD 0x6B */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0xFF00) | cpu->b.E;
}

static void ld_ixl_ixh(cpuZ80 *cpu) /* 0xDD 0x6C */
{
    cpu->cycles += 8;
    byte ixh = cpu->IX >> 8;
    cpu->IX = (cpu->IX & 0xFF00) | ixh;
}

static void ld_ixl_ixl(cpuZ80 *cpu) /* 0xDD 0x6D */
{
    cpu->cycles += 8;
}

static void ld_l_mixd(cpuZ80 *cpu) /* 0xDD 0x6E */
{
    cpu->cycles += 19;
    cpu->b.L = read8(cpu, cpu->IX+sread8(cpu, cpu->PC++));
}

static void ld_ixl_a(cpuZ80 *cpu) /* 0xDD 0x6F */
{
    cpu->cycles += 8;
    cpu->IX = (cpu->IX & 0xFF00) | cpu->b.A;
}

static void ld_mixd_b(cpuZ80 *cpu) /* 0xDD 0x70 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IX+sread8(cpu, cpu->PC++), cpu->b.B);
}

static void ld_mixd_c(cpuZ80 *cpu) /* 0xDD 0x71 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IX+sread8(cpu, cpu->PC++), cpu->b.C);
}

static void ld_mixd_d(cpuZ80 *cpu) /* 0xDD 0x72 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IX+sread8(cpu, cpu->PC++), cpu->b.D);
}

static void ld_mixd_e(cpuZ80 *cpu) /* 0xDD 0x73 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IX+sread8(cpu, cpu->PC++), cpu->b.E);
}

static void ld_mixd_h(cpuZ80 *cpu) /* 0xDD 0x74 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IX+sread8(cpu, cpu->PC++), cpu->b.H);
}

static void ld_mixd_l(cpuZ80 *cpu) /* 0xDD 0x75 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IX+sread8(cpu, cpu->PC++), cpu->b.L);
}

static void ld_mixd_a(cpuZ80 *cpu) /* 0xDD 0x77 */
{
    cpu->cycles += 19;
    write8(cpu, cpu->IX+sread8(cpu, cpu->PC++), cpu->b.A);
}

static void ld_a_ixh(cpuZ80 *cpu) /* 0xDD 0x7C */
{
    cpu->cycles += 8;
    cpu->b.A = ((cpu->IX & 0xFF00) >> 8);
}

static void ld_a_ixl(cpuZ80 *cpu) /* 0xDD 0x7D */
{
    cpu->cycles += 8;
    cpu->b.A = cpu->IX & 0x00FF;
}

static void ld_a_mixd(cpuZ80 *cpu) /* 0xDD 0x7E */
{
    cpu->cycles += 19;
    cpu->b.A = read8(cpu, cpu->IX+sread8(cpu, cpu->PC++));
}

static void add_a_ixh(cpuZ80 *cpu) /* 0xDD 0x84 */
{
    cpu->cycles += 8;
    cpu->b.A = add8(cpu, cpu->b.A, cpu->IX >> 8);
}

static void add_a_ixl(cpuZ80 *cpu) /* 0xDD 0x85 */
{
    cpu->cycles += 8;
    cpu->b.A = add8(cpu, cpu->b.A, cpu->IX & 0x00FF);
}

static void add_a_mixd(cpuZ80 *cpu) /* 0xDD 0x86 */
{
    cpu->cycles += 19;
    cpu->b.A = add8(cpu, cpu->b.A, read8(cpu, cpu->IX+sread8(cpu, cpu->PC++)));
}

static void adc_a_ixh(cpuZ80 *cpu) /* 0xDD 0x8C */
{
    cpu->cycles += 8;
    cpu->b.A = adc8(cpu, cpu->b.A, cpu->IX >> 8);
}

static void adc_a_ixl(cpuZ80 *cpu) /* 0xDD 0x8D */
{
    cpu->cycles += 8;
    cpu->b.A = adc8(cpu, cpu->b.A, cpu->IX & 0x00FF);
}

static void adc_a_mixd(cpuZ80 *cpu) /* 0xDD 0x8E */
{
    cpu->cycles += 19;
    cpu->b.A = adc8(cpu, cpu->b.A, read8(cpu, cpu->IX+sread8(cpu, cpu->PC++)));
}

static void sub_ixh(cpuZ80 *cpu) /* 0xDD 0x94 */
{
    cpu->cycles += 8;
    cpu->b.A = sub8(cpu, cpu->b.A, cpu->IX >> 8);
}

static void sub_ixl(cpuZ80 *cpu) /* 0xDD 0x95 */
{
    cpu->cycles += 8;
    cpu->b.A = sub8(cpu, cpu->b.A, cpu->IX & 0x00FF);
}

static void sub_mixd(cpuZ80 *cpu) /* 0xDD 0x96 */
{
    cpu->cycles += 19;
    cpu->b.A = sub8(cpu, cpu->b.A, read8(cpu, cpu->IX+sread8(cpu, cpu->PC++)));
}

static void sbc_a_ixh(cpuZ80 *cpu) /* 0xDD 0x9C */
{
    cpu->cycles += 8;
    cpu->b.A = sbc8(cpu, cpu->b.A, cpu->IX >> 8);
}

static void sbc_a_ixl(cpuZ80 *cpu) /* 0xDD 0x9D */
{
    cpu->cycles += 8;
    cpu->b.A = sbc8(cpu, cpu->b.A, cpu->IX & 0x00FF);
}

static void sbc_a_mixd(cpuZ80 *cpu) /* 0xDD 0x9E */
{
    cpu->cycles += 19;
    cpu->b.A = sbc8(cpu, cpu->b.A, read8(cpu, cpu->IX+sread8(cpu, cpu->PC++)));
}

static void and_ixh(cpuZ80 *cpu) /* 0xDD 0xA4 */
{
    cpu->cycles += 8;
    cpu->b.A = and8(cpu, cpu->b.A, cpu->IX >> 8);
}

static void and_ixl(cpuZ80 *cpu) /* 0xDD 0xA5 */
{
    cpu->cycles += 8;
    cpu->b.A = and8(cpu, cpu->b.A, cpu->IX & 0x00FF);
}

static void and_mixd(cpuZ80 *cpu) /* 0xDD 0xA6 */
{
    cpu->cycles += 19;
    cpu->b.A = and8(cpu, cpu->b.A, read8(cpu, cpu->IX+sread8(cpu, cpu->PC++)));
}

static void xor_ixh(cpuZ80 *cpu) /* 0xDD 0xAC */
{
    cpu->cycles += 8;
    cpu->b.A = xor8(cpu, cpu->b.A, cpu->IX >> 8);
}

static void xor_ixl(cpuZ80 *cpu) /* 0xDD 0xAD */
{
    cpu->cycles += 8;
    cpu->b.A = xor8(cpu, cpu->b.A, cpu->IX & 0x00FF);
}

static void xor_mixd(cpuZ80 *cpu) /* 0xDD 0xAE */
{
    cpu->cycles += 19;
    cpu->b.A = xor8(cpu, cpu->b.A, read8(cpu, cpu->IX+sread8(cpu, cpu->PC++)));
}

static void or_ixh(cpuZ80 *cpu) /* 0xDD 0xB4 */
{
    cpu->cycles += 8;
    cpu->b.A = or8(cpu, cpu->b.A, cpu->IX >> 8);
}

static void or_ixl(cpuZ80 *cpu) /* 0xDD 0xB5 */
{
    cpu->cycles += 8;
    cpu->b.A = or8(cpu, cpu->b.A, cpu->IX & 0x00FF);
}

static void or_mixd(cpuZ80 *cpu) /* 0xDD 0xB6 */
{
    cpu->cycles += 19;
    cpu->b.A = or8(cpu, cpu->b.A, read8(cpu, cpu->IX+sread8(cpu, cpu->PC++)));
}

static void cp_ixh(cpuZ80 *cpu) /* 0xDD 0xBC */
{
    cpu->cycles += 8;
    sub8(cpu, cpu->b.A, cpu->IX >> 8);
}

static void cp_ixl(cpuZ80 *cpu) /* 0xDD 0xBD */
{
    cpu->cycles += 8;
    sub8(cpu, cpu->b.A, cpu->IX & 0x00FF);
}

static void cp_mixd(cpuZ80 *cpu) /* 0xDD 0xBE */
{
    cpu->cycles += 19;
    sub8(cpu, cpu->b.A, read8(cpu, cpu->IX+sread8(cpu, cpu->PC++)));
}

static void pop_ix(cpuZ80 *cpu) /* 0xDD 0xE1 */
{
    cpu->cycles += 14;
    POP(cpu->IX);
}

static void ex_msp_ix(cpuZ80 *cpu) /* 0xDD 0xE3 */
{
    cpu->cycles += 23;
    word value = read16(cpu, cpu->SP);
    write16(cpu, cpu->SP, cpu->IX);
    cpu->IX = value;
}

static void push_ix(cpuZ80 *cpu) /* 0xDD 0xE5 */
{
    cpu->cycles += 15;
    PUSH(cpu->IX);
}

static void jp_ix(cpuZ80 *cpu) /* 0xDD 0xE9 */
{
    cpu->cycles += 8;
    cpu->PC = cpu->IX;
}

static void ld_sp_ix(cpuZ80 *cpu) /* 0xDD 0xF9 */
{
    cpu->cycles += 10;
    cpu->SP = cpu->IX;
}

opcode opcodes_dd[256] = {
    { NULL,         DASM("")            /* 0xDD 0x00 */ },
    { NULL,         DASM("")            /* 0xDD 0x01 */ },
    { NULL,         DASM("")            /* 0xDD 0x02 */ },
    { NULL,         DASM("")            /* 0xDD 0x03 */ },
    { NULL,         DASM("")            /* 0xDD 0x04 */ },
    { NULL,         DASM("")            /* 0xDD 0x05 */ },
    { NULL,         DASM("")            /* 0xDD 0x06 */ },
    { NULL,         DASM("")            /* 0xDD 0x07 */ },
    { NULL,         DASM("")            /* 0xDD 0x08 */ },
    { add_ix_bc,    DASM("ADD IX,BC")   /* 0xDD 0x09 */ },
    { NULL,         DASM("")            /* 0xDD 0x0A */ },
    { NULL,         DASM("")            /* 0xDD 0x0B */ },
    { NULL,         DASM("")            /* 0xDD 0x0C */ },
    { NULL,         DASM("")            /* 0xDD 0x0D */ },
    { NULL,         DASM("")            /* 0xDD 0x0E */ },
    { NULL,         DASM("")            /* 0xDD 0x0F */ },
    { NULL,         DASM("")            /* 0xDD 0x10 */ },
    { NULL,         DASM("")            /* 0xDD 0x11 */ },
    { NULL,         DASM("")            /* 0xDD 0x12 */ },
    { NULL,         DASM("")            /* 0xDD 0x13 */ },
    { NULL,         DASM("")            /* 0xDD 0x14 */ },
    { NULL,         DASM("")            /* 0xDD 0x15 */ },
    { NULL,         DASM("")            /* 0xDD 0x16 */ },
    { NULL,         DASM("")            /* 0xDD 0x17 */ },
    { NULL,         DASM("")            /* 0xDD 0x18 */ },
    { add_ix_de,    DASM("ADD IX,DE")   /* 0xDD 0x19 */ },
    { NULL,         DASM("")            /* 0xDD 0x1A */ },
    { NULL,         DASM("")            /* 0xDD 0x1B */ },
    { NULL,         DASM("")            /* 0xDD 0x1C */ },
    { NULL,         DASM("")            /* 0xDD 0x1D */ },
    { NULL,         DASM("")            /* 0xDD 0x1E */ },
    { NULL,         DASM("")            /* 0xDD 0x1F */ },
    { NULL,         DASM("")            /* 0xDD 0x20 */ },
    { ld_ix_nn,     DASM("LD IX,#nn")   /* 0xDD 0x21 */ },
    { ld_mnn_ix,    DASM("LD (#nn),IX") /* 0xDD 0x22 */ },
    { inc_ix,       DASM("INC IX")      /* 0xDD 0x23 */ },
    { inc_ixh,      DASM("INC IXh")     /* 0xDD 0x24 */ },
    { dec_ixh,      DASM("DEC IXh")     /* 0xDD 0x25 */ },
    { ld_ixh_n,     DASM("LD IXh,#n")   /* 0xDD 0x26 */ },
    { NULL,         DASM("")            /* 0xDD 0x27 */ },
    { NULL,         DASM("")            /* 0xDD 0x28 */ },
    { add_ix_ix,    DASM("ADD IX,IX")   /* 0xDD 0x29 */ },
    { ld_ix_mnn,    DASM("LD IX,(#nn)") /* 0xDD 0x2A */ },
    { dec_ix,       DASM("DEC IX")      /* 0xDD 0x2B */ },
    { inc_ixl,      DASM("INC IXl")     /* 0xDD 0x2C */ },
    { dec_ixl,      DASM("DEC IXl")     /* 0xDD 0x2D */ },
    { ld_ixl_n,     DASM("LD IXl,#n")   /* 0xDD 0x2E */ },
    { NULL,         DASM("")            /* 0xDD 0x2F */ },
    { NULL,         DASM("")            /* 0xDD 0x30 */ },
    { NULL,         DASM("")            /* 0xDD 0x31 */ },
    { NULL,         DASM("")            /* 0xDD 0x32 */ },
    { NULL,         DASM("")            /* 0xDD 0x33 */ },
    { inc_mixd,     DASM("INC (IX+#d)") /* 0xDD 0x34 */ },
    { dec_mixd,     DASM("DEC (IX+#d)") /* 0xDD 0x35 */ },
    { ld_mixd_n,    DASM("LD (IX+#d),#n")/* 0xDD 0x36 */ },
    { NULL,         DASM("")            /* 0xDD 0x37 */ },
    { NULL,         DASM("")            /* 0xDD 0x38 */ },
    { add_ix_sp,    DASM("ADD IX,SP")   /* 0xDD 0x39 */ },
    { NULL,         DASM("")            /* 0xDD 0x3A */ },
    { NULL,         DASM("")            /* 0xDD 0x3B */ },
    { NULL,         DASM("")            /* 0xDD 0x3C */ },
    { NULL,         DASM("")            /* 0xDD 0x3D */ },
    { NULL,         DASM("")            /* 0xDD 0x3E */ },
    { NULL,         DASM("")            /* 0xDD 0x3F */ },
    { NULL,         DASM("")            /* 0xDD 0x40 */ },
    { NULL,         DASM("")            /* 0xDD 0x41 */ },
    { NULL,         DASM("")            /* 0xDD 0x42 */ },
    { NULL,         DASM("")            /* 0xDD 0x43 */ },
    { ld_b_ixh,     DASM("LD B,IXh")    /* 0xDD 0x44 */ },
    { ld_b_ixl,     DASM("LD B,IXl")    /* 0xDD 0x45 */ },
    { ld_b_mixd,    DASM("LD B,(IX+#d)")/* 0xDD 0x46 */ },
    { NULL,         DASM("")            /* 0xDD 0x47 */ },
    { NULL,         DASM("")            /* 0xDD 0x48 */ },
    { NULL,         DASM("")            /* 0xDD 0x49 */ },
    { NULL,         DASM("")            /* 0xDD 0x4A */ },
    { NULL,         DASM("")            /* 0xDD 0x4B */ },
    { ld_c_ixh,     DASM("LD C,IXh")    /* 0xDD 0x4C */ },
    { ld_c_ixl,     DASM("LD C,IXl")    /* 0xDD 0x4D */ },
    { ld_c_mixd,    DASM("LD C,(IX+#d)")/* 0xDD 0x4E */ },
    { NULL,         DASM("")            /* 0xDD 0x4F */ },
    { NULL,         DASM("")            /* 0xDD 0x50 */ },
    { NULL,         DASM("")            /* 0xDD 0x51 */ },
    { NULL,         DASM("")            /* 0xDD 0x52 */ },
    { NULL,         DASM("")            /* 0xDD 0x53 */ },
    { ld_d_ixh,     DASM("LD D,IXh")    /* 0xDD 0x54 */ },
    { ld_d_ixl,     DASM("LD D,IXl")    /* 0xDD 0x55 */ },
    { ld_d_mixd,    DASM("LD D,(IX+#d)")/* 0xDD 0x56 */ },
    { NULL,         DASM("")            /* 0xDD 0x57 */ },
    { NULL,         DASM("")            /* 0xDD 0x58 */ },
    { NULL,         DASM("")            /* 0xDD 0x59 */ },
    { NULL,         DASM("")            /* 0xDD 0x5A */ },
    { NULL,         DASM("")            /* 0xDD 0x5B */ },
    { ld_e_ixh,     DASM("LD E,IXh")    /* 0xDD 0x5C */ },
    { ld_e_ixl,     DASM("LD E,IXl")    /* 0xDD 0x5D */ },
    { ld_e_mixd,    DASM("LD E,(IX+#d)")/* 0xDD 0x5E */ },
    { NULL,         DASM("")            /* 0xDD 0x5F */ },
    { ld_ixh_b,     DASM("LD IXh,B")    /* 0xDD 0x60 */ },
    { ld_ixh_c,     DASM("LD IXh,C")    /* 0xDD 0x61 */ },
    { ld_ixh_d,     DASM("LD IXh,D")    /* 0xDD 0x62 */ },
    { ld_ixh_e,     DASM("LD IXh,E")    /* 0xDD 0x63 */ },
    { ld_ixh_ixh,   DASM("LD IXh,IXh")  /* 0xDD 0x64 */ },
    { ld_ixh_ixl,   DASM("LD IXh,IXl")  /* 0xDD 0x65 */ },
    { ld_h_mixd,    DASM("LD H,(IX+#d)")/* 0xDD 0x66 */ },
    { ld_ixh_a,     DASM("LD IXh,A")    /* 0xDD 0x67 */ },
    { ld_ixl_b,     DASM("LD IXl,B")    /* 0xDD 0x68 */ },
    { ld_ixl_c,     DASM("LD IXl,C")    /* 0xDD 0x69 */ },
    { ld_ixl_d,     DASM("LD IXl,D")    /* 0xDD 0x6A */ },
    { ld_ixl_e,     DASM("LD IXl,E")    /* 0xDD 0x6B */ },
    { ld_ixl_ixh,   DASM("LD IXl,IXh")  /* 0xDD 0x6C */ },
    { ld_ixl_ixl,   DASM("LD IXl,IXl")  /* 0xDD 0x6D */ },
    { ld_l_mixd,    DASM("LD L,(IX+#d)")/* 0xDD 0x6E */ },
    { ld_ixl_a,     DASM("LD IXl,A")    /* 0xDD 0x6F */ },
    { ld_mixd_b,    DASM("LD (IX+#d),B")/* 0xDD 0x70 */ },
    { ld_mixd_c,    DASM("LD (IX+#d),C")/* 0xDD 0x71 */ },
    { ld_mixd_d,    DASM("LD (IX+#d),D")/* 0xDD 0x72 */ },
    { ld_mixd_e,    DASM("LD (IX+#d),E")/* 0xDD 0x73 */ },
    { ld_mixd_h,    DASM("LD (IX+#d),H")/* 0xDD 0x74 */ },
    { ld_mixd_l,    DASM("LD (IX+#d),L")/* 0xDD 0x75 */ },
    { NULL,         DASM("")            /* 0xDD 0x76 */ },
    { ld_mixd_a,    DASM("LD (IX+#d),A")/* 0xDD 0x77 */ },
    { NULL,         DASM("")            /* 0xDD 0x78 */ },
    { NULL,         DASM("")            /* 0xDD 0x79 */ },
    { NULL,         DASM("")            /* 0xDD 0x7A */ },
    { NULL,         DASM("")            /* 0xDD 0x7B */ },
    { ld_a_ixh,     DASM("LD A,IXh")    /* 0xDD 0x7C */ },
    { ld_a_ixl,     DASM("LD A,IXl")    /* 0xDD 0x7D */ },
    { ld_a_mixd,    DASM("LD A,(IX+#d)")/* 0xDD 0x7E */ },
    { NULL,         DASM("")            /* 0xDD 0x7F */ },
    { NULL,         DASM("")            /* 0xDD 0x80 */ },
    { NULL,         DASM("")            /* 0xDD 0x81 */ },
    { NULL,         DASM("")            /* 0xDD 0x82 */ },
    { NULL,         DASM("")            /* 0xDD 0x83 */ },
    { add_a_ixh,    DASM("ADD A,IXh")   /* 0xDD 0x84 */ },
    { add_a_ixl,    DASM("ADD A,IXl")   /* 0xDD 0x85 */ },
    { add_a_mixd,   DASM("ADD A,(IX+#d)")/* 0xDD 0x86 */ },
    { NULL,         DASM("")            /* 0xDD 0x87 */ },
    { NULL,         DASM("")            /* 0xDD 0x88 */ },
    { NULL,         DASM("")            /* 0xDD 0x89 */ },
    { NULL,         DASM("")            /* 0xDD 0x8A */ },
    { NULL,         DASM("")            /* 0xDD 0x8B */ },
    { adc_a_ixh,    DASM("ADC A,IXh")   /* 0xDD 0x8C */ },
    { adc_a_ixl,    DASM("ADC A,IXl")   /* 0xDD 0x8D */ },
    { adc_a_mixd,   DASM("ADC A,(IX+#d)")/* 0xDD 0x8E */ },
    { NULL,         DASM("")            /* 0xDD 0x8F */ },
    { NULL,         DASM("")            /* 0xDD 0x90 */ },
    { NULL,         DASM("")            /* 0xDD 0x91 */ },
    { NULL,         DASM("")            /* 0xDD 0x92 */ },
    { NULL,         DASM("")            /* 0xDD 0x93 */ },
    { sub_ixh,      DASM("SUB IXh")     /* 0xDD 0x94 */ },
    { sub_ixl,      DASM("SUB IXl")     /* 0xDD 0x95 */ },
    { sub_mixd,     DASM("SUB (IX+#d)") /* 0xDD 0x96 */ },
    { NULL,         DASM("")            /* 0xDD 0x97 */ },
    { NULL,         DASM("")            /* 0xDD 0x98 */ },
    { NULL,         DASM("")            /* 0xDD 0x99 */ },
    { NULL,         DASM("")            /* 0xDD 0x9A */ },
    { NULL,         DASM("")            /* 0xDD 0x9B */ },
    { sbc_a_ixh,    DASM("SBC A,IXh")   /* 0xDD 0x9C */ },
    { sbc_a_ixl,    DASM("SBC A,IXl")   /* 0xDD 0x9D */ },
    { sbc_a_mixd,   DASM("SBC A,(IX+#d)")/* 0xDD 0x9E */ },
    { NULL,         DASM("")            /* 0xDD 0x9F */ },
    { NULL,         DASM("")            /* 0xDD 0xA0 */ },
    { NULL,         DASM("")            /* 0xDD 0xA1 */ },
    { NULL,         DASM("")            /* 0xDD 0xA2 */ },
    { NULL,         DASM("")            /* 0xDD 0xA3 */ },
    { and_ixh,      DASM("AND IXh")     /* 0xDD 0xA4 */ },
    { and_ixl,      DASM("AND IXl")     /* 0xDD 0xA5 */ },
    { and_mixd,     DASM("AND (IX+#d)") /* 0xDD 0xA6 */ },
    { NULL,         DASM("")            /* 0xDD 0xA7 */ },
    { NULL,         DASM("")            /* 0xDD 0xA8 */ },
    { NULL,         DASM("")            /* 0xDD 0xA9 */ },
    { NULL,         DASM("")            /* 0xDD 0xAA */ },
    { NULL,         DASM("")            /* 0xDD 0xAB */ },
    { xor_ixh,      DASM("XOR IXh")     /* 0xDD 0xAC */ },
    { xor_ixl,      DASM("XOR IXl")     /* 0xDD 0xAD */ },
    { xor_mixd,     DASM("XOR (IX+#d)") /* 0xDD 0xAE */ },
    { NULL,         DASM("")            /* 0xDD 0xAF */ },
    { NULL,         DASM("")            /* 0xDD 0xB0 */ },
    { NULL,         DASM("")            /* 0xDD 0xB1 */ },
    { NULL,         DASM("")            /* 0xDD 0xB2 */ },
    { NULL,         DASM("")            /* 0xDD 0xB3 */ },
    { or_ixh,       DASM("OR IXh")      /* 0xDD 0xB4 */ },
    { or_ixl,       DASM("OR IXl")      /* 0xDD 0xB5 */ },
    { or_mixd,      DASM("OR (IX+#d)")  /* 0xDD 0xB6 */ },
    { NULL,         DASM("")            /* 0xDD 0xB7 */ },
    { NULL,         DASM("")            /* 0xDD 0xB8 */ },
    { NULL,         DASM("")            /* 0xDD 0xB9 */ },
    { NULL,         DASM("")            /* 0xDD 0xBA */ },
    { NULL,         DASM("")            /* 0xDD 0xBB */ },
    { cp_ixh,       DASM("CP IXh")      /* 0xDD 0xBC */ },
    { cp_ixl,       DASM("CP IXl")      /* 0xDD 0xBD */ },
    { cp_mixd,      DASM("CP (IX+#d)")  /* 0xDD 0xBE */ },
    { NULL,         DASM("")            /* 0xDD 0xBF */ },
    { NULL,         DASM("")            /* 0xDD 0xC0 */ },
    { NULL,         DASM("")            /* 0xDD 0xC1 */ },
    { NULL,         DASM("")            /* 0xDD 0xC2 */ },
    { NULL,         DASM("")            /* 0xDD 0xC3 */ },
    { NULL,         DASM("")            /* 0xDD 0xC4 */ },
    { NULL,         DASM("")            /* 0xDD 0xC5 */ },
    { NULL,         DASM("")            /* 0xDD 0xC6 */ },
    { NULL,         DASM("")            /* 0xDD 0xC7 */ },
    { NULL,         DASM("")            /* 0xDD 0xC8 */ },
    { NULL,         DASM("")            /* 0xDD 0xC9 */ },
    { NULL,         DASM("")            /* 0xDD 0xCA */ },
    { opcode_ddcb,  NULL                /* 0xDD 0xCB */ },
    { NULL,         DASM("")            /* 0xDD 0xCC */ },
    { NULL,         DASM("")            /* 0xDD 0xCD */ },
    { NULL,         DASM("")            /* 0xDD 0xCE */ },
    { NULL,         DASM("")            /* 0xDD 0xCF */ },
    { NULL,         DASM("")            /* 0xDD 0xD0 */ },
    { NULL,         DASM("")            /* 0xDD 0xD1 */ },
    { NULL,         DASM("")            /* 0xDD 0xD2 */ },
    { NULL,         DASM("")            /* 0xDD 0xD3 */ },
    { NULL,         DASM("")            /* 0xDD 0xD4 */ },
    { NULL,         DASM("")            /* 0xDD 0xD5 */ },
    { NULL,         DASM("")            /* 0xDD 0xD6 */ },
    { NULL,         DASM("")            /* 0xDD 0xD7 */ },
    { NULL,         DASM("")            /* 0xDD 0xD8 */ },
    { NULL,         DASM("")            /* 0xDD 0xD9 */ },
    { NULL,         DASM("")            /* 0xDD 0xDA */ },
    { NULL,         DASM("")            /* 0xDD 0xDB */ },
    { NULL,         DASM("")            /* 0xDD 0xDC */ },
    { NULL,         DASM("")            /* 0xDD 0xDD */ },
    { NULL,         DASM("")            /* 0xDD 0xDE */ },
    { NULL,         DASM("")            /* 0xDD 0xDF */ },
    { NULL,         DASM("")            /* 0xDD 0xE0 */ },
    { pop_ix,       DASM("POP IX")      /* 0xDD 0xE1 */ },
    { NULL,         DASM("")            /* 0xDD 0xE2 */ },
    { ex_msp_ix,    DASM("EX (SP),IX")  /* 0xDD 0xE3 */ },
    { NULL,         DASM("")            /* 0xDD 0xE4 */ },
    { push_ix,      DASM("PUSH IX")     /* 0xDD 0xE5 */ },
    { NULL,         DASM("")            /* 0xDD 0xE6 */ },
    { NULL,         DASM("")            /* 0xDD 0xE7 */ },
    { NULL,         DASM("")            /* 0xDD 0xE8 */ },
    { jp_ix,        DASM("JP IX")       /* 0xDD 0xE9 */ },
    { NULL,         DASM("")            /* 0xDD 0xEA */ },
    { NULL,         DASM("")            /* 0xDD 0xEB */ },
    { NULL,         DASM("")            /* 0xDD 0xEC */ },
    { NULL,         DASM("")            /* 0xDD 0xED */ },
    { NULL,         DASM("")            /* 0xDD 0xEE */ },
    { NULL,         DASM("")            /* 0xDD 0xEF */ },
    { NULL,         DASM("")            /* 0xDD 0xF0 */ },
    { NULL,         DASM("")            /* 0xDD 0xF1 */ },
    { NULL,         DASM("")            /* 0xDD 0xF2 */ },
    { NULL,         DASM("")            /* 0xDD 0xF3 */ },
    { NULL,         DASM("")            /* 0xDD 0xF4 */ },
    { NULL,         DASM("")            /* 0xDD 0xF5 */ },
    { NULL,         DASM("")            /* 0xDD 0xF6 */ },
    { NULL,         DASM("")            /* 0xDD 0xF7 */ },
    { NULL,         DASM("")            /* 0xDD 0xF8 */ },
    { ld_sp_ix,     DASM("LD SP,IX")    /* 0xDD 0xF9 */ },
    { NULL,         DASM("")            /* 0xDD 0xFA */ },
    { NULL,         DASM("")            /* 0xDD 0xFB */ },
    { NULL,         DASM("")            /* 0xDD 0xFC */ },
    { NULL,         DASM("")            /* 0xDD 0xFD */ },
    { NULL,         DASM("")            /* 0xDD 0xFE */ },
    { NULL,         DASM("")            /* 0xDD 0xFF */ }
};

void opcode_dd(cpuZ80 *cpu)
{
    word pc = cpu->PC-1;

    int op = (int)read8(cpu, cpu->PC++);

    if(opcodes_dd[op].func==NULL) {
        log4me_error(LOG_EMU_Z80, "0x%04X : opcode = 0xDD 0x%02X not implemented\n", pc, op);
        exit(EXIT_FAILURE);
    }

    cpu->R++;
    opcodes_dd[op].func(cpu);
}

#ifdef DEBUG
void dasmopcode_ddcb(cpuZ80 *cpu, word addr, char *buffer, int len);

void dasmopcode_dd(cpuZ80 *cpu, word addr, char *buffer, int len)
{
    int op = (int)read8(cpu, addr);

    if(opcodes_dd[op].func==opcode_ddcb)
        dasmopcode_ddcb(cpu, addr+1, buffer, len);
    else
        dasmopcode(cpu, opcodes_dd[op].dasm, addr+1, buffer, len);
}
#endif /* DEBUG */
