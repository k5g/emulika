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

static void in_b_c(cpuZ80 *cpu) /* 0xED 0x40 */
{
    cpu->cycles += 12;
    cpu->b.B = in8(cpu, cpu->b.C);
}

static void out_c_b(cpuZ80 *cpu) /* 0xED 0x41 */
{
    cpu->cycles += 12;
    writeio(cpu, cpu->b.C, cpu->b.B);
}

static void sbc_hl_bc(cpuZ80 *cpu) /* 0xED 0x42 */
{
    cpu->cycles += 15;
    cpu->w.HL = sbc16(cpu, cpu->w.HL, cpu->w.BC);
}

static void ld_mnn_bc(cpuZ80 *cpu) /* 0xED 0x43 */
{
    cpu->cycles += 20;
    write16(cpu, read16(cpu, cpu->PC), cpu->w.BC);
    cpu->PC += 2;
}

static void neg(cpuZ80 *cpu) /* 0xED 0x44 */
{
    cpu->cycles += 8;
    cpu->b.F = (cpu->b.A!=0 ? FLAG_CARRY : 0) | FLAG_ADD_SUB | (cpu->b.A==0x80 ? FLAG_OVERFLOW : 0);
    cpu->b.F |= (0 - (cpu->b.A & 0xF)) & 0x10; assert(FLAG_HALF_CARRY==0x10); // Half carry
    cpu->b.A = 0 - cpu->b.A;
    cpu->b.F |= flags_sz(cpu->b.A);
}

static void retn(cpuZ80 *cpu) /* 0xED 0x45 */
{
    cpu->cycles += 14;
    cpu->IFF1 = cpu->IFF2;
    POP(cpu->PC);
}

static void ld_i_a(cpuZ80 *cpu) /* 0xED 0x47 */
{
    cpu->cycles += 9;
    cpu->I = cpu->b.A;
}

static void in_c_c(cpuZ80 *cpu) /* 0xED 0x48 */
{
    cpu->cycles += 12;
    cpu->b.C = in8(cpu, cpu->b.C);
}

static void out_c_c(cpuZ80 *cpu) /* 0xED 0x49 */
{
    cpu->cycles += 12;
    writeio(cpu, cpu->b.C, cpu->b.C);
}

static void adc_hl_bc(cpuZ80 *cpu) /* 0xED 0x4A */
{
    cpu->cycles += 15;
    cpu->w.HL = adc16(cpu, cpu->w.HL, cpu->w.BC);
}

static void ld_bc_mnn(cpuZ80 *cpu) /* 0xED 0x4B */
{
    cpu->cycles += 20;
    cpu->w.BC = read16(cpu, read16(cpu, cpu->PC));
    cpu->PC += 2;
}

static void reti(cpuZ80 *cpu) /* 0xED 0x4D */
{
    cpu->cycles += 14;
    POP(cpu->PC);
    cpu->IFF1 = cpu->IFF2; // Sean Young documentation / Section 5.3
}

static void ld_r_a(cpuZ80 *cpu) /* 0xED 0x4F */
{
    cpu->cycles += 9;
    cpu->R = cpu->b.A;
    cpu->R7 = cpu->b.A & 0x80;
}

static void in_d_c(cpuZ80 *cpu) /* 0xED 0x50 */
{
    cpu->cycles += 12;
    cpu->b.D = in8(cpu, cpu->b.C);
}

static void out_c_d(cpuZ80 *cpu) /* 0xED 0x51 */
{
    cpu->cycles += 12;
    writeio(cpu, cpu->b.C, cpu->b.D);
}

static void sbc_hl_de(cpuZ80 *cpu) /* 0xED 0x52 */
{
    cpu->cycles += 15;
    cpu->w.HL = sbc16(cpu, cpu->w.HL, cpu->w.DE);
}

static void ld_mnn_de(cpuZ80 *cpu) /* 0xED 0x53 */
{
    cpu->cycles += 20;
    write16(cpu, read16(cpu, cpu->PC), cpu->w.DE);
    cpu->PC += 2;
}

static void im1(cpuZ80 *cpu) /* 0xED 0x56 */
{
    cpu->cycles += 8;
    cpu->IM = 1;
}

static void ld_a_i(cpuZ80 *cpu) /* 0xED 0x57 */
{
    cpu->cycles += 9;
    cpu->b.A = cpu->I;
    cpu->b.F &= FLAG_CARRY;
    cpu->b.F |= flags_sz(cpu->I);
    cpu->b.F |= (cpu->IFF2 ? FLAG_PARITY : 0);
}

static void in_e_c(cpuZ80 *cpu) /* 0xED 0x58 */
{
    cpu->cycles += 12;
    cpu->b.E = in8(cpu, cpu->b.C);
}

static void out_c_e(cpuZ80 *cpu) /* 0xED 0x59 */
{
    cpu->cycles += 12;
    writeio(cpu, cpu->b.C, cpu->b.E);
}

static void adc_hl_de(cpuZ80 *cpu) /* 0xED 0x5A */
{
    cpu->cycles += 15;
    cpu->w.HL = adc16(cpu, cpu->w.HL, cpu->w.DE);
}

static void ld_de_mnn(cpuZ80 *cpu) /* 0xED 0x5B */
{
    cpu->cycles += 20;
    cpu->w.DE = read16(cpu, read16(cpu, cpu->PC));
    cpu->PC += 2;
}

static void ld_a_r(cpuZ80 *cpu) /* 0xED 0x5F */
{
    cpu->cycles += 9;
    cpu->b.A = (cpu->R & 0x7F) | (cpu->R7 & 0x80);
    cpu->b.F &= FLAG_CARRY;
    cpu->b.F |= flags_sz(cpu->b.A);
    cpu->b.F |= (cpu->IFF2 ? FLAG_OVERFLOW : 0);
}

static void in_h_c(cpuZ80 *cpu) /* 0xED 0x60 */
{
    cpu->cycles += 12;
    cpu->b.H = in8(cpu, cpu->b.C);
}

static void out_c_h(cpuZ80 *cpu) /* 0xED 0x61 */
{
    cpu->cycles += 12;
    writeio(cpu, cpu->b.C, cpu->b.H);
}

static void sbc_hl_hl(cpuZ80 *cpu) /* 0xED 0x62 */
{
    cpu->cycles += 15;
    cpu->w.HL = sbc16(cpu, cpu->w.HL, cpu->w.HL);
}

static void ld_mnn_hl(cpuZ80 *cpu) /* 0xED 0x63 */
{
    cpu->cycles += 20;
    write16(cpu, read16(cpu, cpu->PC), cpu->w.HL);
    cpu->PC += 2;
}

static void rrd(cpuZ80 *cpu) /* 0xED 0x67 */
{
    cpu->cycles += 18;
    cpu->b.F &= FLAG_CARRY;
    byte value = read8(cpu, cpu->w.HL);
    byte acc = cpu->b.A;
    cpu->b.A = (acc & 0xF0) | (value & 0x0F);
    write8(cpu, cpu->w.HL, ((acc&0x0F) << 4) | (value >> 4));
    cpu->b.F |= flags_szp(cpu->b.A);
}

static void in_l_c(cpuZ80 *cpu) /* 0xED 0x68 */
{
    cpu->cycles += 12;
    cpu->b.L = in8(cpu, cpu->b.C);
}

static void out_c_l(cpuZ80 *cpu) /* 0xED 0x69 */
{
    cpu->cycles += 12;
    writeio(cpu, cpu->b.C, cpu->b.L);
}

static void adc_hl_hl(cpuZ80 *cpu) /* 0xED 0x6A */
{
    cpu->cycles += 15;
    cpu->w.HL = adc16(cpu, cpu->w.HL, cpu->w.HL);
}

static void ld_hl_mnn(cpuZ80 *cpu) /* 0xED 0x6B */
{
    cpu->cycles += 20;
    cpu->w.HL = read16(cpu, read16(cpu, cpu->PC));
    cpu->PC += 2;
}

static void rld(cpuZ80 *cpu) /* 0xED 0x6F */
{
    cpu->cycles += 18;
    cpu->b.F &= FLAG_CARRY;
    byte value = read8(cpu, cpu->w.HL);
    byte acc = cpu->b.A;
    cpu->b.A = (value >> 4) | (acc & 0xF0);
    write8(cpu, cpu->w.HL, (value<<4) | (acc&0x0F));
    cpu->b.F |= flags_szp(cpu->b.A);
}

static void out_c_0(cpuZ80 *cpu) /* 0xED 0x71 */
{
    // Undocumented instruction
    // See section 8.11 Input and Output Group
    cpu->cycles += 12;
    writeio(cpu, cpu->b.C, 0);
}

static void sbc_hl_sp(cpuZ80 *cpu) /* 0xED 0x72 */
{
    cpu->cycles += 15;
    cpu->w.HL = sbc16(cpu, cpu->w.HL, cpu->SP);
}

static void ld_mnn_sp(cpuZ80 *cpu) /* 0xED 0x73 */
{
    cpu->cycles += 20;
    write16(cpu, read16(cpu, cpu->PC), cpu->SP);
    cpu->PC += 2;
}

static void in_a_c(cpuZ80 *cpu) /* 0xED 0x78 */
{
    cpu->cycles += 12;
    cpu->b.A = in8(cpu, cpu->b.C);
}

static void out_c_a(cpuZ80 *cpu) /* 0xED 0x79 */
{
    cpu->cycles += 12;
    writeio(cpu, cpu->b.C, cpu->b.A);
}

static void adc_hl_sp(cpuZ80 *cpu) /* 0xED 0x7A */
{
    cpu->cycles += 15;
    cpu->w.HL = adc16(cpu, cpu->w.HL, cpu->SP);
}

static void ld_sp_mnn(cpuZ80 *cpu) /* 0xED 0x7B */
{
    cpu->cycles += 20;
    cpu->SP = read16(cpu, read16(cpu, cpu->PC));
    cpu->PC += 2;
}

static void ldi(cpuZ80 *cpu) /* 0xED 0xA0 */
{
    cpu->cycles += 16;
    write8(cpu, cpu->w.DE++, read8(cpu, cpu->w.HL++));
    cpu->b.F &= ~(FLAG_HALF_CARRY | FLAG_OVERFLOW | FLAG_ADD_SUB);
    cpu->w.BC--;
    cpu->b.F |= (cpu->w.BC!=0 ? FLAG_OVERFLOW : 0);
}

static void cpi(cpuZ80 *cpu) /* 0xED 0xA1 */
{
    byte carry = cpu->b.F & FLAG_CARRY;
    cpu->cycles += 16;
    sub8(cpu, cpu->b.A, read8(cpu, cpu->w.HL++));
    cpu->w.BC--;
    cpu->b.F &= ~(FLAG_OVERFLOW | FLAG_CARRY);
    cpu->b.F |= (cpu->w.BC!=0 ? FLAG_OVERFLOW : 0);
    cpu->b.F |= carry;
}

static void ini(cpuZ80 *cpu) /* 0xED 0xA2 */
{
    cpu->cycles += 16;
    register byte v = readio(cpu, cpu->b.C);
    write8(cpu, cpu->w.HL++, v);
    cpu->b.B--;
    cpu->b.F = flags_sz(cpu->b.B);
    cpu->b.F |= v&0x80 ? FLAG_ADD_SUB : 0; // NF flag A copy of bit 7 of the value read from or written to an I/O port.
    word k = v + ((cpu->b.C+1) & 255);
    if(k>255) cpu->b.F |= (FLAG_HALF_CARRY | FLAG_CARRY);
    cpu->b.F |= flags_p((k & 7)^cpu->b.B);
}

static void outi(cpuZ80 *cpu) /* 0xED 0xA3 */
{
    cpu->cycles += 16;
    register byte v = read8(cpu, cpu->w.HL++);
    writeio(cpu, cpu->b.C, v);
    cpu->b.B--;
    cpu->b.F = flags_sz(cpu->b.B);
    cpu->b.F |= v&0x80 ? FLAG_ADD_SUB : 0; // NF flag A copy of bit 7 of the value read from or written to an I/O port.
    word k = v + cpu->b.L;
    if(k>255) cpu->b.F |= (FLAG_HALF_CARRY | FLAG_CARRY);
    cpu->b.F |= flags_p((k & 7)^cpu->b.B);
}

static void ldd(cpuZ80 *cpu) /* 0xED 0xA8 */
{
    write8(cpu, cpu->w.DE--, read8(cpu, cpu->w.HL--));
    cpu->w.BC--;
    cpu->cycles += 16;
    cpu->b.F &= (FLAG_SIGN | FLAG_ZERO | FLAG_CARRY);
    cpu->b.F |= (cpu->w.BC!=0 ? FLAG_OVERFLOW : 0);
}

static void cpd(cpuZ80 *cpu) /* 0xED 0xA9 */
{
    byte carry = cpu->b.F & FLAG_CARRY;
    cpu->cycles += 16;
    sub8(cpu, cpu->b.A, read8(cpu, cpu->w.HL--));
    cpu->w.BC--;
    cpu->b.F &= ~(FLAG_OVERFLOW | FLAG_CARRY);
    cpu->b.F |= (cpu->w.BC!=0 ? FLAG_OVERFLOW : 0);
    cpu->b.F |= carry;
}

static void ind(cpuZ80 *cpu) /* 0xED 0xAA */
{
    cpu->cycles += 16;
    register byte v = readio(cpu, cpu->b.C);
    write8(cpu, cpu->w.HL--, v);
    cpu->b.B--;
    cpu->b.F = flags_sz(cpu->b.B);
    cpu->b.F |= v&0x80 ? FLAG_ADD_SUB : 0; // NF flag A copy of bit 7 of the value read from or written to an I/O port.
    word k = v + ((cpu->b.C+1) & 255);
    if(k>255) cpu->b.F |= (FLAG_HALF_CARRY | FLAG_CARRY);
    cpu->b.F |= flags_p((k & 7)^cpu->b.B);
}

static void outd(cpuZ80 *cpu) /* 0xED 0xAB */
{
    cpu->cycles += 16;
    register byte v = read8(cpu, cpu->w.HL--);
    writeio(cpu, cpu->b.C, v);
    cpu->b.B--;
    cpu->b.F = flags_sz(cpu->b.B);
    cpu->b.F |= v&0x80 ? FLAG_ADD_SUB : 0; // NF flag A copy of bit 7 of the value read from or written to an I/O port.
    word k = v + cpu->b.L;
    if(k>255) cpu->b.F |= (FLAG_HALF_CARRY | FLAG_CARRY);
    cpu->b.F |= flags_p((k & 7)^cpu->b.B);
}

static void ldir(cpuZ80 *cpu) /* 0xED 0xB0 */
{
    ldi(cpu);
    if(cpu->w.BC!=0) {
        cpu->cycles += 5;
        cpu->PC -= 2;
    }
}

static void cpir(cpuZ80 *cpu) /* 0xED 0xB1 */
{
    cpi(cpu);
    if((cpu->w.BC!=0) && !(cpu->b.F&FLAG_ZERO)) {
        cpu->cycles += 5;
        cpu->PC -= 2;
    }
}

static void inir(cpuZ80 *cpu) /* 0xED 0xB2 */
{
    ini(cpu);
    if(cpu->b.B!=0) {
        cpu->cycles += 5;
        cpu->PC -= 2;
    }
}

static void otir(cpuZ80 *cpu) /* 0xED 0xB3 */
{
    outi(cpu);
    if(cpu->b.B!=0) {
        cpu->cycles += 5;
        cpu->PC -= 2;
    }
}

static void lddr(cpuZ80 *cpu) /* 0xED 0xB8 */
{
    ldd(cpu);
    if(cpu->w.BC!=0) {
        cpu->cycles += 5;
        cpu->PC -= 2;
    }
}

static void cpdr(cpuZ80 *cpu) /* 0xED 0xB9 */
{
    cpd(cpu);
    if((cpu->w.BC!=0) && !(cpu->b.F&FLAG_ZERO)) {
        cpu->cycles += 5;
        cpu->PC -= 2;
    }
}

static void indr(cpuZ80 *cpu) /* 0xED 0xBA */
{
    ind(cpu);
    if(cpu->b.B!=0) {
        cpu->cycles += 5;
        cpu->PC -= 2;
    }
}

static void otdr(cpuZ80 *cpu) /* 0xED 0xBB */
{
    outd(cpu);
    if(cpu->b.B!=0) {
        cpu->cycles += 5;
        cpu->PC -= 2;
    }
}


opcode opcodes_ed[256] = {
    { NULL,         DASM("")            /* 0xED 0x00 */ },
    { NULL,         DASM("")            /* 0xED 0x01 */ },
    { NULL,         DASM("")            /* 0xED 0x02 */ },
    { NULL,         DASM("")            /* 0xED 0x03 */ },
    { NULL,         DASM("")            /* 0xED 0x04 */ },
    { NULL,         DASM("")            /* 0xED 0x05 */ },
    { NULL,         DASM("")            /* 0xED 0x06 */ },
    { NULL,         DASM("")            /* 0xED 0x07 */ },
    { NULL,         DASM("")            /* 0xED 0x08 */ },
    { NULL,         DASM("")            /* 0xED 0x09 */ },
    { NULL,         DASM("")            /* 0xED 0x0A */ },
    { NULL,         DASM("")            /* 0xED 0x0B */ },
    { NULL,         DASM("")            /* 0xED 0x0C */ },
    { NULL,         DASM("")            /* 0xED 0x0D */ },
    { NULL,         DASM("")            /* 0xED 0x0E */ },
    { NULL,         DASM("")            /* 0xED 0x0F */ },
    { NULL,         DASM("")            /* 0xED 0x10 */ },
    { NULL,         DASM("")            /* 0xED 0x11 */ },
    { NULL,         DASM("")            /* 0xED 0x12 */ },
    { NULL,         DASM("")            /* 0xED 0x13 */ },
    { NULL,         DASM("")            /* 0xED 0x14 */ },
    { NULL,         DASM("")            /* 0xED 0x15 */ },
    { NULL,         DASM("")            /* 0xED 0x16 */ },
    { NULL,         DASM("")            /* 0xED 0x17 */ },
    { NULL,         DASM("")            /* 0xED 0x18 */ },
    { NULL,         DASM("")            /* 0xED 0x19 */ },
    { NULL,         DASM("")            /* 0xED 0x1A */ },
    { NULL,         DASM("")            /* 0xED 0x1B */ },
    { NULL,         DASM("")            /* 0xED 0x1C */ },
    { NULL,         DASM("")            /* 0xED 0x1D */ },
    { NULL,         DASM("")            /* 0xED 0x1E */ },
    { NULL,         DASM("")            /* 0xED 0x1F */ },
    { NULL,         DASM("")            /* 0xED 0x20 */ },
    { NULL,         DASM("")            /* 0xED 0x21 */ },
    { NULL,         DASM("")            /* 0xED 0x22 */ },
    { NULL,         DASM("")            /* 0xED 0x23 */ },
    { NULL,         DASM("")            /* 0xED 0x24 */ },
    { NULL,         DASM("")            /* 0xED 0x25 */ },
    { NULL,         DASM("")            /* 0xED 0x26 */ },
    { NULL,         DASM("")            /* 0xED 0x27 */ },
    { NULL,         DASM("")            /* 0xED 0x28 */ },
    { NULL,         DASM("")            /* 0xED 0x29 */ },
    { NULL,         DASM("")            /* 0xED 0x2A */ },
    { NULL,         DASM("")            /* 0xED 0x2B */ },
    { NULL,         DASM("")            /* 0xED 0x2C */ },
    { NULL,         DASM("")            /* 0xED 0x2D */ },
    { NULL,         DASM("")            /* 0xED 0x2E */ },
    { NULL,         DASM("")            /* 0xED 0x2F */ },
    { NULL,         DASM("")            /* 0xED 0x30 */ },
    { NULL,         DASM("")            /* 0xED 0x31 */ },
    { NULL,         DASM("")            /* 0xED 0x32 */ },
    { NULL,         DASM("")            /* 0xED 0x33 */ },
    { NULL,         DASM("")            /* 0xED 0x34 */ },
    { NULL,         DASM("")            /* 0xED 0x35 */ },
    { NULL,         DASM("")            /* 0xED 0x36 */ },
    { NULL,         DASM("")            /* 0xED 0x37 */ },
    { NULL,         DASM("")            /* 0xED 0x38 */ },
    { NULL,         DASM("")            /* 0xED 0x39 */ },
    { NULL,         DASM("")            /* 0xED 0x3A */ },
    { NULL,         DASM("")            /* 0xED 0x3B */ },
    { NULL,         DASM("")            /* 0xED 0x3C */ },
    { NULL,         DASM("")            /* 0xED 0x3D */ },
    { NULL,         DASM("")            /* 0xED 0x3E */ },
    { NULL,         DASM("")            /* 0xED 0x3F */ },
    { in_b_c,       DASM("IN B,(C)")    /* 0xED 0x40 */ },
    { out_c_b,      DASM("OUT (C),B")   /* 0xED 0x41 */ },
    { sbc_hl_bc,    DASM("SBC HL,BC")   /* 0xED 0x42 */ },
    { ld_mnn_bc,    DASM("LD (#nn),BC") /* 0xED 0x43 */ },
    { neg,          DASM("NEG")         /* 0xED 0x44 */ },
    { retn,         DASM("RETN")        /* 0xED 0x45 */ },
    { NULL,         DASM("")            /* 0xED 0x46 */ },
    { ld_i_a,       DASM("LD I,A")      /* 0xED 0x47 */ },
    { in_c_c,       DASM("IN C,(C)")    /* 0xED 0x48 */ },
    { out_c_c,      DASM("OUT (C),C")   /* 0xED 0x49 */ },
    { adc_hl_bc,    DASM("ADC HL,BC")   /* 0xED 0x4A */ },
    { ld_bc_mnn,    DASM("LD BC,(#nn)") /* 0xED 0x4B */ },
    { NULL,         DASM("")            /* 0xED 0x4C */ },
    { reti,         DASM("RETI")        /* 0xED 0x4D */ },
    { NULL,         DASM("")            /* 0xED 0x4E */ },
    { ld_r_a,       DASM("LD R,A")      /* 0xED 0x4F */ },
    { in_d_c,       DASM("IN D,(C)")    /* 0xED 0x50 */ },
    { out_c_d,      DASM("OUT (C),D")   /* 0xED 0x51 */ },
    { sbc_hl_de,    DASM("SBC HL,DE")   /* 0xED 0x52 */ },
    { ld_mnn_de,    DASM("LD (#nn),DE") /* 0xED 0x53 */ },
    { NULL,         DASM("")            /* 0xED 0x54 */ },
    { NULL,         DASM("")            /* 0xED 0x55 */ },
    { im1,          DASM("IM 1")        /* 0xED 0x56 */ },
    { ld_a_i,       DASM("LD A,I")      /* 0xED 0x57 */ },
    { in_e_c,       DASM("IN E,(C)")    /* 0xED 0x58 */ },
    { out_c_e,      DASM("OUT (C),E")   /* 0xED 0x59 */ },
    { adc_hl_de,    DASM("ADC HL,DE")   /* 0xED 0x5A */ },
    { ld_de_mnn,    DASM("LD DE,(#nn)") /* 0xED 0x5B */ },
    { NULL,         DASM("")            /* 0xED 0x5C */ },
    { NULL,         DASM("")            /* 0xED 0x5D */ },
    { NULL,         DASM("")            /* 0xED 0x5E */ },
    { ld_a_r,       DASM("LD A,R")      /* 0xED 0x5F */ },
    { in_h_c,       DASM("IN H,(C)")    /* 0xED 0x60 */ },
    { out_c_h,      DASM("OUT (C),H")   /* 0xED 0x61 */ },
    { sbc_hl_hl,    DASM("SBC HL,HL")   /* 0xED 0x62 */ },
    { ld_mnn_hl,    DASM("LD (#nn),HL") /* 0xED 0x63 */ },
    { NULL,         DASM("")            /* 0xED 0x64 */ },
    { NULL,         DASM("")            /* 0xED 0x65 */ },
    { NULL,         DASM("")            /* 0xED 0x66 */ },
    { rrd,          DASM("RRD")         /* 0xED 0x67 */ },
    { in_l_c,       DASM("IN L,(C)")    /* 0xED 0x68 */ },
    { out_c_l,      DASM("OUT (C),L")   /* 0xED 0x69 */ },
    { adc_hl_hl,    DASM("ADC HL,HL")   /* 0xED 0x6A */ },
    { ld_hl_mnn,    DASM("LD HL,(#nn)") /* 0xED 0x6B */ },
    { NULL,         DASM("")            /* 0xED 0x6C */ },
    { NULL,         DASM("")            /* 0xED 0x6D */ },
    { NULL,         DASM("")            /* 0xED 0x6E */ },
    { rld,          DASM("RLD")         /* 0xED 0x6F */ },
    { NULL,         DASM("")            /* 0xED 0x70 */ },
    { out_c_0,      DASM("OUT (C),0")   /* 0xED 0x71 */ },
    { sbc_hl_sp,    DASM("SBC HL,SP")   /* 0xED 0x72 */ },
    { ld_mnn_sp,    DASM("LD (#nn),SP") /* 0xED 0x73 */ },
    { NULL,         DASM("")            /* 0xED 0x74 */ },
    { NULL,         DASM("")            /* 0xED 0x75 */ },
    { NULL,         DASM("")            /* 0xED 0x76 */ },
    { NULL,         DASM("")            /* 0xED 0x77 */ },
    { in_a_c,       DASM("IN A,(C)")    /* 0xED 0x78 */ },
    { out_c_a,      DASM("OUT (C),A")   /* 0xED 0x79 */ },
    { adc_hl_sp,    DASM("ADC HL,SP")   /* 0xED 0x7A */ },
    { ld_sp_mnn,    DASM("LD SP,(#nn)") /* 0xED 0x7B */ },
    { NULL,         DASM("")            /* 0xED 0x7C */ },
    { NULL,         DASM("")            /* 0xED 0x7D */ },
    { NULL,         DASM("")            /* 0xED 0x7E */ },
    { NULL,         DASM("")            /* 0xED 0x7F */ },
    { NULL,         DASM("")            /* 0xED 0x80 */ },
    { NULL,         DASM("")            /* 0xED 0x81 */ },
    { NULL,         DASM("")            /* 0xED 0x82 */ },
    { NULL,         DASM("")            /* 0xED 0x83 */ },
    { NULL,         DASM("")            /* 0xED 0x84 */ },
    { NULL,         DASM("")            /* 0xED 0x85 */ },
    { NULL,         DASM("")            /* 0xED 0x86 */ },
    { NULL,         DASM("")            /* 0xED 0x87 */ },
    { NULL,         DASM("")            /* 0xED 0x88 */ },
    { NULL,         DASM("")            /* 0xED 0x89 */ },
    { NULL,         DASM("")            /* 0xED 0x8A */ },
    { NULL,         DASM("")            /* 0xED 0x8B */ },
    { NULL,         DASM("")            /* 0xED 0x8C */ },
    { NULL,         DASM("")            /* 0xED 0x8D */ },
    { NULL,         DASM("")            /* 0xED 0x8E */ },
    { NULL,         DASM("")            /* 0xED 0x8F */ },
    { NULL,         DASM("")            /* 0xED 0x90 */ },
    { NULL,         DASM("")            /* 0xED 0x91 */ },
    { NULL,         DASM("")            /* 0xED 0x92 */ },
    { NULL,         DASM("")            /* 0xED 0x93 */ },
    { NULL,         DASM("")            /* 0xED 0x94 */ },
    { NULL,         DASM("")            /* 0xED 0x95 */ },
    { NULL,         DASM("")            /* 0xED 0x96 */ },
    { NULL,         DASM("")            /* 0xED 0x97 */ },
    { NULL,         DASM("")            /* 0xED 0x98 */ },
    { NULL,         DASM("")            /* 0xED 0x99 */ },
    { NULL,         DASM("")            /* 0xED 0x9A */ },
    { NULL,         DASM("")            /* 0xED 0x9B */ },
    { NULL,         DASM("")            /* 0xED 0x9C */ },
    { NULL,         DASM("")            /* 0xED 0x9D */ },
    { NULL,         DASM("")            /* 0xED 0x9E */ },
    { NULL,         DASM("")            /* 0xED 0x9F */ },
    { ldi,          DASM("LDI")         /* 0xED 0xA0 */ },
    { cpi,          DASM("CPI")         /* 0xED 0xA1 */ },
    { ini,          DASM("INI")         /* 0xED 0xA2 */ },
    { outi,         DASM("OUTI")        /* 0xED 0xA3 */ },
    { NULL,         DASM("")            /* 0xED 0xA4 */ },
    { NULL,         DASM("")            /* 0xED 0xA5 */ },
    { NULL,         DASM("")            /* 0xED 0xA6 */ },
    { NULL,         DASM("")            /* 0xED 0xA7 */ },
    { ldd,          DASM("LDD")         /* 0xED 0xA8 */ },
    { cpd,          DASM("CPD")         /* 0xED 0xA9 */ },
    { ind,          DASM("IND")         /* 0xED 0xAA */ },
    { outd,         DASM("OUTD")        /* 0xED 0xAB */ },
    { NULL,         DASM("")            /* 0xED 0xAC */ },
    { NULL,         DASM("")            /* 0xED 0xAD */ },
    { NULL,         DASM("")            /* 0xED 0xAE */ },
    { NULL,         DASM("")            /* 0xED 0xAF */ },
    { ldir,         DASM("LDIR")        /* 0xED 0xB0 */ },
    { cpir,         DASM("CPIR")        /* 0xED 0xB1 */ },
    { inir,         DASM("INIR")        /* 0xED 0xB2 */ },
    { otir,         DASM("OTIR")        /* 0xED 0xB3 */ },
    { NULL,         DASM("")            /* 0xED 0xB4 */ },
    { NULL,         DASM("")            /* 0xED 0xB5 */ },
    { NULL,         DASM("")            /* 0xED 0xB6 */ },
    { NULL,         DASM("")            /* 0xED 0xB7 */ },
    { lddr,         DASM("LDDR")        /* 0xED 0xB8 */ },
    { cpdr,         DASM("CPDR")        /* 0xED 0xB9 */ },
    { indr,         DASM("INDR")        /* 0xED 0xBA */ },
    { otdr,         DASM("OTDR")        /* 0xED 0xBB */ },
    { NULL,         DASM("")            /* 0xED 0xBC */ },
    { NULL,         DASM("")            /* 0xED 0xBD */ },
    { NULL,         DASM("")            /* 0xED 0xBE */ },
    { NULL,         DASM("")            /* 0xED 0xBF */ },
    { NULL,         DASM("")            /* 0xED 0xC0 */ },
    { NULL,         DASM("")            /* 0xED 0xC1 */ },
    { NULL,         DASM("")            /* 0xED 0xC2 */ },
    { NULL,         DASM("")            /* 0xED 0xC3 */ },
    { NULL,         DASM("")            /* 0xED 0xC4 */ },
    { NULL,         DASM("")            /* 0xED 0xC5 */ },
    { NULL,         DASM("")            /* 0xED 0xC6 */ },
    { NULL,         DASM("")            /* 0xED 0xC7 */ },
    { NULL,         DASM("")            /* 0xED 0xC8 */ },
    { NULL,         DASM("")            /* 0xED 0xC9 */ },
    { NULL,         DASM("")            /* 0xED 0xCA */ },
    { NULL,         DASM("")            /* 0xED 0xCB */ },
    { NULL,         DASM("")            /* 0xED 0xCC */ },
    { NULL,         DASM("")            /* 0xED 0xCD */ },
    { NULL,         DASM("")            /* 0xED 0xCE */ },
    { NULL,         DASM("")            /* 0xED 0xCF */ },
    { NULL,         DASM("")            /* 0xED 0xD0 */ },
    { NULL,         DASM("")            /* 0xED 0xD1 */ },
    { NULL,         DASM("")            /* 0xED 0xD2 */ },
    { NULL,         DASM("")            /* 0xED 0xD3 */ },
    { NULL,         DASM("")            /* 0xED 0xD4 */ },
    { NULL,         DASM("")            /* 0xED 0xD5 */ },
    { NULL,         DASM("")            /* 0xED 0xD6 */ },
    { NULL,         DASM("")            /* 0xED 0xD7 */ },
    { NULL,         DASM("")            /* 0xED 0xD8 */ },
    { NULL,         DASM("")            /* 0xED 0xD9 */ },
    { NULL,         DASM("")            /* 0xED 0xDA */ },
    { NULL,         DASM("")            /* 0xED 0xDB */ },
    { NULL,         DASM("")            /* 0xED 0xDC */ },
    { NULL,         DASM("")            /* 0xED 0xDD */ },
    { NULL,         DASM("")            /* 0xED 0xDE */ },
    { NULL,         DASM("")            /* 0xED 0xDF */ },
    { NULL,         DASM("")            /* 0xED 0xE0 */ },
    { NULL,         DASM("")            /* 0xED 0xE1 */ },
    { NULL,         DASM("")            /* 0xED 0xE2 */ },
    { NULL,         DASM("")            /* 0xED 0xE3 */ },
    { NULL,         DASM("")            /* 0xED 0xE4 */ },
    { NULL,         DASM("")            /* 0xED 0xE5 */ },
    { NULL,         DASM("")            /* 0xED 0xE6 */ },
    { NULL,         DASM("")            /* 0xED 0xE7 */ },
    { NULL,         DASM("")            /* 0xED 0xE8 */ },
    { NULL,         DASM("")            /* 0xED 0xE9 */ },
    { NULL,         DASM("")            /* 0xED 0xEA */ },
    { NULL,         DASM("")            /* 0xED 0xEB */ },
    { NULL,         DASM("")            /* 0xED 0xEC */ },
    { NULL,         DASM("")            /* 0xED 0xED */ },
    { NULL,         DASM("")            /* 0xED 0xEE */ },
    { NULL,         DASM("")            /* 0xED 0xEF */ },
    { NULL,         DASM("")            /* 0xED 0xF0 */ },
    { NULL,         DASM("")            /* 0xED 0xF1 */ },
    { NULL,         DASM("")            /* 0xED 0xF2 */ },
    { NULL,         DASM("")            /* 0xED 0xF3 */ },
    { NULL,         DASM("")            /* 0xED 0xF4 */ },
    { NULL,         DASM("")            /* 0xED 0xF5 */ },
    { NULL,         DASM("")            /* 0xED 0xF6 */ },
    { NULL,         DASM("")            /* 0xED 0xF7 */ },
    { NULL,         DASM("")            /* 0xED 0xF8 */ },
    { NULL,         DASM("")            /* 0xED 0xF9 */ },
    { NULL,         DASM("")            /* 0xED 0xFA */ },
    { NULL,         DASM("")            /* 0xED 0xFB */ },
    { NULL,         DASM("")            /* 0xED 0xFC */ },
    { NULL,         DASM("")            /* 0xED 0xFD */ },
    { NULL,         DASM("")            /* 0xED 0xFE */ },
    { NULL,         DASM("")            /* 0xED 0xFF */ }
};


void opcode_ed(cpuZ80 *cpu)
{
    word pc = cpu->PC-1;

    int op = (int)read8(cpu, cpu->PC++);

    if(opcodes_ed[op].func==NULL) {
        log4me_error(LOG_EMU_Z80, "0x%04X : opcode = 0xED 0x%02X not implemented\n", pc, op);
        exit(EXIT_FAILURE);
    }

    cpu->R++;
    opcodes_ed[op].func(cpu);
}

#ifdef DEBUG
void dasmopcode_ed(cpuZ80 *cpu, word addr, char *buffer, int len)
{
    int op = (int)read8(cpu, addr);
    dasmopcode(cpu, opcodes_ed[op].dasm, addr+1, buffer, len);
}
#endif
