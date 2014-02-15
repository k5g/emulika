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

/*
Z80 implementation based on :
    - Zylog Z80 Family CPU User Manual (UM008005-0205)
    - Sean Young - The Undocumented Z80 Documented
    - http://www.worldofspectrum.org/faq/reference/z80reference.htm
*/

#include <stdlib.h>

#include "z80.h"
#include "z80_common.h"

void opcode_ed(cpuZ80 *cpu);
void opcode_cb(cpuZ80 *cpu);
void opcode_dd(cpuZ80 *cpu);
void opcode_fd(cpuZ80 *cpu);

byte read8(cpuZ80 *cpu, word addr)
{
    return cpu->readmem(cpu->param, addr);
}

word read16(cpuZ80 *cpu, word addr)
{
    return (word)cpu->readmem(cpu->param, addr) | ((word)cpu->readmem(cpu->param, addr+1) << 8);
    //return (word)read8(cpu, addr) | ((word)read8(cpu, addr+1) << 8);
}

void write8(cpuZ80 *cpu, word addr, byte value)
{
    cpu->writemem(cpu->param, addr, value);
}

void write16(cpuZ80 *cpu, word addr, word value)
{
    write8(cpu, addr, value & 0xFF);
    write8(cpu, addr+1, value >> 8);
}

byte readio(cpuZ80 *cpu, byte port)
{
    byte value = cpu->readio(cpu->param, port);
    return value;
}

void writeio(cpuZ80 *cpu, byte port, byte value)
{
    cpu->writeio(cpu->param, port, value);
}

//--------------------------------------------------------------------------------------------------

static void nop(cpuZ80 *cpu) /* 0x00 */
{
    cpu->cycles += 4;
}

static void ld_bc_nn(cpuZ80 *cpu) /* 0x01 */
{
    cpu->cycles += 10;
    cpu->w.BC = read16(cpu, cpu->PC);
    cpu->PC += 2;
}

static void ld_mbc_a(cpuZ80 *cpu) /* 0x02 */
{
    cpu->cycles += 7;
    write8(cpu, cpu->w.BC, cpu->b.A);
}

static void inc_bc(cpuZ80 *cpu) /* 0x03 */
{
    cpu->cycles += 6;
    cpu->w.BC++;
}

static void inc_b(cpuZ80 *cpu) /* 0x04 */
{
    cpu->cycles += 4;
    cpu->b.B = inc8(cpu, cpu->b.B);
}

static void dec_b(cpuZ80 *cpu) /* 0x05 */
{
    cpu->cycles += 4;
    cpu->b.B = dec8(cpu, cpu->b.B);
}

static void ld_b_n(cpuZ80 *cpu) /* 0x06 */
{
    cpu->cycles += 7;
    cpu->b.B = read8(cpu, cpu->PC++);
}

static void rlca(cpuZ80 *cpu) /* 0x07 */
{
    cpu->cycles += 4;
    cpu->b.F &= ~(FLAG_HALF_CARRY | FLAG_ADD_SUB | FLAG_CARRY);
    cpu->b.F |= (cpu->b.A&0x80 ? FLAG_CARRY : 0);
    cpu->b.A <<= 1;
    cpu->b.A |= cpu->b.F&FLAG_CARRY ? 0x01 : 0;
}

static void ex_af_afp(cpuZ80 *cpu) /* 0x08 */
{
    cpu->cycles += 4;
    word tmp = cpu->w.AF;
    cpu->w.AF = cpu->wp.AFp;
    cpu->wp.AFp = tmp;
}

static void add_hl_bc(cpuZ80 *cpu) /* 0x09 */
{
    cpu->cycles += 11;
    cpu->w.HL = add16(cpu, cpu->w.HL, cpu->w.BC);
}

static void ld_a_mbc(cpuZ80 *cpu) /* 0x0A */
{
    cpu->cycles += 7;
    cpu->b.A = read8(cpu, cpu->w.BC);
}

static void dec_bc(cpuZ80 *cpu) /* 0x0B */
{
    cpu->cycles += 6;
    cpu->w.BC--;
}

static void inc_c(cpuZ80 *cpu) /* 0x0C */
{
    cpu->cycles += 4;
    cpu->b.C = inc8(cpu, cpu->b.C);
}

static void dec_c(cpuZ80 *cpu) /* 0x0D */
{
    cpu->cycles += 4;
    cpu->b.C = dec8(cpu, cpu->b.C);
}

static void ld_c_n(cpuZ80 *cpu) /* 0x0E */
{
    cpu->cycles += 7;
    cpu->b.C = read8(cpu, cpu->PC++);
}

static void rrca(cpuZ80 *cpu) /* 0x0F */
{
    cpu->cycles += 4;
    cpu->b.F &= ~(FLAG_HALF_CARRY | FLAG_ADD_SUB | FLAG_CARRY);
    cpu->b.F |= cpu->b.A&1 ? FLAG_CARRY : 0;
    cpu->b.A >>= 1;
    cpu->b.A |= cpu->b.F&FLAG_CARRY ? 0x80 : 0;
}

static void djnz(cpuZ80 *cpu) /* 0x10 */
{
    register int8_t e = sread8(cpu, cpu->PC++);
    if(--cpu->b.B!=0) {
        cpu->cycles += 13;
        cpu->PC += e;
    } else {
        cpu->cycles += 8;
    }
}

static void ld_de_nn(cpuZ80 *cpu) /* 0x11 */
{
    cpu->cycles += 10;
    cpu->w.DE = read16(cpu, cpu->PC);
    cpu->PC += 2;
}

static void ld_mde_a(cpuZ80 *cpu) /* 0x12 */
{
    cpu->cycles += 7;
    write8(cpu, cpu->w.DE, cpu->b.A);
}

static void inc_de(cpuZ80 *cpu) /* 0x13 */
{
    cpu->cycles += 6;
    cpu->w.DE++;
}

static void inc_d(cpuZ80 *cpu) /* 0x14 */
{
    cpu->cycles += 4;
    cpu->b.D = inc8(cpu, cpu->b.D);
}

static void dec_d(cpuZ80 *cpu) /* 0x15 */
{
    cpu->cycles += 4;
    cpu->b.D = dec8(cpu, cpu->b.D);
}

static void ld_d_n(cpuZ80 *cpu) /* 0x16 */
{
    cpu->cycles += 7;
    cpu->b.D = read8(cpu, cpu->PC++);
}

static void rla(cpuZ80 *cpu) /* 0x17 */
{
    cpu->cycles += 4;
    byte carry = cpu->b.F & FLAG_CARRY;
    cpu->b.F &= ~(FLAG_HALF_CARRY | FLAG_ADD_SUB | FLAG_CARRY);
    cpu->b.F |= (cpu->b.A&0x80 ? FLAG_CARRY : 0);
    cpu->b.A <<= 1;
    cpu->b.A |= (carry ? 0x01 : 0);
}

static void jr_n(cpuZ80 *cpu) /* 0x18 */
{
    cpu->cycles += 12;
    register int8_t e = sread8(cpu, cpu->PC++);
    cpu->PC += e;
}

static void add_hl_de(cpuZ80 *cpu) /* 0x19 */
{
    cpu->cycles += 11;
    cpu->w.HL = add16(cpu, cpu->w.HL, cpu->w.DE);
}

static void ld_a_mde(cpuZ80 *cpu) /* 0x1A */
{
    cpu->cycles += 7;
    cpu->b.A = read8(cpu, cpu->w.DE);
}

static void dec_de(cpuZ80 *cpu) /* 0x1B */
{
    cpu->cycles += 6;
    cpu->w.DE--;
}

static void inc_e(cpuZ80 *cpu) /* 0x1C */
{
    cpu->cycles += 4;
    cpu->b.E = inc8(cpu, cpu->b.E);
}

static void dec_e(cpuZ80 *cpu) /* 0x1D */
{
    cpu->cycles += 4;
    cpu->b.E = dec8(cpu, cpu->b.E);
}

static void ld_e_n(cpuZ80 *cpu) /* 0x1E */
{
    cpu->cycles += 7;
    cpu->b.E = read8(cpu, cpu->PC++);
}

static void rra(cpuZ80 *cpu) /* 0x1E */
{
    cpu->cycles += 4;
    byte carry = cpu->b.F & FLAG_CARRY;
    cpu->b.F &= ~(FLAG_HALF_CARRY | FLAG_ADD_SUB | FLAG_CARRY);
    cpu->b.F |= (cpu->b.A&0x01 ? FLAG_CARRY : 0);
    cpu->b.A >>= 1;
    cpu->b.A |= (carry ? 0x80 : 0);
}

static void jr_nz(cpuZ80 *cpu) /* 0x20 */
{
    register int8_t e = sread8(cpu, cpu->PC++);
    if(cpu->b.F & FLAG_ZERO) {
        cpu->cycles += 7;
    } else {
        cpu->cycles += 12;
        cpu->PC += e;
    }
}

static void ld_hl_nn(cpuZ80 *cpu) /* 0x21 */
{
    cpu->cycles += 10;
    cpu->w.HL = read16(cpu, cpu->PC);
    cpu->PC += 2;
}

static void ld_mnn_hl(cpuZ80 *cpu) /* 0x22 */
{
    cpu->cycles += 16;
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    write16(cpu, addr, cpu->w.HL);
}

static void inc_hl(cpuZ80 *cpu) /* 0x23 */
{
    cpu->cycles += 6;
    cpu->w.HL++;
}

static void inc_h(cpuZ80 *cpu) /* 0x24 */
{
    cpu->cycles += 4;
    cpu->b.H = inc8(cpu, cpu->b.H);
}

static void dec_h(cpuZ80 *cpu) /* 0x25 */
{
    cpu->cycles += 4;
    cpu->b.H = dec8(cpu, cpu->b.H);
}

static void ld_h_n(cpuZ80 *cpu) /* 0x26 */
{
    cpu->cycles += 7;
    cpu->b.H = read8(cpu, cpu->PC++);
}

static void daa(cpuZ80 *cpu) /* 0x27 */
{
    byte carry, correction = 0;
    byte oldA = cpu->b.A;

    cpu->cycles += 4;

    carry = (cpu->b.A>0x99) || (cpu->b.F & FLAG_CARRY);
    correction = carry ? 0x60 : 0;

    if(((cpu->b.A&0x0F)>9) || (cpu->b.F & FLAG_HALF_CARRY)) {
        correction |= 0x06;
    }

    if(cpu->b.F & FLAG_ADD_SUB)
        cpu->b.A -= correction;
    else
        cpu->b.A += correction;

    cpu->b.F &= FLAG_ADD_SUB;
    cpu->b.F |= (carry ? FLAG_CARRY : 0);
    cpu->b.F |= ((oldA ^ cpu->b.A) & 0x10); assert(FLAG_HALF_CARRY==0x10);
    cpu->b.F |= flags_szp(cpu->b.A);
}

static void jr_z_n(cpuZ80 *cpu) /* 0x28 */
{
    register int8_t e = sread8(cpu, cpu->PC++);
    if(cpu->b.F & FLAG_ZERO) {
        cpu->cycles += 12;
        cpu->PC += e;
    } else {
        cpu->cycles += 7;
    }
}

static void add_hl_hl(cpuZ80 *cpu) /* 0x29 */
{
    cpu->cycles += 11;
    cpu->w.HL = add16(cpu, cpu->w.HL, cpu->w.HL);
}

static void ld_hl_mnn(cpuZ80 *cpu) /* 0x2A */
{
    cpu->cycles += 16;
    cpu->w.HL = read16(cpu, read16(cpu, cpu->PC));
    cpu->PC += 2;
}

static void dec_hl(cpuZ80 *cpu) /* 0x2B */
{
    cpu->cycles += 6;
    cpu->w.HL--;
}

static void inc_l(cpuZ80 *cpu) /* 0x2C */
{
    cpu->cycles += 4;
    cpu->b.L = inc8(cpu, cpu->b.L);
}

static void dec_l(cpuZ80 *cpu) /* 0x2D */
{
    cpu->cycles += 4;
    cpu->b.L = dec8(cpu, cpu->b.L);
}

static void ld_l_n(cpuZ80 *cpu) /* 0x2E */
{
    cpu->cycles += 7;
    cpu->b.L = read8(cpu, cpu->PC++);
}

static void cpl(cpuZ80 *cpu) /* 0x2F */
{
    cpu->cycles += 4;
    cpu->b.A ^= 0xFF;
    cpu->b.F |= (FLAG_HALF_CARRY | FLAG_ADD_SUB);
}

static void jr_nc_n(cpuZ80 *cpu) /* 0x30 */
{
    register int8_t e = sread8(cpu, cpu->PC++);
    if(cpu->b.F & FLAG_CARRY) {
        cpu->cycles += 7;
    } else {
        cpu->cycles += 12;
        cpu->PC += e;
    }
}

static void ld_sp_nn(cpuZ80 *cpu) /* 0x31 */
{
    cpu->cycles += 10;
    cpu->SP = read16(cpu, cpu->PC);
    cpu->PC += 2;
}

static void ld_mnn_a(cpuZ80 *cpu) /* 0x32 */
{
    cpu->cycles += 13;
    write8(cpu, read16(cpu, cpu->PC), cpu->b.A);
    cpu->PC += 2;
}

static void inc_sp(cpuZ80 *cpu) /* 0x33 */
{
    cpu->cycles += 6;
    cpu->SP++;
}

static void inc_mhl(cpuZ80 *cpu) /* 0x34 */
{
    cpu->cycles += 11;
    write8(cpu, cpu->w.HL, inc8(cpu, read8(cpu, cpu->w.HL)));
}

static void dec_mhl(cpuZ80 *cpu) /* 0x35 */
{
    cpu->cycles += 11;
    write8(cpu, cpu->w.HL, dec8(cpu, read8(cpu, cpu->w.HL)));
}

static void ld_mhl_n(cpuZ80 *cpu) /* 0x36 */
{
    cpu->cycles += 10;
    write8(cpu, cpu->w.HL, read8(cpu, cpu->PC++));
}

static void scf(cpuZ80 *cpu) /* 0x37 */
{
    cpu->cycles += 4;
    cpu->b.F |= FLAG_CARRY;
    cpu->b.F &= ~(FLAG_HALF_CARRY | FLAG_ADD_SUB);
}

static void jr_c_n(cpuZ80 *cpu) /* 0x38 */
{
    register int8_t e = sread8(cpu, cpu->PC++);
    if(cpu->b.F & FLAG_CARRY) {
        cpu->cycles += 12;
        cpu->PC += e;
    } else {
        cpu->cycles += 7;
    }
}

static void add_hl_sp(cpuZ80 *cpu) /* 0x39 */
{
    cpu->cycles += 11;
    cpu->w.HL = add16(cpu, cpu->w.HL, cpu->SP);
}


static void ld_a_mnn(cpuZ80 *cpu) /* 0x3A */
{
    cpu->cycles += 13;
    cpu->b.A = read8(cpu, read16(cpu, cpu->PC));
    cpu->PC += 2;
}

static void dec_sp(cpuZ80 *cpu) /* 0x3B */
{
    cpu->cycles += 6;
    cpu->SP--;
}

static void inc_a(cpuZ80 *cpu) /* 0x3C */
{
    cpu->cycles += 4;
    cpu->b.A = inc8(cpu, cpu->b.A);
}

static void dec_a(cpuZ80 *cpu) /* 0x3D */
{
    cpu->cycles += 4;
    cpu->b.A = dec8(cpu, cpu->b.A);
}

static void ld_a_n(cpuZ80 *cpu) /* 0x3E */
{
    cpu->cycles += 7;
    cpu->b.A = read8(cpu, cpu->PC++);
}

static void ccf(cpuZ80 *cpu) /* 0x40 */
{
    cpu->cycles += 4;
    cpu->b.F &= ~(FLAG_HALF_CARRY | FLAG_ADD_SUB);
    cpu->b.F |= (cpu->b.F&FLAG_CARRY ? FLAG_HALF_CARRY : 0);
    cpu->b.F ^= FLAG_CARRY;
}

static void ld_b_b(cpuZ80 *cpu) /* 0x40 */
{
    cpu->cycles += 4;
    //cpu->b.B = cpu->b.B;
}

static void ld_b_c(cpuZ80 *cpu) /* 0x41 */
{
    cpu->cycles += 4;
    cpu->b.B = cpu->b.C;
}

static void ld_b_d(cpuZ80 *cpu) /* 0x42 */
{
    cpu->cycles += 4;
    cpu->b.B = cpu->b.D;
}

static void ld_b_e(cpuZ80 *cpu) /* 0x43 */
{
    cpu->cycles += 4;
    cpu->b.B = cpu->b.E;
}

static void ld_b_h(cpuZ80 *cpu) /* 0x44 */
{
    cpu->cycles += 4;
    cpu->b.B = cpu->b.H;
}

static void ld_b_l(cpuZ80 *cpu) /* 0x45 */
{
    cpu->cycles += 4;
    cpu->b.B = cpu->b.L;
}

static void ld_b_mhl(cpuZ80 *cpu) /* 0x46 */
{
    cpu->cycles += 7;
    cpu->b.B = read8(cpu, cpu->w.HL);
}

static void ld_b_a(cpuZ80 *cpu) /* 0x47 */
{
    cpu->cycles += 4;
    cpu->b.B = cpu->b.A;
}

static void ld_c_b(cpuZ80 *cpu) /* 0x48 */
{
    cpu->cycles += 4;
    cpu->b.C = cpu->b.B;
}

static void ld_c_c(cpuZ80 *cpu) /* 0x49 */
{
    cpu->cycles += 4;
    cpu->b.C = cpu->b.C;
}

static void ld_c_d(cpuZ80 *cpu) /* 0x4A */
{
    cpu->cycles += 4;
    cpu->b.C = cpu->b.D;
}

static void ld_c_e(cpuZ80 *cpu) /* 0x4B */
{
    cpu->cycles += 4;
    cpu->b.C = cpu->b.E;
}

static void ld_c_h(cpuZ80 *cpu) /* 0x4C */
{
    cpu->cycles += 4;
    cpu->b.C = cpu->b.H;
}

static void ld_c_l(cpuZ80 *cpu) /* 0x4D */
{
    cpu->cycles += 4;
    cpu->b.C = cpu->b.L;
}

static void ld_c_mhl(cpuZ80 *cpu) /* 0x4E */
{
    cpu->cycles += 7;
    cpu->b.C = read8(cpu, cpu->w.HL);
}

static void ld_c_a(cpuZ80 *cpu) /* 0x4F */
{
    cpu->cycles += 4;
    cpu->b.C = cpu->b.A;
}

static void ld_d_b(cpuZ80 *cpu) /* 0x50 */
{
    cpu->cycles += 4;
    cpu->b.D = cpu->b.B;
}

static void ld_d_c(cpuZ80 *cpu) /* 0x51 */
{
    cpu->cycles += 4;
    cpu->b.D = cpu->b.C;
}

static void ld_d_d(cpuZ80 *cpu) /* 0x52 */
{
    cpu->cycles += 4;
    cpu->b.D = cpu->b.D;
}

static void ld_d_e(cpuZ80 *cpu) /* 0x53 */
{
    cpu->cycles += 4;
    cpu->b.D = cpu->b.E;
}

static void ld_d_h(cpuZ80 *cpu) /* 0x54 */
{
    cpu->cycles += 4;
    cpu->b.D = cpu->b.H;
}

static void ld_d_l(cpuZ80 *cpu) /* 0x55 */
{
    cpu->cycles += 4;
    cpu->b.D = cpu->b.L;
}

static void ld_d_mhl(cpuZ80 *cpu) /* 0x56 */
{
    cpu->cycles += 7;
    cpu->b.D = read8(cpu, cpu->w.HL);
}

static void ld_d_a(cpuZ80 *cpu) /* 0x57 */
{
    cpu->cycles += 4;
    cpu->b.D = cpu->b.A;
}

static void ld_e_b(cpuZ80 *cpu) /* 0x58 */
{
    cpu->cycles += 4;
    cpu->b.E = cpu->b.B;
}

static void ld_e_c(cpuZ80 *cpu) /* 0x59 */
{
    cpu->cycles += 4;
    cpu->b.E = cpu->b.C;
}

static void ld_e_d(cpuZ80 *cpu) /* 0x5A */
{
    cpu->cycles += 4;
    cpu->b.E = cpu->b.D;
}

static void ld_e_e(cpuZ80 *cpu) /* 0x5B */
{
    cpu->cycles += 4;
    cpu->b.E = cpu->b.E;
}

static void ld_e_h(cpuZ80 *cpu) /* 0x5C */
{
    cpu->cycles += 4;
    cpu->b.E = cpu->b.H;
}

static void ld_e_l(cpuZ80 *cpu) /* 0x5D */
{
    cpu->cycles += 4;
    cpu->b.E = cpu->b.L;
}

static void ld_e_mhl(cpuZ80 *cpu) /* 0x5E */
{
    cpu->cycles += 7;
    cpu->b.E = read8(cpu, cpu->w.HL);
}

static void ld_e_a(cpuZ80 *cpu) /* 0x5F */
{
    cpu->cycles += 4;
    cpu->b.E = cpu->b.A;
}

static void ld_h_b(cpuZ80 *cpu) /* 0x60 */
{
    cpu->cycles += 4;
    cpu->b.H = cpu->b.B;
}

static void ld_h_c(cpuZ80 *cpu) /* 0x61 */
{
    cpu->cycles += 4;
    cpu->b.H = cpu->b.C;
}

static void ld_h_d(cpuZ80 *cpu) /* 0x62 */
{
    cpu->cycles += 4;
    cpu->b.H = cpu->b.D;
}

static void ld_h_e(cpuZ80 *cpu) /* 0x63 */
{
    cpu->cycles += 4;
    cpu->b.H = cpu->b.E;
}

static void ld_h_h(cpuZ80 *cpu) /* 0x64 */
{
    cpu->cycles += 4;
    //cpu->b.H = cpu->b.H;
}

static void ld_h_l(cpuZ80 *cpu) /* 0x65 */
{
    cpu->cycles += 4;
    cpu->b.H = cpu->b.L;
}

static void ld_h_mhl(cpuZ80 *cpu) /* 0x66 */
{
    cpu->cycles += 7;
    cpu->b.H = read8(cpu, cpu->w.HL);
}

static void ld_h_a(cpuZ80 *cpu) /* 0x67 */
{
    cpu->cycles += 4;
    cpu->b.H = cpu->b.A;
}

static void ld_l_b(cpuZ80 *cpu) /* 0x68 */
{
    cpu->cycles += 4;
    cpu->b.L = cpu->b.B;
}

static void ld_l_c(cpuZ80 *cpu) /* 0x69 */
{
    cpu->cycles += 4;
    cpu->b.L = cpu->b.C;
}

static void ld_l_d(cpuZ80 *cpu) /* 0x6A */
{
    cpu->cycles += 4;
    cpu->b.L = cpu->b.D;
}

static void ld_l_e(cpuZ80 *cpu) /* 0x6B */
{
    cpu->cycles += 4;
    cpu->b.L = cpu->b.E;
}

static void ld_l_h(cpuZ80 *cpu) /* 0x6C */
{
    cpu->cycles += 4;
    cpu->b.L = cpu->b.H;
}

static void ld_l_l(cpuZ80 *cpu) /* 0x6D */
{
    cpu->cycles += 4;
    //cpu->b.L = cpu->b.L;
}

static void ld_l_mhl(cpuZ80 *cpu) /* 0x6E */
{
    cpu->cycles += 7;
    cpu->b.L = read8(cpu, cpu->w.HL);
}

static void ld_l_a(cpuZ80 *cpu) /* 0x6F */
{
    cpu->cycles += 4;
    cpu->b.L = cpu->b.A;
}

static void ld_mhl_b(cpuZ80 *cpu) /* 0x70 */
{
    cpu->cycles += 7;
    write8(cpu, cpu->w.HL, cpu->b.B);
}

static void ld_mhl_c(cpuZ80 *cpu) /* 0x71 */
{
    cpu->cycles += 7;
    write8(cpu, cpu->w.HL, cpu->b.C);
}

static void ld_mhl_d(cpuZ80 *cpu) /* 0x72 */
{
    cpu->cycles += 7;
    write8(cpu, cpu->w.HL, cpu->b.D);
}

static void ld_mhl_e(cpuZ80 *cpu) /* 0x73 */
{
    cpu->cycles += 7;
    write8(cpu, cpu->w.HL, cpu->b.E);
}

static void ld_mhl_h(cpuZ80 *cpu) /* 0x74 */
{
    cpu->cycles += 7;
    write8(cpu, cpu->w.HL, cpu->b.H);
}

static void ld_mhl_l(cpuZ80 *cpu) /* 0x75 */
{
    cpu->cycles += 7;
    write8(cpu, cpu->w.HL, cpu->b.L);
}

static void halt(cpuZ80 *cpu) /* 0x76 */
{
    cpu->cycles += 4;
    cpu->halted = 1;
    cpu->PC--;
}

static void ld_mhl_a(cpuZ80 *cpu) /* 0x77 */
{
    cpu->cycles += 7;
    write8(cpu, cpu->w.HL, cpu->b.A);
}

static void ld_a_b(cpuZ80 *cpu) /* 0x78 */
{
   cpu->cycles += 4;
   cpu->b.A = cpu->b.B;
}

static void ld_a_c(cpuZ80 *cpu) /* 0x79 */
{
   cpu->cycles += 4;
   cpu->b.A = cpu->b.C;
}

static void ld_a_d(cpuZ80 *cpu) /* 0x7A */
{
   cpu->cycles += 4;
   cpu->b.A = cpu->b.D;
}

static void ld_a_e(cpuZ80 *cpu) /* 0x7B */
{
   cpu->cycles += 4;
   cpu->b.A = cpu->b.E;
}

static void ld_a_h(cpuZ80 *cpu) /* 0x7C */
{
   cpu->cycles += 4;
   cpu->b.A = cpu->b.H;
}

static void ld_a_l(cpuZ80 *cpu) /* 0x7D */
{
   cpu->cycles += 4;
   cpu->b.A = cpu->b.L;
}

static void ld_a_mhl(cpuZ80 *cpu) /* 0x7E */
{
    cpu->cycles += 7;
    cpu->b.A = read8(cpu, cpu->w.HL);
}

static void ld_a_a(cpuZ80 *cpu) /* 0x7F */
{
   cpu->cycles += 4;
   //cpu->b.A = cpu->b.A;
}

static void add_a_b(cpuZ80 *cpu) /* 0x80 */
{
    cpu->cycles += 4;
    cpu->b.A = add8(cpu, cpu->b.A, cpu->b.B);
}

static void add_a_c(cpuZ80 *cpu) /* 0x81 */
{
    cpu->cycles += 4;
    cpu->b.A = add8(cpu, cpu->b.A, cpu->b.C);
}

static void add_a_d(cpuZ80 *cpu) /* 0x82 */
{
    cpu->cycles += 4;
    cpu->b.A = add8(cpu, cpu->b.A, cpu->b.D);
}

static void add_a_e(cpuZ80 *cpu) /* 0x83 */
{
    cpu->cycles += 4;
    cpu->b.A = add8(cpu, cpu->b.A, cpu->b.E);
}

static void add_a_h(cpuZ80 *cpu) /* 0x84 */
{
    cpu->cycles += 4;
    cpu->b.A = add8(cpu, cpu->b.A, cpu->b.H);
}

static void add_a_l(cpuZ80 *cpu) /* 0x85 */
{
    cpu->cycles += 4;
    cpu->b.A = add8(cpu, cpu->b.A, cpu->b.L);
}

static void add_a_mhl(cpuZ80 *cpu) /* 0x86 */
{
    cpu->cycles += 7;
    cpu->b.A = add8(cpu, cpu->b.A, read8(cpu, cpu->w.HL));
}

static void add_a_a(cpuZ80 *cpu) /* 0x87 */
{
    cpu->cycles += 4;
    cpu->b.A = add8(cpu, cpu->b.A, cpu->b.A);
}

static void adc_a_b(cpuZ80 *cpu) /* 0x88 */
{
    cpu->cycles += 4;
    cpu->b.A = adc8(cpu, cpu->b.A, cpu->b.B);
}

static void adc_a_c(cpuZ80 *cpu) /* 0x89 */
{
    cpu->cycles += 4;
    cpu->b.A = adc8(cpu, cpu->b.A, cpu->b.C);
}

static void adc_a_d(cpuZ80 *cpu) /* 0x8A */
{
    cpu->cycles += 4;
    cpu->b.A = adc8(cpu, cpu->b.A, cpu->b.D);
}

static void adc_a_e(cpuZ80 *cpu) /* 0x8B */
{
    cpu->cycles += 4;
    cpu->b.A = adc8(cpu, cpu->b.A, cpu->b.E);
}

static void adc_a_h(cpuZ80 *cpu) /* 0x8C */
{
    cpu->cycles += 4;
    cpu->b.A = adc8(cpu, cpu->b.A, cpu->b.H);
}

static void adc_a_l(cpuZ80 *cpu) /* 0x8D */
{
    cpu->cycles += 4;
    cpu->b.A = adc8(cpu, cpu->b.A, cpu->b.L);
}

static void adc_a_mhl(cpuZ80 *cpu) /* 0x8E */
{
    cpu->cycles += 7;
    cpu->b.A = adc8(cpu, cpu->b.A, read8(cpu, cpu->w.HL));
}

static void adc_a_a(cpuZ80 *cpu) /* 0x8F */
{
    cpu->cycles += 4;
    cpu->b.A = adc8(cpu, cpu->b.A, cpu->b.A);
}

static void sub_b(cpuZ80 *cpu) /* 0x90 */
{
    cpu->cycles += 4;
    cpu->b.A = sub8(cpu, cpu->b.A, cpu->b.B);
}

static void sub_c(cpuZ80 *cpu) /* 0x91 */
{
    cpu->cycles += 4;
    cpu->b.A = sub8(cpu, cpu->b.A, cpu->b.C);
}

static void sub_d(cpuZ80 *cpu) /* 0x92 */
{
    cpu->cycles += 4;
    cpu->b.A = sub8(cpu, cpu->b.A, cpu->b.D);
}

static void sub_e(cpuZ80 *cpu) /* 0x93 */
{
    cpu->cycles += 4;
    cpu->b.A = sub8(cpu, cpu->b.A, cpu->b.E);
}

static void sub_h(cpuZ80 *cpu) /* 0x94 */
{
    cpu->cycles += 4;
    cpu->b.A = sub8(cpu, cpu->b.A, cpu->b.H);
}

static void sub_l(cpuZ80 *cpu) /* 0x95 */
{
    cpu->cycles += 4;
    cpu->b.A = sub8(cpu, cpu->b.A, cpu->b.L);
}

static void sub_mhl(cpuZ80 *cpu) /* 0x96 */
{
    cpu->cycles += 7;
    cpu->b.A = sub8(cpu, cpu->b.A, read8(cpu, cpu->w.HL));
}

static void sub_a(cpuZ80 *cpu) /* 0x97 */
{
    cpu->cycles += 4;
    cpu->b.A = sub8(cpu, cpu->b.A, cpu->b.A);
}

static void sbc_a_b(cpuZ80 *cpu) /* 0x98 */
{
    cpu->cycles += 4;
    cpu->b.A = sbc8(cpu, cpu->b.A, cpu->b.B);
}

static void sbc_a_c(cpuZ80 *cpu) /* 0x99 */
{
    cpu->cycles += 4;
    cpu->b.A = sbc8(cpu, cpu->b.A, cpu->b.C);
}

static void sbc_a_d(cpuZ80 *cpu) /* 0x9A */
{
    cpu->cycles += 4;
    cpu->b.A = sbc8(cpu, cpu->b.A, cpu->b.D);
}

static void sbc_a_e(cpuZ80 *cpu) /* 0x9B */
{
    cpu->cycles += 4;
    cpu->b.A = sbc8(cpu, cpu->b.A, cpu->b.E);
}

static void sbc_a_h(cpuZ80 *cpu) /* 0x9C */
{
    cpu->cycles += 4;
    cpu->b.A = sbc8(cpu, cpu->b.A, cpu->b.H);
}

static void sbc_a_l(cpuZ80 *cpu) /* 0x9D */
{
    cpu->cycles += 4;
    cpu->b.A = sbc8(cpu, cpu->b.A, cpu->b.L);
}

static void sbc_a_mhl(cpuZ80 *cpu) /* 0x9E */
{
    cpu->cycles += 7;
    cpu->b.A = sbc8(cpu, cpu->b.A, read8(cpu, cpu->w.HL));
}

static void sbc_a_a(cpuZ80 *cpu) /* 0x9F */
{
    cpu->cycles += 4;
    cpu->b.A = sbc8(cpu, cpu->b.A, cpu->b.A);
}

static void and_b(cpuZ80 *cpu) /* 0xA0 */
{
    cpu->cycles += 4;
    cpu->b.A = and8(cpu, cpu->b.A, cpu->b.B);
}

static void and_c(cpuZ80 *cpu) /* 0xA1 */
{
    cpu->cycles += 4;
    cpu->b.A = and8(cpu, cpu->b.A, cpu->b.C);
}

static void and_d(cpuZ80 *cpu) /* 0xA2 */
{
    cpu->cycles += 4;
    cpu->b.A = and8(cpu, cpu->b.A, cpu->b.D);
}

static void and_e(cpuZ80 *cpu) /* 0xA3 */
{
    cpu->cycles += 4;
    cpu->b.A = and8(cpu, cpu->b.A, cpu->b.E);
}

static void and_h(cpuZ80 *cpu) /* 0xA4 */
{
    cpu->cycles += 4;
    cpu->b.A = and8(cpu, cpu->b.A, cpu->b.H);
}

static void and_l(cpuZ80 *cpu) /* 0xA5 */
{
    cpu->cycles += 4;
    cpu->b.A = and8(cpu, cpu->b.A, cpu->b.L);
}

static void and_mhl(cpuZ80 *cpu) /* 0xA6 */
{
    cpu->cycles += 7;
    cpu->b.A = and8(cpu, cpu->b.A, read8(cpu, cpu->w.HL));
}

static void and_a(cpuZ80 *cpu) /* 0xA7 */
{
    cpu->cycles += 4;
    cpu->b.A = and8(cpu, cpu->b.A, cpu->b.A);
}

static void xor_b(cpuZ80 *cpu) /* 0xA8 */
{
    cpu->cycles += 4;
    cpu->b.A = xor8(cpu, cpu->b.A, cpu->b.B);
}

static void xor_c(cpuZ80 *cpu) /* 0xA9 */
{
    cpu->cycles += 4;
    cpu->b.A = xor8(cpu, cpu->b.A, cpu->b.C);
}

static void xor_d(cpuZ80 *cpu) /* 0xAA */
{
    cpu->cycles += 4;
    cpu->b.A = xor8(cpu, cpu->b.A, cpu->b.D);
}

static void xor_e(cpuZ80 *cpu) /* 0xAB */
{
    cpu->cycles += 4;
    cpu->b.A = xor8(cpu, cpu->b.A, cpu->b.E);
}

static void xor_h(cpuZ80 *cpu) /* 0xAC */
{
    cpu->cycles += 4;
    cpu->b.A = xor8(cpu, cpu->b.A, cpu->b.H);
}

static void xor_l(cpuZ80 *cpu) /* 0xAD */
{
    cpu->cycles += 4;
    cpu->b.A = xor8(cpu, cpu->b.A, cpu->b.L);
}

static void xor_mhl(cpuZ80 *cpu) /* 0xAE */
{
    cpu->cycles += 7;
    cpu->b.A = xor8(cpu, cpu->b.A, read8(cpu, cpu->w.HL));
}

static void xor_a(cpuZ80 *cpu) /* 0xAF */
{
    cpu->cycles += 4;
    cpu->b.A = xor8(cpu, cpu->b.A, cpu->b.A);
}

static void or_b(cpuZ80 *cpu) /* 0xB0 */
{
    cpu->cycles += 4;
    cpu->b.A = or8(cpu, cpu->b.A, cpu->b.B);
}

static void or_c(cpuZ80 *cpu) /* 0xB1 */
{
    cpu->cycles += 4;
    cpu->b.A = or8(cpu, cpu->b.A, cpu->b.C);
}

static void or_d(cpuZ80 *cpu) /* 0xB2 */
{
    cpu->cycles += 4;
    cpu->b.A = or8(cpu, cpu->b.A, cpu->b.D);
}

static void or_e(cpuZ80 *cpu) /* 0xB3 */
{
    cpu->cycles += 4;
    cpu->b.A = or8(cpu, cpu->b.A, cpu->b.E);
}

static void or_h(cpuZ80 *cpu) /* 0xB4 */
{
    cpu->cycles += 4;
    cpu->b.A = or8(cpu, cpu->b.A, cpu->b.H);
}

static void or_l(cpuZ80 *cpu) /* 0xB5 */
{
    cpu->cycles += 4;
    cpu->b.A = or8(cpu, cpu->b.A, cpu->b.L);
}

static void or_mhl(cpuZ80 *cpu) /* 0xB6 */
{
    cpu->cycles += 7;
    cpu->b.A = or8(cpu, cpu->b.A, read8(cpu, cpu->w.HL));
}

static void or_a(cpuZ80 *cpu) /* 0xB7 */
{
    cpu->cycles += 4;
    cpu->b.A = or8(cpu, cpu->b.A, cpu->b.A);
}

static void cp_b(cpuZ80 *cpu) /* 0xB8 */
{
    cpu->cycles += 4;
    sub8(cpu, cpu->b.A, cpu->b.B);
}

static void cp_c(cpuZ80 *cpu) /* 0xB9 */
{
    cpu->cycles += 4;
    sub8(cpu, cpu->b.A, cpu->b.C);
}

static void cp_d(cpuZ80 *cpu) /* 0xBA */
{
    cpu->cycles += 4;
    sub8(cpu, cpu->b.A, cpu->b.D);
}

static void cp_e(cpuZ80 *cpu) /* 0xBB */
{
    cpu->cycles += 4;
    sub8(cpu, cpu->b.A, cpu->b.E);
}

static void cp_h(cpuZ80 *cpu) /* 0xBC */
{
    cpu->cycles += 4;
    sub8(cpu, cpu->b.A, cpu->b.H);
}

static void cp_l(cpuZ80 *cpu) /* 0xBD */
{
    cpu->cycles += 4;
    sub8(cpu, cpu->b.A, cpu->b.L);
}

static void cp_mhl(cpuZ80 *cpu) /* 0xBE */
{
    cpu->cycles += 7;
    sub8(cpu, cpu->b.A, read8(cpu, cpu->w.HL));
}

static void cp_a(cpuZ80 *cpu) /* 0xBF */
{
    cpu->cycles += 4;
    sub8(cpu, cpu->b.A, cpu->b.A);
}

static void ret_nz(cpuZ80 *cpu) /* 0xC0 */
{
    cpu->cycles += 5;
    if(!(cpu->b.F & FLAG_ZERO)) {
        cpu->cycles += 6;
        POP(cpu->PC);
    }
}

static void pop_bc(cpuZ80 *cpu) /* 0xC1 */
{
    cpu->cycles += 10;
    POP(cpu->w.BC);
}

static void jp_nz_nn(cpuZ80 *cpu) /* 0xC2 */
{
    cpu->cycles += 10;
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(!(cpu->b.F & FLAG_ZERO)) cpu->PC = addr;
}

static void jp_nn(cpuZ80 *cpu) /* 0xC3 */
{
    cpu->cycles += 10;
    cpu->PC = read16(cpu, cpu->PC);
}

static void call_nz_nn(cpuZ80 *cpu) /* 0xC4 */
{
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_ZERO) {
        cpu->cycles += 10;
    } else {
        cpu->cycles += 17;
        PUSH(cpu->PC);
        cpu->PC = addr;
    }
}

static void push_bc(cpuZ80 *cpu) /* 0xC5 */
{
    cpu->cycles += 11;
    PUSH(cpu->w.BC);
}

static void add_a_n(cpuZ80 *cpu) /* 0xC6 */
{
    cpu->cycles += 7;
    cpu->b.A = add8(cpu, cpu->b.A, read8(cpu, cpu->PC++));
}

static void rst_0(cpuZ80 *cpu) /* 0xC7 */
{
    RST(0x0);
}

static void ret_z(cpuZ80 *cpu) /* 0xC8 */
{
    cpu->cycles += 5;
    if(cpu->b.F & FLAG_ZERO) {
        cpu->cycles += 6;
        POP(cpu->PC);
    }
}

static void ret(cpuZ80 *cpu) /* 0xC9 */
{
    cpu->cycles += 10;
    POP(cpu->PC);
}

static void jp_z_nn(cpuZ80 *cpu) /* 0xCA */
{
    cpu->cycles += 10;
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_ZERO) cpu->PC = addr;
}

static void call_z_nn(cpuZ80 *cpu) /* 0xCC */
{
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_ZERO) {
        cpu->cycles += 17;
        PUSH(cpu->PC);
        cpu->PC = addr;
    } else
       cpu->cycles += 10;
}

static void call_nn(cpuZ80 *cpu) /* 0xCD */
{
    cpu->cycles += 17;
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    PUSH(cpu->PC);
    cpu->PC = addr;
}

static void adc_a_n(cpuZ80 *cpu) /* 0xCE */
{
    cpu->cycles += 7;
    cpu->b.A = adc8(cpu, cpu->b.A, read8(cpu, cpu->PC++));
}

static void rst_8(cpuZ80 *cpu) /* 0xCF */
{
    RST(0x8);
}

static void ret_nc(cpuZ80 *cpu) /* 0xD0 */
{
    cpu->cycles += 5;
    if(!(cpu->b.F & FLAG_CARRY)) {
        cpu->cycles += 6;
        POP(cpu->PC);
    }
}

static void pop_de(cpuZ80 *cpu) /* 0xD1 */
{
    cpu->cycles += 10;
    POP(cpu->w.DE);
}

static void jp_nc_nn(cpuZ80 *cpu) /* 0xD2 */
{
    cpu->cycles += 10;
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(!(cpu->b.F & FLAG_CARRY)) cpu->PC = addr;
}

static void out_n_a(cpuZ80 *cpu) /* 0xD3 */
{
    cpu->cycles += 11;
    writeio(cpu, read8(cpu, cpu->PC++), cpu->b.A);
}

static void call_nc_nn(cpuZ80 *cpu) /* 0xD4 */
{
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_CARRY) {
        cpu->cycles += 10;
    } else {
        cpu->cycles += 17;
        PUSH(cpu->PC);
        cpu->PC = addr;
    }
}

static void push_de(cpuZ80 *cpu) /* 0xD5 */
{
    cpu->cycles += 11;
    PUSH(cpu->w.DE);
}

static void sub_n(cpuZ80 *cpu) /* 0xD6 */
{
    cpu->cycles += 7;
    cpu->b.A = sub8(cpu, cpu->b.A, read8(cpu, cpu->PC++));
}

static void rst_10(cpuZ80 *cpu) /* 0xD7 */
{
    RST(0x10);
}

static void ret_c(cpuZ80 *cpu) /* 0xD8 */
{
    cpu->cycles += 5;
    if(cpu->b.F & FLAG_CARRY) {
        cpu->cycles += 6;
        POP(cpu->PC);
    }
}

static void exx(cpuZ80 *cpu) /* 0xD9 */
{
    cpu->cycles += 4;
    word tmp = cpu->w.BC;
    cpu->w.BC = cpu->wp.BCp;
    cpu->wp.BCp = tmp;
    tmp = cpu->w.DE;
    cpu->w.DE = cpu->wp.DEp;
    cpu->wp.DEp = tmp;
    tmp = cpu->w.HL;
    cpu->w.HL = cpu->wp.HLp;
    cpu->wp.HLp = tmp;
}

static void jp_c_nn(cpuZ80 *cpu) /* 0xDA */
{
    cpu->cycles += 10;
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_CARRY) cpu->PC = addr;
}

static void in_a_n(cpuZ80 *cpu) /* 0xDB */
{
    cpu->cycles += 11;
    cpu->b.A = readio(cpu, read8(cpu, cpu->PC++));
}

static void call_c_nn(cpuZ80 *cpu) /* 0xDC */
{
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_CARRY) {
        cpu->cycles += 17;
        PUSH(cpu->PC);
        cpu->PC = addr;
    } else
       cpu->cycles += 10;
}

static void sbc_a_n(cpuZ80 *cpu) /* 0xDE */
{
    cpu->cycles += 7;
    cpu->b.A = sbc8(cpu, cpu->b.A, read8(cpu, cpu->PC++));
}

static void rst_18(cpuZ80 *cpu) /* 0xDF */
{
    RST(0x18);
}

static void ret_po(cpuZ80 *cpu) /* 0xE0 */
{
    cpu->cycles += 5;
    if(!(cpu->b.F & FLAG_PARITY)) {
        cpu->cycles += 6;
        POP(cpu->PC);
    }
}

static void pop_hl(cpuZ80 *cpu) /* 0xE1 */
{
    cpu->cycles += 10;
    POP(cpu->w.HL);
}

static void jp_po_nn(cpuZ80 *cpu) /* 0xE2 */
{
    cpu->cycles += 10;
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(!(cpu->b.F & FLAG_PARITY)) cpu->PC = addr;
}

static void ex_msp_hl(cpuZ80 *cpu) /* 0xE3 */
{
    cpu->cycles += 19;
    register word tmp = read16(cpu, cpu->SP);
    write16(cpu, cpu->SP, cpu->w.HL);
    cpu->w.HL = tmp;
}

static void call_po_nn(cpuZ80 *cpu) /* 0xE4 */
{
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_PARITY) {
        cpu->cycles += 10;
    } else {
        cpu->cycles += 17;
        PUSH(cpu->PC);
        cpu->PC = addr;
    }
}

static void push_hl(cpuZ80 *cpu) /* 0xE5 */
{
    cpu->cycles += 11;
    PUSH(cpu->w.HL);
}

static void and_n(cpuZ80 *cpu) /* 0xE6 */
{
    cpu->cycles += 7;
    cpu->b.A = and8(cpu, cpu->b.A, read8(cpu, cpu->PC++));
}

static void rst_20(cpuZ80 *cpu) /* 0xE7 */
{
    RST(0x20);
}

static void ret_pe(cpuZ80 *cpu) /* 0xE8 */
{
    cpu->cycles += 5;
    if(cpu->b.F & FLAG_PARITY) {
        cpu->cycles += 6;
        POP(cpu->PC);
    }
}

static void jp_hl(cpuZ80 *cpu) /* 0xE9 */
{
    cpu->cycles += 4;
    cpu->PC = cpu->w.HL;
}

static void jp_pe_nn(cpuZ80 *cpu) /* 0xEA */
{
    cpu->cycles += 10;
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_PARITY) cpu->PC = addr;
}

static void ex_de_hl(cpuZ80 *cpu) /* 0xEB */
{
    cpu->cycles += 4;
    register word tmp = cpu->w.DE;
    cpu->w.DE = cpu->w.HL;
    cpu->w.HL = tmp;
}

static void call_pe_nn(cpuZ80 *cpu) /* 0xEC */
{
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_PARITY) {
        cpu->cycles += 17;
        PUSH(cpu->PC);
        cpu->PC = addr;
    } else
       cpu->cycles += 10;
}

static void xor_n(cpuZ80 *cpu) /* 0xEE */
{
    cpu->cycles += 7;
    cpu->b.A = xor8(cpu, cpu->b.A, read8(cpu, cpu->PC++));
}

static void rst_28(cpuZ80 *cpu) /* 0xEF */
{
    RST(0x28);
}

static void ret_p(cpuZ80 *cpu) /* 0xF0 */
{
    cpu->cycles += 5;
    if(!(cpu->b.F & FLAG_SIGN)) {
        cpu->cycles += 6;
        POP(cpu->PC);
    }
}

static void pop_af(cpuZ80 *cpu) /* 0xF1 */
{
    cpu->cycles += 10;
    POP(cpu->w.AF);
}

static void jp_p_nn(cpuZ80 *cpu) /* 0xF2 */
{
    cpu->cycles += 10;
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(!(cpu->b.F & FLAG_SIGN)) cpu->PC = addr;
}

static void di(cpuZ80 *cpu) /* 0xF3 */
{
    cpu->cycles += 4;
    cpu->IFF1 = cpu->IFF2 = 0;
}

static void call_p_nn(cpuZ80 *cpu) /* 0xC4 */
{
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_SIGN) {
        cpu->cycles += 10;
    } else {
        cpu->cycles += 17;
        PUSH(cpu->PC);
        cpu->PC = addr;
    }
}

static void push_af(cpuZ80 *cpu) /* 0xF5 */
{
    cpu->cycles += 11;
    PUSH(cpu->w.AF);
}

static void or_n(cpuZ80 *cpu) /* 0xF6 */
{
    cpu->cycles += 7;
    cpu->b.A |= read8(cpu, cpu->PC++);
    cpu->b.F = flags_szp(cpu->b.A);
}

static void rst_30(cpuZ80 *cpu) /* 0xF7 */
{
    RST(0x30);
}

static void ret_m(cpuZ80 *cpu) /* 0xF8 */
{
    cpu->cycles += 5;
    if(cpu->b.F & FLAG_SIGN) {
        cpu->cycles += 6;
        POP(cpu->PC);
    }
}

static void ld_sp_hl(cpuZ80 *cpu) /* 0xF9 */
{
    cpu->cycles += 6;
    cpu->SP = cpu->w.HL;
}

static void jp_m_nn(cpuZ80 *cpu) /* 0xFA */
{
    cpu->cycles += 10;
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_SIGN) cpu->PC = addr;
}

static void ei(cpuZ80 *cpu) /* 0xFB */
{
    cpu->cycles += 4;
    cpu->IFF1 = cpu->IFF2 = 1;
}

static void call_m_nn(cpuZ80 *cpu) /* 0xFC */
{
    register word addr = read16(cpu, cpu->PC);
    cpu->PC += 2;
    if(cpu->b.F & FLAG_SIGN) {
        cpu->cycles += 17;
        PUSH(cpu->PC);
        cpu->PC = addr;
    } else
       cpu->cycles += 10;
}

static void cp_n(cpuZ80 *cpu) /* 0xFE */
{
    cpu->cycles += 7;
    sub8(cpu, cpu->b.A, read8(cpu, cpu->PC++));
}

static void rst_38(cpuZ80 *cpu) /* 0xFF */
{
    RST(0x38);
}


opcode opcodes[256] = {
    { nop,          DASM("NOP")         /* 0x00 */ },
    { ld_bc_nn,     DASM("LD BC,#nn")   /* 0x01 */ },
    { ld_mbc_a,     DASM("LD (BC),A")   /* 0x02 */ },
    { inc_bc,       DASM("INC BC")      /* 0x03 */ },
    { inc_b,        DASM("INC B")       /* 0x04 */ },
    { dec_b,        DASM("DEC B")       /* 0x05 */ },
    { ld_b_n,       DASM("LD B,#n")     /* 0x06 */ },
    { rlca,         DASM("RLCA")        /* 0x07 */ },
    { ex_af_afp,    DASM("EX AF,AF'")   /* 0x08 */ },
    { add_hl_bc,    DASM("ADD HL,BC")   /* 0x09 */ },
    { ld_a_mbc,     DASM("LD A,(BC)")   /* 0x0A */ },
    { dec_bc,       DASM("DEC BC")      /* 0x0B */ },
    { inc_c,        DASM("INC C")       /* 0x0C */ },
    { dec_c,        DASM("DEC C")       /* 0x0D */ },
    { ld_c_n,       DASM("LD C,#n")     /* 0x0E */ },
    { rrca,         DASM("RRCA")        /* 0x0F */ },
    { djnz,         DASM("DJNZ #e")     /* 0x10 */ },
    { ld_de_nn,     DASM("LD DE,#nn")   /* 0x11 */ },
    { ld_mde_a,     DASM("LD (DE),A")   /* 0x12 */ },
    { inc_de,       DASM("INC DE")      /* 0x13 */ },
    { inc_d,        DASM("INC D")       /* 0x14 */ },
    { dec_d,        DASM("DEC D")       /* 0x15 */ },
    { ld_d_n,       DASM("LD D,#n")     /* 0x16 */ },
    { rla,          DASM("RLA")         /* 0x17 */ },
    { jr_n,         DASM("JR #e")       /* 0x18 */ },
    { add_hl_de,    DASM("ADD HL,DE")   /* 0x19 */ },
    { ld_a_mde,     DASM("LD A,(DE)")   /* 0x1A */ },
    { dec_de,       DASM("DEC DE")      /* 0x1B */ },
    { inc_e,        DASM("INC E")       /* 0x1C */ },
    { dec_e,        DASM("DEC E")       /* 0x1D */ },
    { ld_e_n,       DASM("LD E,#n")     /* 0x1E */ },
    { rra,          DASM("RRA")         /* 0x1F */ },
    { jr_nz,        DASM("JR NZ,#e")    /* 0x20 */ },
    { ld_hl_nn,     DASM("LD HL,#nn")   /* 0x21 */ },
    { ld_mnn_hl,    DASM("LD (#nn),HL") /* 0x22 */ },
    { inc_hl,       DASM("INC HL")      /* 0x23 */ },
    { inc_h,        DASM("INC H")       /* 0x24 */ },
    { dec_h,        DASM("DEC H")       /* 0x25 */ },
    { ld_h_n,       DASM("LD H,#n")     /* 0x26 */ },
    { daa,          DASM("DAA")         /* 0x27 */ },
    { jr_z_n,       DASM("JR Z,#e")     /* 0x28 */ },
    { add_hl_hl,    DASM("ADD HL,HL")   /* 0x29 */ },
    { ld_hl_mnn,    DASM("LD HL,(#nn)") /* 0x2A */ },
    { dec_hl,       DASM("DEC HL")      /* 0x2B */ },
    { inc_l,        DASM("INC L")       /* 0x2C */ },
    { dec_l,        DASM("DEC L")       /* 0x2D */ },
    { ld_l_n,       DASM("LD L,#n")     /* 0x2E */ },
    { cpl,          DASM("CPL")         /* 0x2F */ },
    { jr_nc_n,      DASM("JR NC,#e")    /* 0x30 */ },
    { ld_sp_nn,     DASM("LD SP,#nn")   /* 0x31 */ },
    { ld_mnn_a,     DASM("LD (#nn),A")  /* 0x32 */ },
    { inc_sp,       DASM("INC SP")      /* 0x33 */ },
    { inc_mhl,      DASM("INC (HL)")    /* 0x34 */ },
    { dec_mhl,      DASM("DEC (HL)")    /* 0x35 */ },
    { ld_mhl_n,     DASM("LD (HL),#n")  /* 0x36 */ },
    { scf,          DASM("SCF")         /* 0x37 */ },
    { jr_c_n,       DASM("JR C,#e")     /* 0x38 */ },
    { add_hl_sp,    DASM("ADD HL,SP")   /* 0x39 */ },
    { ld_a_mnn,     DASM("LD A,(#nn)")  /* 0x3A */ },
    { dec_sp,       DASM("DEC SP")      /* 0x3B */ },
    { inc_a,        DASM("INC A")       /* 0x3C */ },
    { dec_a,        DASM("DEC A")       /* 0x3D */ },
    { ld_a_n,       DASM("LD A,#n")     /* 0x3E */ },
    { ccf,          DASM("CCF")         /* 0x3F */ },
    { ld_b_b,       DASM("LD B,B")      /* 0x40 */ },
    { ld_b_c,       DASM("LD B,C")      /* 0x41 */ },
    { ld_b_d,       DASM("LD B,D")      /* 0x42 */ },
    { ld_b_e,       DASM("LD B,E")      /* 0x43 */ },
    { ld_b_h,       DASM("LD B,H")      /* 0x44 */ },
    { ld_b_l,       DASM("LD B,L")      /* 0x45 */ },
    { ld_b_mhl,     DASM("LD B,(HL)")   /* 0x46 */ },
    { ld_b_a,       DASM("LD B,A")      /* 0x47 */ },
    { ld_c_b,       DASM("LD C,B")      /* 0x48 */ },
    { ld_c_c,       DASM("LD C,C")      /* 0x49 */ },
    { ld_c_d,       DASM("LD C,D")      /* 0x4A */ },
    { ld_c_e,       DASM("LD C,E")      /* 0x4B */ },
    { ld_c_h,       DASM("LD C,H")      /* 0x4C */ },
    { ld_c_l,       DASM("LD C,L")      /* 0x4D */ },
    { ld_c_mhl,     DASM("LD C,(HL)")   /* 0x4E */ },
    { ld_c_a,       DASM("LD C,A")      /* 0x4F */ },
    { ld_d_b,       DASM("LD D,B")      /* 0x50 */ },
    { ld_d_c,       DASM("LD D,C")      /* 0x51 */ },
    { ld_d_d,       DASM("LD D,D")      /* 0x52 */ },
    { ld_d_e,       DASM("LD D,E")      /* 0x53 */ },
    { ld_d_h,       DASM("LD D,H")      /* 0x54 */ },
    { ld_d_l,       DASM("LD D,L")      /* 0x55 */ },
    { ld_d_mhl,     DASM("LD D,(HL)")   /* 0x56 */ },
    { ld_d_a,       DASM("LD D,A")      /* 0x57 */ },
    { ld_e_b,       DASM("LD E,B")      /* 0x58 */ },
    { ld_e_c,       DASM("LD E,C")      /* 0x59 */ },
    { ld_e_d,       DASM("LD E,D")      /* 0x5A */ },
    { ld_e_e,       DASM("LD E,E")      /* 0x5B */ },
    { ld_e_h,       DASM("LD E,H")      /* 0x5C */ },
    { ld_e_l,       DASM("LD E,L")      /* 0x5D */ },
    { ld_e_mhl,     DASM("LD E,(HL)")   /* 0x5E */ },
    { ld_e_a,       DASM("LD E,A")      /* 0x5F */ },
    { ld_h_b,       DASM("LD H,B")      /* 0x60 */ },
    { ld_h_c,       DASM("LD H,C")      /* 0x61 */ },
    { ld_h_d,       DASM("LD H,D")      /* 0x62 */ },
    { ld_h_e,       DASM("LD H,E")      /* 0x63 */ },
    { ld_h_h,       DASM("LD H,H")      /* 0x64 */ },
    { ld_h_l,       DASM("LD H,L")      /* 0x65 */ },
    { ld_h_mhl,     DASM("LD H,(HL)")   /* 0x66 */ },
    { ld_h_a,       DASM("LD H,A")      /* 0x67 */ },
    { ld_l_b,       DASM("LD L,B")      /* 0x68 */ },
    { ld_l_c,       DASM("LD L,C")      /* 0x69 */ },
    { ld_l_d,       DASM("LD L,D")      /* 0x6A */ },
    { ld_l_e,       DASM("LD L,E")      /* 0x6B */ },
    { ld_l_h,       DASM("LD L,H")      /* 0x6C */ },
    { ld_l_l,       DASM("LD L,L")      /* 0x6D */ },
    { ld_l_mhl,     DASM("LD L,(HL)")   /* 0x6E */ },
    { ld_l_a,       DASM("LD L,A")      /* 0x6F */ },
    { ld_mhl_b,     DASM("LD (HL),B")   /* 0x70 */ },
    { ld_mhl_c,     DASM("LD (HL),C")   /* 0x71 */ },
    { ld_mhl_d,     DASM("LD (HL),D")   /* 0x72 */ },
    { ld_mhl_e,     DASM("LD (HL),E")   /* 0x73 */ },
    { ld_mhl_h,     DASM("LD (HL),H")   /* 0x74 */ },
    { ld_mhl_l,     DASM("LD (HL),L")   /* 0x75 */ },
    { halt,         DASM("HALT")        /* 0x76 */ },
    { ld_mhl_a,     DASM("LD (HL),A")   /* 0x77 */ },
    { ld_a_b,       DASM("LD A,B")      /* 0x78 */ },
    { ld_a_c,       DASM("LD A,C")      /* 0x79 */ },
    { ld_a_d,       DASM("LD A,D")      /* 0x7A */ },
    { ld_a_e,       DASM("LD A,E")      /* 0x7B */ },
    { ld_a_h,       DASM("LD A,H")      /* 0x7C */ },
    { ld_a_l,       DASM("LD A,L")      /* 0x7D */ },
    { ld_a_mhl,     DASM("LD A,(HL)")   /* 0x7E */ },
    { ld_a_a,       DASM("LD A,A")      /* 0x7F */ },
    { add_a_b,      DASM("ADD A,B")     /* 0x80 */ },
    { add_a_c,      DASM("ADD A,C")     /* 0x81 */ },
    { add_a_d,      DASM("ADD A,D")     /* 0x82 */ },
    { add_a_e,      DASM("ADD A,E")     /* 0x83 */ },
    { add_a_h,      DASM("ADD A,H")     /* 0x84 */ },
    { add_a_l,      DASM("ADD A,L")     /* 0x85 */ },
    { add_a_mhl,    DASM("ADD A,(HL)")  /* 0x86 */ },
    { add_a_a,      DASM("ADD A,A")     /* 0x87 */ },
    { adc_a_b,      DASM("ADC A,B")     /* 0x88 */ },
    { adc_a_c,      DASM("ADC A,C")     /* 0x89 */ },
    { adc_a_d,      DASM("ADC A,D")     /* 0x8A */ },
    { adc_a_e,      DASM("ADC A,E")     /* 0x8B */ },
    { adc_a_h,      DASM("ADC A,H")     /* 0x8C */ },
    { adc_a_l,      DASM("ADC A,L")     /* 0x8D */ },
    { adc_a_mhl,    DASM("ADC A,(HL)")  /* 0x8E */ },
    { adc_a_a,      DASM("ADC A,A")     /* 0x8F */ },
    { sub_b,        DASM("SUB B")       /* 0x90 */ },
    { sub_c,        DASM("SUB C")       /* 0x91 */ },
    { sub_d,        DASM("SUB D")       /* 0x92 */ },
    { sub_e,        DASM("SUB E")       /* 0x93 */ },
    { sub_h,        DASM("SUB H")       /* 0x94 */ },
    { sub_l,        DASM("SUB L")       /* 0x95 */ },
    { sub_mhl,      DASM("SUB (HL)")    /* 0x96 */ },
    { sub_a,        DASM("SUB A")       /* 0x97 */ },
    { sbc_a_b,      DASM("SBC A,B")     /* 0x98 */ },
    { sbc_a_c,      DASM("SBC A,C")     /* 0x99 */ },
    { sbc_a_d,      DASM("SBC A,D")     /* 0x9A */ },
    { sbc_a_e,      DASM("SBC A,E")     /* 0x9B */ },
    { sbc_a_h,      DASM("SBC A,H")     /* 0x9C */ },
    { sbc_a_l,      DASM("SBC A,L")     /* 0x9D */ },
    { sbc_a_mhl,    DASM("SBC A,(HL)")  /* 0x9E */ },
    { sbc_a_a,      DASM("SBC A,A")     /* 0x9F */ },
    { and_b,        DASM("AND B")       /* 0xA0 */ },
    { and_c,        DASM("AND C")       /* 0xA1 */ },
    { and_d,        DASM("AND D")       /* 0xA2 */ },
    { and_e,        DASM("AND E")       /* 0xA3 */ },
    { and_h,        DASM("AND H")       /* 0xA4 */ },
    { and_l,        DASM("AND L")       /* 0xA5 */ },
    { and_mhl,      DASM("AND (HL)")    /* 0xA6 */ },
    { and_a,        DASM("AND A")       /* 0xA7 */ },
    { xor_b,        DASM("XOR B")       /* 0xA8 */ },
    { xor_c,        DASM("XOR C")       /* 0xA9 */ },
    { xor_d,        DASM("XOR D")       /* 0xAA */ },
    { xor_e,        DASM("XOR E")       /* 0xAB */ },
    { xor_h,        DASM("XOR H")       /* 0xAC */ },
    { xor_l,        DASM("XOR L")       /* 0xAD */ },
    { xor_mhl,      DASM("XOR (HL)")    /* 0xAE */ },
    { xor_a,        DASM("XOR A")       /* 0xAF */ },
    { or_b,         DASM("OR B")        /* 0xB0 */ },
    { or_c,         DASM("OR C")        /* 0xB1 */ },
    { or_d,         DASM("OR D")        /* 0xB2 */ },
    { or_e,         DASM("OR E")        /* 0xB3 */ },
    { or_h,         DASM("OR H")        /* 0xB4 */ },
    { or_l,         DASM("OR L")        /* 0xB5 */ },
    { or_mhl,       DASM("OR (HL)")     /* 0xB6 */ },
    { or_a,         DASM("OR A")        /* 0xB7 */ },
    { cp_b,         DASM("CP B")        /* 0xB8 */ },
    { cp_c,         DASM("CP C")        /* 0xB9 */ },
    { cp_d,         DASM("CP D")        /* 0xBA */ },
    { cp_e,         DASM("CP E")        /* 0xBB */ },
    { cp_h,         DASM("CP H")        /* 0xBC */ },
    { cp_l,         DASM("CP L")        /* 0xBD */ },
    { cp_mhl,       DASM("CP (HL)")     /* 0xBE */ },
    { cp_a,         DASM("CP A")        /* 0xBF */ },
    { ret_nz,       DASM("RET NZ")      /* 0xC0 */ },
    { pop_bc,       DASM("POP BC")      /* 0xC1 */ },
    { jp_nz_nn,     DASM("JP NZ,#nn")   /* 0xC2 */ },
    { jp_nn,        DASM("JP #nn")      /* 0xC3 */ },
    { call_nz_nn,   DASM("CALL NZ,#nn") /* 0xC4 */ },
    { push_bc,      DASM("PUSH BC")     /* 0xC5 */ },
    { add_a_n,      DASM("ADD A,#n")    /* 0xC6 */ },
    { rst_0,        DASM("RST #00")     /* 0xC7 */ },
    { ret_z,        DASM("RET Z")       /* 0xC8 */ },
    { ret,          DASM("RET")         /* 0xC9 */ },
    { jp_z_nn,      DASM("JP Z,#nn")    /* 0xCA */ },
    { opcode_cb,    NULL                /* 0xCB */ },
    { call_z_nn,    DASM("CALL Z,#nn")  /* 0xCC */ },
    { call_nn,      DASM("CALL #nn")    /* 0xCD */ },
    { adc_a_n,      DASM("ADC A,#n")    /* 0xCE */ },
    { rst_8,        DASM("RST #08")     /* 0xCF */ },
    { ret_nc,       DASM("RET NC")      /* 0xD0 */ },
    { pop_de,       DASM("POP DE")      /* 0xD1 */ },
    { jp_nc_nn,     DASM("JP NC,#nn")   /* 0xD2 */ },
    { out_n_a,      DASM("OUT (#n),A")  /* 0xD3 */ },
    { call_nc_nn,   DASM("CALL NC,#nn") /* 0xD4 */ },
    { push_de,      DASM("PUSH DE")     /* 0xD5 */ },
    { sub_n,        DASM("SUB #n")      /* 0xD6 */ },
    { rst_10,       DASM("RST #10")     /* 0xD7 */ },
    { ret_c,        DASM("RET C")       /* 0xD8 */ },
    { exx,          DASM("EXX")         /* 0xD9 */ },
    { jp_c_nn,      DASM("JP C,#nn")    /* 0xDA */ },
    { in_a_n,       DASM("IN A,(#n)")   /* 0xDB */ },
    { call_c_nn,    DASM("CALL C,#nn")  /* 0xDC */ },
    { opcode_dd,    NULL                /* 0xDD */ },
    { sbc_a_n,      DASM("SBC A,#n")    /* 0xDE */ },
    { rst_18,       DASM("RST #18")     /* 0xDF */ },
    { ret_po,       DASM("RET PO")      /* 0xE0 */ },
    { pop_hl,       DASM("POP HL")      /* 0xE1 */ },
    { jp_po_nn,     DASM("JP PO,#nn")   /* 0xE2 */ },
    { ex_msp_hl,    DASM("EX (SP),HL")  /* 0xE3 */ },
    { call_po_nn,   DASM("CALL PO,#nn") /* 0xE4 */ },
    { push_hl,      DASM("PUSH HL")     /* 0xE5 */ },
    { and_n,        DASM("AND #n")      /* 0xE6 */ },
    { rst_20,       DASM("RST #20")     /* 0xE7 */ },
    { ret_pe,       DASM("RET PE")      /* 0xE8 */ },
    { jp_hl,        DASM("JP HL")       /* 0xE9 */ },
    { jp_pe_nn,     DASM("JP PE,#nn")   /* 0xEA */ },
    { ex_de_hl,     DASM("EX DE,HL")    /* 0xEB */ },
    { call_pe_nn,   DASM("CALL PE,#nn") /* 0xEC */ },
    { opcode_ed,    NULL                /* 0xED */ },
    { xor_n,        DASM("XOR #n")      /* 0xEE */ },
    { rst_28,       DASM("RST #28")     /* 0xEF */ },
    { ret_p,        DASM("RET P")       /* 0xF0 */ },
    { pop_af,       DASM("POP AF")      /* 0xF1 */ },
    { jp_p_nn,      DASM("JP P,#nn")    /* 0xF2 */ },
    { di,           DASM("DI")          /* 0xF3 */ },
    { call_p_nn,    DASM("CALL P,#nn")  /* 0xF4 */ },
    { push_af,      DASM("PUSH AF")     /* 0xF5 */ },
    { or_n,         DASM("OR #n")       /* 0xF6 */ },
    { rst_30,       DASM("RST #30")     /* 0xF7 */ },
    { ret_m,        DASM("RET M")       /* 0xF8 */ },
    { ld_sp_hl,     DASM("LD SP,HL")    /* 0xF9 */ },
    { jp_m_nn,      DASM("JP M,#nn")    /* 0xFA */ },
    { ei,           DASM("EI")          /* 0xFB */ },
    { call_m_nn,    DASM("CALL M,#nn")  /* 0xFC */ },
    { opcode_fd,    NULL                /* 0xFD */ },
    { cp_n,         DASM("CP #n")       /* 0xFE */ },
    { rst_38,       DASM("RST #38")     /* 0xFF */ }
};


cpuZ80 *cpuZ80_create(void *sms, readmemory_handler readmem, writememory_handler writemem, readio_handler readio, writeio_handler writeio)
{
    cpuZ80 *cpu = malloc(sizeof(cpuZ80));
    if(cpu==NULL) {
        log4me_error(LOG_EMU_Z80, "Unable to allocate the memory block.\n");
        exit(EXIT_FAILURE);
    }

    cpu->param = sms;
    cpu->readmem = readmem;
    cpu->writemem = writemem;
    cpu->readio = readio;
    cpu->writeio = writeio;

    return cpu;
}

void cpuZ80_free(cpuZ80 *cpu)
{
    if(cpu) {
        free(cpu);
    }
}

void cpuZ80_reset(cpuZ80 *cpu)
{
    cpu->w.AF = cpu->w.BC = cpu->w.DE = cpu->w.HL = 0;
    cpu->wp.AFp = cpu->wp.BCp = cpu->wp.DEp = cpu->wp.HLp = 0;
    cpu->IX = cpu->IY = cpu->SP = cpu->PC = 0;
    cpu->I = cpu->R = cpu->R7 = cpu->IM = cpu->IFF1 = cpu->IFF2 = 0;

    cpu->halted = 0;
    cpu->cycles = 0;
}

int cpuZ80_step(cpuZ80 *cpu)
{
    word pc = cpu->PC;

    cpu->cycles = 0;

    int op = (int)read8(cpu, cpu->PC++);

    if(opcodes[op].func==NULL) {
        log4me_error(LOG_EMU_Z80, "0x%04X : opcode = 0x%02X not implemented\n", pc, op);
        exit(EXIT_FAILURE);
    }

    cpu->R++;
    opcodes[op].func(cpu);

    return cpu->cycles;
}

int cpuZ80_int(cpuZ80 *cpu)
{
    cpu->cycles = 0;

    if(cpu->IFF1!=0) {

        if(cpu->halted) {
            cpu->halted = 0;
            cpu->PC++;
        }

        cpu->IFF1 = cpu->IFF2 = 0;

        switch(cpu->IM) {
            case 0 :
                //cpu->cycles += 2; // penality
                //cpu->cycles += cpuZ80_step(cpu);
                log4me_error(LOG_EMU_Z80, "interrupt mode 0 not implemented\n");
                exit(EXIT_FAILURE);
                break;
            case 1 :
                cpu->cycles += 2; // penality
                cpu->R++;
                RST(0x38);
                break;
            default :
                log4me_error(LOG_EMU_Z80, "interrupt mode 2 not implemented\n");
                exit(EXIT_FAILURE);
                break;
        }
    }

    return cpu->cycles;
}

int cpuZ80_nmi(cpuZ80 *cpu)
{
    if(cpu->halted) {
        cpu->halted = 0;
        cpu->PC++;
    }
    cpu->cycles = 11;

    cpu->R++;
    cpu->IFF1 = 0;
    PUSH(cpu->PC);
    cpu->PC = 0x66;

    return cpu->cycles;
}

word cpuZ80_getAF(cpuZ80 *cpu) { return cpu->w.AF; }
word cpuZ80_getBC(cpuZ80 *cpu) { return cpu->w.BC; }
word cpuZ80_getDE(cpuZ80 *cpu) { return cpu->w.DE; }
word cpuZ80_getHL(cpuZ80 *cpu) { return cpu->w.HL; }
word cpuZ80_getAF_(cpuZ80 *cpu) { return cpu->wp.AFp; }
word cpuZ80_getBC_(cpuZ80 *cpu) { return cpu->wp.BCp; }
word cpuZ80_getDE_(cpuZ80 *cpu) { return cpu->wp.DEp; }
word cpuZ80_getHL_(cpuZ80 *cpu) { return cpu->wp.HLp; }
word cpuZ80_getIX(cpuZ80 *cpu) { return cpu->IX; }
word cpuZ80_getIY(cpuZ80 *cpu) { return cpu->IY; }
word cpuZ80_getPC(cpuZ80 *cpu) { return cpu->PC; }
word cpuZ80_getSP(cpuZ80 *cpu) { return cpu->SP; }
word cpuZ80_getI(cpuZ80 *cpu) { return cpu->I; }
word cpuZ80_getR(cpuZ80 *cpu) { return cpu->R; }
word cpuZ80_getR7(cpuZ80 *cpu) { return cpu->R7; }
word cpuZ80_getIM(cpuZ80 *cpu) { return cpu->IM; }
word cpuZ80_getIFF1(cpuZ80 *cpu) { return cpu->IFF1; }
word cpuZ80_getIFF2(cpuZ80 *cpu) { return cpu->IFF2; }

void cpuZ80_setAF(cpuZ80 *cpu, dword value) { cpu->w.AF = value; }
void cpuZ80_setBC(cpuZ80 *cpu, dword value) { cpu->w.BC = value; }
void cpuZ80_setDE(cpuZ80 *cpu, dword value) { cpu->w.DE = value; }
void cpuZ80_setHL(cpuZ80 *cpu, dword value) { cpu->w.HL = value; }
void cpuZ80_setAF_(cpuZ80 *cpu, dword value) { cpu->wp.AFp = value; }
void cpuZ80_setBC_(cpuZ80 *cpu, dword value) { cpu->wp.BCp = value; }
void cpuZ80_setDE_(cpuZ80 *cpu, dword value) { cpu->wp.DEp = value; }
void cpuZ80_setHL_(cpuZ80 *cpu, dword value) { cpu->wp.HLp = value; }
void cpuZ80_setIX(cpuZ80 *cpu, dword value) { cpu->IX = value; }
void cpuZ80_setIY(cpuZ80 *cpu, dword value) { cpu->IY = value; }
void cpuZ80_setPC(cpuZ80 *cpu, dword value) { cpu->PC = value; }
void cpuZ80_setSP(cpuZ80 *cpu, dword value) { cpu->SP = value; }
void cpuZ80_setI(cpuZ80 *cpu, dword value) { cpu->I = value; }
void cpuZ80_setR(cpuZ80 *cpu, dword value) { cpu->R = value; }
void cpuZ80_setR7(cpuZ80 *cpu, dword value) { cpu->R7 = value; }
void cpuZ80_setIM(cpuZ80 *cpu, dword value) { cpu->IM = value; }
void cpuZ80_setIFF1(cpuZ80 *cpu, dword value) { cpu->IFF1 = value; }
void cpuZ80_setIFF2(cpuZ80 *cpu, dword value) { cpu->IFF2 = value; }

int cpuZ80_is_halted(cpuZ80 *cpu) { return cpu->halted; }

int cpuZ80_int_accepted(cpuZ80 *cpu)
{
    return cpu->IFF1;
}

#ifdef DEBUG
void dasmopcode_cb(cpuZ80 *cpu, word addr, char *buffer, int len);
void dasmopcode_dd(cpuZ80 *cpu, word addr, char *buffer, int len);
void dasmopcode_ed(cpuZ80 *cpu, word addr, char *buffer, int len);
void dasmopcode_fd(cpuZ80 *cpu, word addr, char *buffer, int len);

void cpuZ80_dasm(cpuZ80 *cpu, word addr, char *buffer, int len)
{
    int op = (int)read8(cpu, addr);

    assert(buffer!=NULL);
    buffer[0] = 0;

    if(opcodes[op].func==opcode_cb)
        dasmopcode_cb(cpu, addr+1, buffer, len);
    else
    if(opcodes[op].func==opcode_dd)
        dasmopcode_dd(cpu, addr+1, buffer, len);
    else
    if(opcodes[op].func==opcode_ed)
        dasmopcode_ed(cpu, addr+1, buffer, len);
    else
    if(opcodes[op].func==opcode_fd)
        dasmopcode_fd(cpu, addr+1, buffer, len);
    else
        dasmopcode(cpu, opcodes[op].dasm, addr+1, buffer, len);
}
#endif


#define XML_WRITE_CPU_BYTE_REG(w,reg,value) { \
            xmlTextWriterStartElement(w, BAD_CAST "register"); \
            xmlTextWriterWriteAttribute(w, BAD_CAST "name", BAD_CAST reg); \
            xmlTextWriterWriteFormatString(w, "%02X", value); \
            xmlTextWriterEndElement(w); }

#define XML_WRITE_CPU_WORD_REG(w,reg,value) { \
            xmlTextWriterStartElement(w, BAD_CAST "register"); \
            xmlTextWriterWriteAttribute(w, BAD_CAST "name", BAD_CAST reg); \
            xmlTextWriterWriteFormatString(w, "%04X", value); \
            xmlTextWriterEndElement(w); }

void cpuZ80_takesnapshot(cpuZ80 *cpu, xmlTextWriterPtr writer)
{
    xmlTextWriterStartElement(writer, BAD_CAST "z80");
        xmlTextWriterStartElement(writer, BAD_CAST "registers");
            XML_WRITE_CPU_WORD_REG(writer, "AF", cpuZ80_getAF(cpu));
            XML_WRITE_CPU_WORD_REG(writer, "BC", cpuZ80_getBC(cpu));
            XML_WRITE_CPU_WORD_REG(writer, "DE", cpuZ80_getDE(cpu));
            XML_WRITE_CPU_WORD_REG(writer, "HL", cpuZ80_getHL(cpu));

            XML_WRITE_CPU_WORD_REG(writer, "AFp", cpuZ80_getAF_(cpu));
            XML_WRITE_CPU_WORD_REG(writer, "BCp", cpuZ80_getBC_(cpu));
            XML_WRITE_CPU_WORD_REG(writer, "DEp", cpuZ80_getDE_(cpu));
            XML_WRITE_CPU_WORD_REG(writer, "HLp", cpuZ80_getHL_(cpu));

            XML_WRITE_CPU_WORD_REG(writer, "IX", cpuZ80_getIX(cpu));
            XML_WRITE_CPU_WORD_REG(writer, "IY", cpuZ80_getIY(cpu));
            XML_WRITE_CPU_WORD_REG(writer, "PC", cpuZ80_getPC(cpu));
            XML_WRITE_CPU_WORD_REG(writer, "SP", cpuZ80_getSP(cpu));

            XML_WRITE_CPU_BYTE_REG(writer, "I", cpuZ80_getI(cpu));
            XML_WRITE_CPU_BYTE_REG(writer, "R", cpuZ80_getR(cpu));
            XML_WRITE_CPU_BYTE_REG(writer, "R7", cpuZ80_getR7(cpu));
            XML_WRITE_CPU_BYTE_REG(writer, "IM", cpuZ80_getIM(cpu));
            XML_WRITE_CPU_BYTE_REG(writer, "IFF1", cpuZ80_getIFF1(cpu));
            XML_WRITE_CPU_BYTE_REG(writer, "IFF2", cpuZ80_getIFF2(cpu));
        xmlTextWriterEndElement(writer); /* registers */
    xmlTextWriterEndElement(writer); /* z80 */
}

void cpuZ80_loadsnapshot(cpuZ80 *cpu, xmlNode *cpunode)
{
    XML_ENUM_CHILD(cpunode, node,
        XML_ELEMENT_ENUM_CHILD("registers", node, child,
            XML_ELEMENT_CONTENT("register", child,
                xmlChar *name = xmlGetProp(child, BAD_CAST "name");
                if(xmlStrCaseCmp(name, "AF"))
                    cpuZ80_setAF(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "BC"))
                    cpuZ80_setBC(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "DE"))
                    cpuZ80_setDE(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "HL"))
                    cpuZ80_setHL(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "AFp"))
                    cpuZ80_setAF_(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "BCp"))
                    cpuZ80_setBC_(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "DEp"))
                    cpuZ80_setDE_(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "HLp"))
                    cpuZ80_setHL_(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "IX"))
                    cpuZ80_setIX(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "IY"))
                    cpuZ80_setIY(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "PC"))
                    cpuZ80_setPC(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "SP"))
                    cpuZ80_setSP(cpu, (word)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "I"))
                    cpuZ80_setI(cpu, (byte)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "R"))
                    cpuZ80_setR(cpu, (byte)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "R7"))
                    cpuZ80_setR7(cpu, (byte)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "IM"))
                    cpuZ80_setIM(cpu, (byte)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "IFF1"))
                    cpuZ80_setIFF1(cpu, (byte)strtoul((const char*)content, NULL, 16));
                else
                if(xmlStrCaseCmp(name, "IFF2"))
                    cpuZ80_setIFF2(cpu, (byte)strtoul((const char*)content, NULL, 16));
            )
        )
    )
}
