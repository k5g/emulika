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

static void rlc_b(cpuZ80 *cpu) /* 0xCB 0x00 */
{
    cpu->cycles += 8;
    cpu->b.B = rlc8(cpu, cpu->b.B);
}

static void rlc_c(cpuZ80 *cpu) /* 0xCB 0x01 */
{
    cpu->cycles += 8;
    cpu->b.C = rlc8(cpu, cpu->b.C);
}

static void rlc_d(cpuZ80 *cpu) /* 0xCB 0x02 */
{
    cpu->cycles += 8;
    cpu->b.D = rlc8(cpu, cpu->b.D);
}

static void rlc_e(cpuZ80 *cpu) /* 0xCB 0x03 */
{
    cpu->cycles += 8;
    cpu->b.E = rlc8(cpu, cpu->b.E);
}

static void rlc_h(cpuZ80 *cpu) /* 0xCB 0x04 */
{
    cpu->cycles += 8;
    cpu->b.H = rlc8(cpu, cpu->b.H);
}

static void rlc_l(cpuZ80 *cpu) /* 0xCB 0x05 */
{
    cpu->cycles += 8;
    cpu->b.L = rlc8(cpu, cpu->b.L);
}

static void rlc_mhl(cpuZ80 *cpu) /* 0xCB 0x06 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, rlc8(cpu, read8(cpu, cpu->w.HL)));
}

static void rlc_a(cpuZ80 *cpu) /* 0xCB 0x07 */
{
    cpu->cycles += 8;
    cpu->b.A = rlc8(cpu, cpu->b.A);
}

static void rrc_b(cpuZ80 *cpu) /* 0xCB 0x08 */
{
    cpu->cycles += 8;
    cpu->b.B = rrc8(cpu, cpu->b.B);
}

static void rrc_c(cpuZ80 *cpu) /* 0xCB 0x09 */
{
    cpu->cycles += 8;
    cpu->b.C = rrc8(cpu, cpu->b.C);
}

static void rrc_d(cpuZ80 *cpu) /* 0xCB 0x0A */
{
    cpu->cycles += 8;
    cpu->b.D = rrc8(cpu, cpu->b.D);
}

static void rrc_e(cpuZ80 *cpu) /* 0xCB 0x0B */
{
    cpu->cycles += 8;
    cpu->b.E = rrc8(cpu, cpu->b.E);
}

static void rrc_h(cpuZ80 *cpu) /* 0xCB 0x0C */
{
    cpu->cycles += 8;
    cpu->b.H = rrc8(cpu, cpu->b.H);
}

static void rrc_l(cpuZ80 *cpu) /* 0xCB 0x0D */
{
    cpu->cycles += 8;
    cpu->b.L = rrc8(cpu, cpu->b.L);
}

static void rrc_mhl(cpuZ80 *cpu) /* 0xCB 0x0E */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, rrc8(cpu, read8(cpu, cpu->w.HL)));
}

static void rrc_a(cpuZ80 *cpu) /* 0xCB 0x0F */
{
    cpu->cycles += 8;
    cpu->b.A = rrc8(cpu, cpu->b.A);
}

static void rl_b(cpuZ80 *cpu) /* 0xCB 0x10 */
{
    cpu->cycles += 8;
    cpu->b.B = rl8(cpu, cpu->b.B);
}

static void rl_c(cpuZ80 *cpu) /* 0xCB 0x11 */
{
    cpu->cycles += 8;
    cpu->b.C = rl8(cpu, cpu->b.C);
}

static void rl_d(cpuZ80 *cpu) /* 0xCB 0x12 */
{
    cpu->cycles += 8;
    cpu->b.D = rl8(cpu, cpu->b.D);
}

static void rl_e(cpuZ80 *cpu) /* 0xCB 0x13 */
{
    cpu->cycles += 8;
    cpu->b.E = rl8(cpu, cpu->b.E);
}

static void rl_h(cpuZ80 *cpu) /* 0xCB 0x14 */
{
    cpu->cycles += 8;
    cpu->b.H = rl8(cpu, cpu->b.H);
}

static void rl_l(cpuZ80 *cpu) /* 0xCB 0x15 */
{
    cpu->cycles += 8;
    cpu->b.L = rl8(cpu, cpu->b.L);
}

static void rl_mhl(cpuZ80 *cpu) /* 0xCB 0x16 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, rl8(cpu, read8(cpu, cpu->w.HL)));
}

static void rl_a(cpuZ80 *cpu) /* 0xCB 0x17 */
{
    cpu->cycles += 8;
    cpu->b.A = rl8(cpu, cpu->b.A);
}

static void rr_b(cpuZ80 *cpu) /* 0xCB 0x18 */
{
    cpu->cycles += 8;
    cpu->b.B = rr8(cpu, cpu->b.B);
}

static void rr_c(cpuZ80 *cpu) /* 0xCB 0x19 */
{
    cpu->cycles += 8;
    cpu->b.C = rr8(cpu, cpu->b.C);
}

static void rr_d(cpuZ80 *cpu) /* 0xCB 0x1A */
{
    cpu->cycles += 8;
    cpu->b.D = rr8(cpu, cpu->b.D);
}

static void rr_e(cpuZ80 *cpu) /* 0xCB 0x1B */
{
    cpu->cycles += 8;
    cpu->b.E = rr8(cpu, cpu->b.E);
}

static void rr_h(cpuZ80 *cpu) /* 0xCB 0x1C */
{
    cpu->cycles += 8;
    cpu->b.H = rr8(cpu, cpu->b.H);
}

static void rr_l(cpuZ80 *cpu) /* 0xCB 0x1D */
{
    cpu->cycles += 8;
    cpu->b.L = rr8(cpu, cpu->b.L);
}

static void rr_mhl(cpuZ80 *cpu) /* 0xCB 0x1E */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, rr8(cpu, read8(cpu, cpu->w.HL)));
}

static void rr_a(cpuZ80 *cpu) /* 0xCB 0x1F */
{
    cpu->cycles += 8;
    cpu->b.A = rr8(cpu, cpu->b.A);
}

static void sla_b(cpuZ80 *cpu) /* 0xCB 0x20 */
{
    cpu->cycles += 8;
    cpu->b.B = sla8(cpu, cpu->b.B);
}

static void sla_c(cpuZ80 *cpu) /* 0xCB 0x21 */
{
    cpu->cycles += 8;
    cpu->b.C = sla8(cpu, cpu->b.C);
}

static void sla_d(cpuZ80 *cpu) /* 0xCB 0x22 */
{
    cpu->cycles += 8;
    cpu->b.D = sla8(cpu, cpu->b.D);
}

static void sla_e(cpuZ80 *cpu) /* 0xCB 0x23 */
{
    cpu->cycles += 8;
    cpu->b.E = sla8(cpu, cpu->b.E);
}

static void sla_h(cpuZ80 *cpu) /* 0xCB 0x24 */
{
    cpu->cycles += 8;
    cpu->b.H = sla8(cpu, cpu->b.H);
}

static void sla_l(cpuZ80 *cpu) /* 0xCB 0x25 */
{
    cpu->cycles += 8;
    cpu->b.L = sla8(cpu, cpu->b.L);
}

static void sla_mhl(cpuZ80 *cpu) /* 0xCB 0x26 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, sla8(cpu, read8(cpu, cpu->w.HL)));
}

static void sla_a(cpuZ80 *cpu) /* 0xCB 0x27 */
{
    cpu->cycles += 8;
    cpu->b.A = sla8(cpu, cpu->b.A);
}

static void sra_b(cpuZ80 *cpu) /* 0xCB 0x28 */
{
    cpu->cycles += 8;
    cpu->b.B = sra8(cpu, cpu->b.B);
}

static void sra_c(cpuZ80 *cpu) /* 0xCB 0x29 */
{
    cpu->cycles += 8;
    cpu->b.C = sra8(cpu, cpu->b.C);
}

static void sra_d(cpuZ80 *cpu) /* 0xCB 0x2A */
{
    cpu->cycles += 8;
    cpu->b.D = sra8(cpu, cpu->b.D);
}

static void sra_e(cpuZ80 *cpu) /* 0xCB 0x2B */
{
    cpu->cycles += 8;
    cpu->b.E = sra8(cpu, cpu->b.E);
}

static void sra_h(cpuZ80 *cpu) /* 0xCB 0x2C */
{
    cpu->cycles += 8;
    cpu->b.H = sra8(cpu, cpu->b.H);
}

static void sra_l(cpuZ80 *cpu) /* 0xCB 0x2D */
{
    cpu->cycles += 8;
    cpu->b.L = sra8(cpu, cpu->b.L);
}

static void sra_mhl(cpuZ80 *cpu) /* 0xCB 0x2E */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, sra8(cpu, read8(cpu, cpu->w.HL)));
}

static void sra_a(cpuZ80 *cpu) /* 0xCB 0x2F */
{
    cpu->cycles += 8;
    cpu->b.A = sra8(cpu, cpu->b.A);
}

static void sll_b(cpuZ80 *cpu) /* 0xCB 0x30 */
{
    cpu->cycles += 8;
    cpu->b.B = sll8(cpu, cpu->b.B);
}

static void sll_c(cpuZ80 *cpu) /* 0xCB 0x31 */
{
    cpu->cycles += 8;
    cpu->b.C = sll8(cpu, cpu->b.C);
}

static void sll_d(cpuZ80 *cpu) /* 0xCB 0x32 */
{
    cpu->cycles += 8;
    cpu->b.D = sll8(cpu, cpu->b.D);
}

static void sll_e(cpuZ80 *cpu) /* 0xCB 0x33 */
{
    cpu->cycles += 8;
    cpu->b.E = sll8(cpu, cpu->b.E);
}

static void sll_h(cpuZ80 *cpu) /* 0xCB 0x34 */
{
    cpu->cycles += 8;
    cpu->b.H = sll8(cpu, cpu->b.H);
}

static void sll_l(cpuZ80 *cpu) /* 0xCB 0x35 */
{
    cpu->cycles += 8;
    cpu->b.L = sll8(cpu, cpu->b.L);
}

static void sll_mhl(cpuZ80 *cpu) /* 0xCB 0x36 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, sll8(cpu, read8(cpu, cpu->w.HL)));
}

static void sll_a(cpuZ80 *cpu) /* 0xCB 0x37 */
{
    cpu->cycles += 8;
    cpu->b.A = sll8(cpu, cpu->b.A);
}

static void srl_b(cpuZ80 *cpu) /* 0xCB 0x38 */
{
    cpu->cycles += 8;
    cpu->b.B = srl8(cpu, cpu->b.B);
}

static void srl_c(cpuZ80 *cpu) /* 0xCB 0x39 */
{
    cpu->cycles += 8;
    cpu->b.C = srl8(cpu, cpu->b.C);
}

static void srl_d(cpuZ80 *cpu) /* 0xCB 0x3A */
{
    cpu->cycles += 8;
    cpu->b.D = srl8(cpu, cpu->b.D);
}

static void srl_e(cpuZ80 *cpu) /* 0xCB 0x3B */
{
    cpu->cycles += 8;
    cpu->b.E = srl8(cpu, cpu->b.E);
}

static void srl_h(cpuZ80 *cpu) /* 0xCB 0x3C */
{
    cpu->cycles += 8;
    cpu->b.H = srl8(cpu, cpu->b.H);
}

static void srl_l(cpuZ80 *cpu) /* 0xCB 0x3D */
{
    cpu->cycles += 8;
    cpu->b.L = srl8(cpu, cpu->b.L);
}

static void srl_mhl(cpuZ80 *cpu) /* 0xCB 0x3E */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, srl8(cpu, read8(cpu, cpu->w.HL)));
}

static void srl_a(cpuZ80 *cpu) /* 0xCB 0x3F */
{
    cpu->cycles += 8;
    cpu->b.A = srl8(cpu, cpu->b.A);
}

static void bit_0_b(cpuZ80 *cpu) /* 0xCB 0x40 */
{
    cpu->cycles += 8;
    bit8(cpu, 0, cpu->b.B);
}

static void bit_0_c(cpuZ80 *cpu) /* 0xCB 0x41 */
{
    cpu->cycles += 8;
    bit8(cpu, 0, cpu->b.C);
}

static void bit_0_d(cpuZ80 *cpu) /* 0xCB 0x42 */
{
    cpu->cycles += 8;
    bit8(cpu, 0, cpu->b.D);
}

static void bit_0_e(cpuZ80 *cpu) /* 0xCB 0x43 */
{
    cpu->cycles += 8;
    bit8(cpu, 0, cpu->b.E);
}

static void bit_0_h(cpuZ80 *cpu) /* 0xCB 0x44 */
{
    cpu->cycles += 8;
    bit8(cpu, 0, cpu->b.H);
}

static void bit_0_l(cpuZ80 *cpu) /* 0xCB 0x45 */
{
    cpu->cycles += 8;
    bit8(cpu, 0, cpu->b.L);
}

static void bit_0_mhl(cpuZ80 *cpu) /* 0xCB 0x46 */
{
    cpu->cycles += 12;
    bit8(cpu, 0, read8(cpu, cpu->w.HL));
}

static void bit_0_a(cpuZ80 *cpu) /* 0xCB 0x47 */
{
    cpu->cycles += 8;
    bit8(cpu, 0, cpu->b.A);
}

static void bit_1_b(cpuZ80 *cpu) /* 0xCB 0x48 */
{
    cpu->cycles += 8;
    bit8(cpu, 1, cpu->b.B);
}

static void bit_1_c(cpuZ80 *cpu) /* 0xCB 0x49 */
{
    cpu->cycles += 8;
    bit8(cpu, 1, cpu->b.C);
}

static void bit_1_d(cpuZ80 *cpu) /* 0xCB 0x4A */
{
    cpu->cycles += 8;
    bit8(cpu, 1, cpu->b.D);
}

static void bit_1_e(cpuZ80 *cpu) /* 0xCB 0x4B */
{
    cpu->cycles += 8;
    bit8(cpu, 1, cpu->b.E);
}

static void bit_1_h(cpuZ80 *cpu) /* 0xCB 0x4C */
{
    cpu->cycles += 8;
    bit8(cpu, 1, cpu->b.H);
}

static void bit_1_l(cpuZ80 *cpu) /* 0xCB 0x4D */
{
    cpu->cycles += 8;
    bit8(cpu, 1, cpu->b.L);
}

static void bit_1_mhl(cpuZ80 *cpu) /* 0xCB 0x4E */
{
    cpu->cycles += 12;
    bit8(cpu, 1, read8(cpu, cpu->w.HL));
}

static void bit_1_a(cpuZ80 *cpu) /* 0xCB 0x4F */
{
    cpu->cycles += 8;
    bit8(cpu, 1, cpu->b.A);
}

static void bit_2_b(cpuZ80 *cpu) /* 0xCB 0x50 */
{
    cpu->cycles += 8;
    bit8(cpu, 2, cpu->b.B);
}

static void bit_2_c(cpuZ80 *cpu) /* 0xCB 0x51 */
{
    cpu->cycles += 8;
    bit8(cpu, 2, cpu->b.C);
}

static void bit_2_d(cpuZ80 *cpu) /* 0xCB 0x52 */
{
    cpu->cycles += 8;
    bit8(cpu, 2, cpu->b.D);
}

static void bit_2_e(cpuZ80 *cpu) /* 0xCB 0x53 */
{
    cpu->cycles += 8;
    bit8(cpu, 2, cpu->b.E);
}

static void bit_2_h(cpuZ80 *cpu) /* 0xCB 0x54 */
{
    cpu->cycles += 8;
    bit8(cpu, 2, cpu->b.H);
}

static void bit_2_l(cpuZ80 *cpu) /* 0xCB 0x55 */
{
    cpu->cycles += 8;
    bit8(cpu, 2, cpu->b.L);
}

static void bit_2_mhl(cpuZ80 *cpu) /* 0xCB 0x56 */
{
    cpu->cycles += 12;
    bit8(cpu, 2, read8(cpu, cpu->w.HL));
}

static void bit_2_a(cpuZ80 *cpu) /* 0xCB 0x57 */
{
    cpu->cycles += 8;
    bit8(cpu, 2, cpu->b.A);
}

static void bit_3_b(cpuZ80 *cpu) /* 0xCB 0x58 */
{
    cpu->cycles += 8;
    bit8(cpu, 3, cpu->b.B);
}

static void bit_3_c(cpuZ80 *cpu) /* 0xCB 0x59 */
{
    cpu->cycles += 8;
    bit8(cpu, 3, cpu->b.C);
}

static void bit_3_d(cpuZ80 *cpu) /* 0xCB 0x5A */
{
    cpu->cycles += 8;
    bit8(cpu, 3, cpu->b.D);
}

static void bit_3_e(cpuZ80 *cpu) /* 0xCB 0x5B */
{
    cpu->cycles += 8;
    bit8(cpu, 3, cpu->b.E);
}

static void bit_3_h(cpuZ80 *cpu) /* 0xCB 0x5C */
{
    cpu->cycles += 8;
    bit8(cpu, 3, cpu->b.H);
}

static void bit_3_l(cpuZ80 *cpu) /* 0xCB 0x5D */
{
    cpu->cycles += 8;
    bit8(cpu, 3, cpu->b.L);
}

static void bit_3_mhl(cpuZ80 *cpu) /* 0xCB 0x5E */
{
    cpu->cycles += 12;
    bit8(cpu, 3, read8(cpu, cpu->w.HL));
}

static void bit_3_a(cpuZ80 *cpu) /* 0xCB 0x5F */
{
    cpu->cycles += 8;
    bit8(cpu, 3, cpu->b.A);
}

static void bit_4_b(cpuZ80 *cpu) /* 0xCB 0x60 */
{
    cpu->cycles += 8;
    bit8(cpu, 4, cpu->b.B);
}

static void bit_4_c(cpuZ80 *cpu) /* 0xCB 0x61 */
{
    cpu->cycles += 8;
    bit8(cpu, 4, cpu->b.C);
}

static void bit_4_d(cpuZ80 *cpu) /* 0xCB 0x62 */
{
    cpu->cycles += 8;
    bit8(cpu, 4, cpu->b.D);
}

static void bit_4_e(cpuZ80 *cpu) /* 0xCB 0x63 */
{
    cpu->cycles += 8;
    bit8(cpu, 4, cpu->b.E);
}

static void bit_4_h(cpuZ80 *cpu) /* 0xCB 0x64 */
{
    cpu->cycles += 8;
    bit8(cpu, 4, cpu->b.H);
}

static void bit_4_l(cpuZ80 *cpu) /* 0xCB 0x65 */
{
    cpu->cycles += 8;
    bit8(cpu, 4, cpu->b.L);
}

static void bit_4_mhl(cpuZ80 *cpu) /* 0xCB 0x66 */
{
    cpu->cycles += 12;
    bit8(cpu, 4, read8(cpu, cpu->w.HL));
}

static void bit_4_a(cpuZ80 *cpu) /* 0xCB 0x67 */
{
    cpu->cycles += 8;
    bit8(cpu, 4, cpu->b.A);
}

static void bit_5_b(cpuZ80 *cpu) /* 0xCB 0x68 */
{
    cpu->cycles += 8;
    bit8(cpu, 5, cpu->b.B);
}

static void bit_5_c(cpuZ80 *cpu) /* 0xCB 0x69 */
{
    cpu->cycles += 8;
    bit8(cpu, 5, cpu->b.C);
}

static void bit_5_d(cpuZ80 *cpu) /* 0xCB 0x6A */
{
    cpu->cycles += 8;
    bit8(cpu, 5, cpu->b.D);
}

static void bit_5_e(cpuZ80 *cpu) /* 0xCB 0x6B */
{
    cpu->cycles += 8;
    bit8(cpu, 5, cpu->b.E);
}

static void bit_5_h(cpuZ80 *cpu) /* 0xCB 0x6C */
{
    cpu->cycles += 8;
    bit8(cpu, 5, cpu->b.H);
}

static void bit_5_l(cpuZ80 *cpu) /* 0xCB 0x6D */
{
    cpu->cycles += 8;
    bit8(cpu, 5, cpu->b.L);
}

static void bit_5_mhl(cpuZ80 *cpu) /* 0xCB 0x6E */
{
    cpu->cycles += 12;
    bit8(cpu, 5, read8(cpu, cpu->w.HL));
}

static void bit_5_a(cpuZ80 *cpu) /* 0xCB 0x6F */
{
    cpu->cycles += 8;
    bit8(cpu, 5, cpu->b.A);
}

static void bit_6_b(cpuZ80 *cpu) /* 0xCB 0x70 */
{
    cpu->cycles += 8;
    bit8(cpu, 6, cpu->b.B);
}

static void bit_6_c(cpuZ80 *cpu) /* 0xCB 0x71 */
{
    cpu->cycles += 8;
    bit8(cpu, 6, cpu->b.C);
}

static void bit_6_d(cpuZ80 *cpu) /* 0xCB 0x72 */
{
    cpu->cycles += 8;
    bit8(cpu, 6, cpu->b.D);
}

static void bit_6_e(cpuZ80 *cpu) /* 0xCB 0x73 */
{
    cpu->cycles += 8;
    bit8(cpu, 6, cpu->b.E);
}

static void bit_6_h(cpuZ80 *cpu) /* 0xCB 0x74 */
{
    cpu->cycles += 8;
    bit8(cpu, 6, cpu->b.H);
}

static void bit_6_l(cpuZ80 *cpu) /* 0xCB 0x75 */
{
    cpu->cycles += 8;
    bit8(cpu, 6, cpu->b.L);
}

static void bit_6_mhl(cpuZ80 *cpu) /* 0xCB 0x76 */
{
    cpu->cycles += 12;
    bit8(cpu, 6, read8(cpu, cpu->w.HL));
}

static void bit_6_a(cpuZ80 *cpu) /* 0xCB 0x77 */
{
    cpu->cycles += 8;
    bit8(cpu, 6, cpu->b.A);
}

static void bit_7_b(cpuZ80 *cpu) /* 0xCB 0x78 */
{
    cpu->cycles += 8;
    bit8(cpu, 7, cpu->b.B);
}

static void bit_7_c(cpuZ80 *cpu) /* 0xCB 0x79 */
{
    cpu->cycles += 8;
    bit8(cpu, 7, cpu->b.C);
}

static void bit_7_d(cpuZ80 *cpu) /* 0xCB 0x7A */
{
    cpu->cycles += 8;
    bit8(cpu, 7, cpu->b.D);
}

static void bit_7_e(cpuZ80 *cpu) /* 0xCB 0x7B */
{
    cpu->cycles += 8;
    bit8(cpu, 7, cpu->b.E);
}

static void bit_7_h(cpuZ80 *cpu) /* 0xCB 0x7C */
{
    cpu->cycles += 8;
    bit8(cpu, 7, cpu->b.H);
}

static void bit_7_l(cpuZ80 *cpu) /* 0xCB 0x7D */
{
    cpu->cycles += 8;
    bit8(cpu, 7, cpu->b.L);
}

static void bit_7_mhl(cpuZ80 *cpu) /* 0xCB 0x7E */
{
    cpu->cycles += 12;
    bit8(cpu, 7, read8(cpu, cpu->w.HL));
}

static void bit_7_a(cpuZ80 *cpu) /* 0xCB 0x7F */
{
    cpu->cycles += 8;
    bit8(cpu, 7, cpu->b.A);
}

static void res_0_b(cpuZ80 *cpu) /* 0xCB 0x80 */
{
    cpu->cycles += 8;
    cpu->b.B = res8(cpu, 0, cpu->b.B);
}

static void res_0_c(cpuZ80 *cpu) /* 0xCB 0x81 */
{
    cpu->cycles += 8;
    cpu->b.C = res8(cpu, 0, cpu->b.C);
}

static void res_0_d(cpuZ80 *cpu) /* 0xCB 0x82 */
{
    cpu->cycles += 8;
    cpu->b.D = res8(cpu, 0, cpu->b.D);
}

static void res_0_e(cpuZ80 *cpu) /* 0xCB 0x83 */
{
    cpu->cycles += 8;
    cpu->b.E = res8(cpu, 0, cpu->b.E);
}

static void res_0_h(cpuZ80 *cpu) /* 0xCB 0x84 */
{
    cpu->cycles += 8;
    cpu->b.H = res8(cpu, 0, cpu->b.H);
}

static void res_0_l(cpuZ80 *cpu) /* 0xCB 0x85 */
{
    cpu->cycles += 8;
    cpu->b.L = res8(cpu, 0, cpu->b.L);
}

static void res_0_mhl(cpuZ80 *cpu) /* 0xCB 0x86 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, res8(cpu, 0, read8(cpu, cpu->w.HL)));
}

static void res_0_a(cpuZ80 *cpu) /* 0xCB 0x87 */
{
    cpu->cycles += 8;
    cpu->b.A = res8(cpu, 0, cpu->b.A);
}

static void res_1_b(cpuZ80 *cpu) /* 0xCB 0x88 */
{
    cpu->cycles += 8;
    cpu->b.B = res8(cpu, 1, cpu->b.B);
}

static void res_1_c(cpuZ80 *cpu) /* 0xCB 0x89 */
{
    cpu->cycles += 8;
    cpu->b.C = res8(cpu, 1, cpu->b.C);
}

static void res_1_d(cpuZ80 *cpu) /* 0xCB 0x8A */
{
    cpu->cycles += 8;
    cpu->b.D = res8(cpu, 1, cpu->b.D);
}

static void res_1_e(cpuZ80 *cpu) /* 0xCB 0x8B */
{
    cpu->cycles += 8;
    cpu->b.E = res8(cpu, 1, cpu->b.E);
}

static void res_1_h(cpuZ80 *cpu) /* 0xCB 0x8C */
{
    cpu->cycles += 8;
    cpu->b.H = res8(cpu, 1, cpu->b.H);
}

static void res_1_l(cpuZ80 *cpu) /* 0xCB 0x8D */
{
    cpu->cycles += 8;
    cpu->b.L = res8(cpu, 1, cpu->b.L);
}

static void res_1_mhl(cpuZ80 *cpu) /* 0xCB 0x8E */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, res8(cpu, 1, read8(cpu, cpu->w.HL)));
}

static void res_1_a(cpuZ80 *cpu) /* 0xCB 0x8F */
{
    cpu->cycles += 8;
    cpu->b.A = res8(cpu, 1, cpu->b.A);
}

static void res_2_b(cpuZ80 *cpu) /* 0xCB 0x90 */
{
    cpu->cycles += 8;
    cpu->b.B = res8(cpu, 2, cpu->b.B);
}

static void res_2_c(cpuZ80 *cpu) /* 0xCB 0x91 */
{
    cpu->cycles += 8;
    cpu->b.C = res8(cpu, 2, cpu->b.C);
}

static void res_2_d(cpuZ80 *cpu) /* 0xCB 0x92 */
{
    cpu->cycles += 8;
    cpu->b.D = res8(cpu, 2, cpu->b.D);
}

static void res_2_e(cpuZ80 *cpu) /* 0xCB 0x93 */
{
    cpu->cycles += 8;
    cpu->b.E = res8(cpu, 2, cpu->b.E);
}

static void res_2_h(cpuZ80 *cpu) /* 0xCB 0x94 */
{
    cpu->cycles += 8;
    cpu->b.H = res8(cpu, 2, cpu->b.H);
}

static void res_2_l(cpuZ80 *cpu) /* 0xCB 0x95 */
{
    cpu->cycles += 8;
    cpu->b.L = res8(cpu, 2, cpu->b.L);
}

static void res_2_mhl(cpuZ80 *cpu) /* 0xCB 0x96 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, res8(cpu, 2, read8(cpu, cpu->w.HL)));
}

static void res_2_a(cpuZ80 *cpu) /* 0xCB 0x97 */
{
    cpu->cycles += 8;
    cpu->b.A = res8(cpu, 2, cpu->b.A);
}

static void res_3_b(cpuZ80 *cpu) /* 0xCB 0x98 */
{
    cpu->cycles += 8;
    cpu->b.B = res8(cpu, 3, cpu->b.B);
}

static void res_3_c(cpuZ80 *cpu) /* 0xCB 0x99 */
{
    cpu->cycles += 8;
    cpu->b.C = res8(cpu, 3, cpu->b.C);
}

static void res_3_d(cpuZ80 *cpu) /* 0xCB 0x9A */
{
    cpu->cycles += 8;
    cpu->b.D = res8(cpu, 3, cpu->b.D);
}

static void res_3_e(cpuZ80 *cpu) /* 0xCB 0x9B */
{
    cpu->cycles += 8;
    cpu->b.E = res8(cpu, 3, cpu->b.E);
}

static void res_3_h(cpuZ80 *cpu) /* 0xCB 0x9C */
{
    cpu->cycles += 8;
    cpu->b.H = res8(cpu, 3, cpu->b.H);
}

static void res_3_l(cpuZ80 *cpu) /* 0xCB 0x9D */
{
    cpu->cycles += 8;
    cpu->b.L = res8(cpu, 3, cpu->b.L);
}

static void res_3_mhl(cpuZ80 *cpu) /* 0xCB 0x9E */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, res8(cpu, 3, read8(cpu, cpu->w.HL)));
}

static void res_3_a(cpuZ80 *cpu) /* 0xCB 0x9F */
{
    cpu->cycles += 8;
    cpu->b.A = res8(cpu, 3, cpu->b.A);
}

static void res_4_b(cpuZ80 *cpu) /* 0xCB 0xA0 */
{
    cpu->cycles += 8;
    cpu->b.B = res8(cpu, 4, cpu->b.B);
}

static void res_4_c(cpuZ80 *cpu) /* 0xCB 0xA1 */
{
    cpu->cycles += 8;
    cpu->b.C = res8(cpu, 4, cpu->b.C);
}

static void res_4_d(cpuZ80 *cpu) /* 0xCB 0xA2 */
{
    cpu->cycles += 8;
    cpu->b.D = res8(cpu, 4, cpu->b.D);
}

static void res_4_e(cpuZ80 *cpu) /* 0xCB 0xA3 */
{
    cpu->cycles += 8;
    cpu->b.E = res8(cpu, 4, cpu->b.E);
}

static void res_4_h(cpuZ80 *cpu) /* 0xCB 0xA4 */
{
    cpu->cycles += 8;
    cpu->b.H = res8(cpu, 4, cpu->b.H);
}

static void res_4_l(cpuZ80 *cpu) /* 0xCB 0xA5 */
{
    cpu->cycles += 8;
    cpu->b.L = res8(cpu, 4, cpu->b.L);
}

static void res_4_mhl(cpuZ80 *cpu) /* 0xCB 0xA6 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, res8(cpu, 4, read8(cpu, cpu->w.HL)));
}

static void res_4_a(cpuZ80 *cpu) /* 0xCB 0xA7 */
{
    cpu->cycles += 8;
    cpu->b.A = res8(cpu, 4, cpu->b.A);
}

static void res_5_b(cpuZ80 *cpu) /* 0xCB 0xA8 */
{
    cpu->cycles += 8;
    cpu->b.B = res8(cpu, 5, cpu->b.B);
}

static void res_5_c(cpuZ80 *cpu) /* 0xCB 0xA9 */
{
    cpu->cycles += 8;
    cpu->b.C = res8(cpu, 5, cpu->b.C);
}

static void res_5_d(cpuZ80 *cpu) /* 0xCB 0xAA */
{
    cpu->cycles += 8;
    cpu->b.D = res8(cpu, 5, cpu->b.D);
}

static void res_5_e(cpuZ80 *cpu) /* 0xCB 0xAB */
{
    cpu->cycles += 8;
    cpu->b.E = res8(cpu, 5, cpu->b.E);
}

static void res_5_h(cpuZ80 *cpu) /* 0xCB 0xAC */
{
    cpu->cycles += 8;
    cpu->b.H = res8(cpu, 5, cpu->b.H);
}

static void res_5_l(cpuZ80 *cpu) /* 0xCB 0xAD */
{
    cpu->cycles += 8;
    cpu->b.L = res8(cpu, 5, cpu->b.L);
}

static void res_5_mhl(cpuZ80 *cpu) /* 0xCB 0xAE */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, res8(cpu, 5, read8(cpu, cpu->w.HL)));
}

static void res_5_a(cpuZ80 *cpu) /* 0xCB 0xAF */
{
    cpu->cycles += 8;
    cpu->b.A = res8(cpu, 5, cpu->b.A);
}

static void res_6_b(cpuZ80 *cpu) /* 0xCB 0xB0 */
{
    cpu->cycles += 8;
    cpu->b.B = res8(cpu, 6, cpu->b.B);
}

static void res_6_c(cpuZ80 *cpu) /* 0xCB 0xB1 */
{
    cpu->cycles += 8;
    cpu->b.C = res8(cpu, 6, cpu->b.C);
}

static void res_6_d(cpuZ80 *cpu) /* 0xCB 0xB2 */
{
    cpu->cycles += 8;
    cpu->b.D = res8(cpu, 6, cpu->b.D);
}

static void res_6_e(cpuZ80 *cpu) /* 0xCB 0xB3 */
{
    cpu->cycles += 8;
    cpu->b.E = res8(cpu, 6, cpu->b.E);
}

static void res_6_h(cpuZ80 *cpu) /* 0xCB 0xB4 */
{
    cpu->cycles += 8;
    cpu->b.H = res8(cpu, 6, cpu->b.H);
}

static void res_6_l(cpuZ80 *cpu) /* 0xCB 0xB5 */
{
    cpu->cycles += 8;
    cpu->b.L = res8(cpu, 6, cpu->b.L);
}

static void res_6_mhl(cpuZ80 *cpu) /* 0xCB 0xB6 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, res8(cpu, 6, read8(cpu, cpu->w.HL)));
}

static void res_6_a(cpuZ80 *cpu) /* 0xCB 0xB7 */
{
    cpu->cycles += 8;
    cpu->b.A = res8(cpu, 6, cpu->b.A);
}

static void res_7_b(cpuZ80 *cpu) /* 0xCB 0xB8 */
{
    cpu->cycles += 8;
    cpu->b.B = res8(cpu, 7, cpu->b.B);
}

static void res_7_c(cpuZ80 *cpu) /* 0xCB 0xB9 */
{
    cpu->cycles += 8;
    cpu->b.C = res8(cpu, 7, cpu->b.C);
}

static void res_7_d(cpuZ80 *cpu) /* 0xCB 0xBA */
{
    cpu->cycles += 8;
    cpu->b.D = res8(cpu, 7, cpu->b.D);
}

static void res_7_e(cpuZ80 *cpu) /* 0xCB 0xBB */
{
    cpu->cycles += 8;
    cpu->b.E = res8(cpu, 7, cpu->b.E);
}

static void res_7_h(cpuZ80 *cpu) /* 0xCB 0xBC */
{
    cpu->cycles += 8;
    cpu->b.H = res8(cpu, 7, cpu->b.H);
}

static void res_7_l(cpuZ80 *cpu) /* 0xCB 0xBD */
{
    cpu->cycles += 8;
    cpu->b.L = res8(cpu, 7, cpu->b.L);
}

static void res_7_mhl(cpuZ80 *cpu) /* 0xCB 0xBE */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, res8(cpu, 7, read8(cpu, cpu->w.HL)));
}

static void res_7_a(cpuZ80 *cpu) /* 0xCB 0xBF */
{
    cpu->cycles += 8;
    cpu->b.A = res8(cpu, 7, cpu->b.A);
}

static void set_0_b(cpuZ80 *cpu) /* 0xCB 0xC0 */
{
    cpu->cycles += 8;
    cpu->b.B = set8(cpu, 0, cpu->b.B);
}

static void set_0_c(cpuZ80 *cpu) /* 0xCB 0xC1 */
{
    cpu->cycles += 8;
    cpu->b.C = set8(cpu, 0, cpu->b.C);
}

static void set_0_d(cpuZ80 *cpu) /* 0xCB 0xC2 */
{
    cpu->cycles += 8;
    cpu->b.D = set8(cpu, 0, cpu->b.D);
}

static void set_0_e(cpuZ80 *cpu) /* 0xCB 0xC3 */
{
    cpu->cycles += 8;
    cpu->b.E = set8(cpu, 0, cpu->b.E);
}

static void set_0_h(cpuZ80 *cpu) /* 0xCB 0xC4 */
{
    cpu->cycles += 8;
    cpu->b.H = set8(cpu, 0, cpu->b.H);
}

static void set_0_l(cpuZ80 *cpu) /* 0xCB 0xC5 */
{
    cpu->cycles += 8;
    cpu->b.L = set8(cpu, 0, cpu->b.L);
}

static void set_0_mhl(cpuZ80 *cpu) /* 0xCB 0xC6 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, set8(cpu, 0, read8(cpu, cpu->w.HL)));
}

static void set_0_a(cpuZ80 *cpu) /* 0xCB 0xC7 */
{
    cpu->cycles += 8;
    cpu->b.A = set8(cpu, 0, cpu->b.A);
}

static void set_1_b(cpuZ80 *cpu) /* 0xCB 0xC8 */
{
    cpu->cycles += 8;
    cpu->b.B = set8(cpu, 1, cpu->b.B);
}

static void set_1_c(cpuZ80 *cpu) /* 0xCB 0xC9 */
{
    cpu->cycles += 8;
    cpu->b.C = set8(cpu, 1, cpu->b.C);
}

static void set_1_d(cpuZ80 *cpu) /* 0xCB 0xCA */
{
    cpu->cycles += 8;
    cpu->b.D = set8(cpu, 1, cpu->b.D);
}

static void set_1_e(cpuZ80 *cpu) /* 0xCB 0xCB */
{
    cpu->cycles += 8;
    cpu->b.E = set8(cpu, 1, cpu->b.E);
}

static void set_1_h(cpuZ80 *cpu) /* 0xCB 0xCC */
{
    cpu->cycles += 8;
    cpu->b.H = set8(cpu, 1, cpu->b.H);
}

static void set_1_l(cpuZ80 *cpu) /* 0xCB 0xCD */
{
    cpu->cycles += 8;
    cpu->b.L = set8(cpu, 1, cpu->b.L);
}

static void set_1_mhl(cpuZ80 *cpu) /* 0xCB 0xCE */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, set8(cpu, 1, read8(cpu, cpu->w.HL)));
}

static void set_1_a(cpuZ80 *cpu) /* 0xCB 0xCF */
{
    cpu->cycles += 8;
    cpu->b.A = set8(cpu, 1, cpu->b.A);
}

static void set_2_b(cpuZ80 *cpu) /* 0xCB 0xD0 */
{
    cpu->cycles += 8;
    cpu->b.B = set8(cpu, 2, cpu->b.B);
}

static void set_2_c(cpuZ80 *cpu) /* 0xCB 0xD1 */
{
    cpu->cycles += 8;
    cpu->b.C = set8(cpu, 2, cpu->b.C);
}

static void set_2_d(cpuZ80 *cpu) /* 0xCB 0xD2 */
{
    cpu->cycles += 8;
    cpu->b.D = set8(cpu, 2, cpu->b.D);
}

static void set_2_e(cpuZ80 *cpu) /* 0xCB 0xD3 */
{
    cpu->cycles += 8;
    cpu->b.E = set8(cpu, 2, cpu->b.E);
}

static void set_2_h(cpuZ80 *cpu) /* 0xCB 0xD4 */
{
    cpu->cycles += 8;
    cpu->b.H = set8(cpu, 2, cpu->b.H);
}

static void set_2_l(cpuZ80 *cpu) /* 0xCB 0xD5 */
{
    cpu->cycles += 8;
    cpu->b.L = set8(cpu, 2, cpu->b.L);
}

static void set_2_mhl(cpuZ80 *cpu) /* 0xCB 0xD6 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, set8(cpu, 2, read8(cpu, cpu->w.HL)));
}

static void set_2_a(cpuZ80 *cpu) /* 0xCB 0xD7 */
{
    cpu->cycles += 8;
    cpu->b.A = set8(cpu, 2, cpu->b.A);
}

static void set_3_b(cpuZ80 *cpu) /* 0xCB 0xD8 */
{
    cpu->cycles += 8;
    cpu->b.B = set8(cpu, 3, cpu->b.B);
}

static void set_3_c(cpuZ80 *cpu) /* 0xCB 0xD9 */
{
    cpu->cycles += 8;
    cpu->b.C = set8(cpu, 3, cpu->b.C);
}

static void set_3_d(cpuZ80 *cpu) /* 0xCB 0xDA */
{
    cpu->cycles += 8;
    cpu->b.D = set8(cpu, 3, cpu->b.D);
}

static void set_3_e(cpuZ80 *cpu) /* 0xCB 0xDB */
{
    cpu->cycles += 8;
    cpu->b.E = set8(cpu, 3, cpu->b.E);
}

static void set_3_h(cpuZ80 *cpu) /* 0xCB 0xDC */
{
    cpu->cycles += 8;
    cpu->b.H = set8(cpu, 3, cpu->b.H);
}

static void set_3_l(cpuZ80 *cpu) /* 0xCB 0xDD */
{
    cpu->cycles += 8;
    cpu->b.L = set8(cpu, 3, cpu->b.L);
}

static void set_3_mhl(cpuZ80 *cpu) /* 0xCB 0xDE */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, set8(cpu, 3, read8(cpu, cpu->w.HL)));
}

static void set_3_a(cpuZ80 *cpu) /* 0xCB 0xDF */
{
    cpu->cycles += 8;
    cpu->b.A = set8(cpu, 3, cpu->b.A);
}

static void set_4_b(cpuZ80 *cpu) /* 0xCB 0xE0 */
{
    cpu->cycles += 8;
    cpu->b.B = set8(cpu, 4, cpu->b.B);
}

static void set_4_c(cpuZ80 *cpu) /* 0xCB 0xE1 */
{
    cpu->cycles += 8;
    cpu->b.C = set8(cpu, 4, cpu->b.C);
}

static void set_4_d(cpuZ80 *cpu) /* 0xCB 0xE2 */
{
    cpu->cycles += 8;
    cpu->b.D = set8(cpu, 4, cpu->b.D);
}

static void set_4_e(cpuZ80 *cpu) /* 0xCB 0xE3 */
{
    cpu->cycles += 8;
    cpu->b.E = set8(cpu, 4, cpu->b.E);
}

static void set_4_h(cpuZ80 *cpu) /* 0xCB 0xE4 */
{
    cpu->cycles += 8;
    cpu->b.H = set8(cpu, 4, cpu->b.H);
}

static void set_4_l(cpuZ80 *cpu) /* 0xCB 0xE5 */
{
    cpu->cycles += 8;
    cpu->b.L = set8(cpu, 4, cpu->b.L);
}

static void set_4_mhl(cpuZ80 *cpu) /* 0xCB 0xE6 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, set8(cpu, 4, read8(cpu, cpu->w.HL)));
}

static void set_4_a(cpuZ80 *cpu) /* 0xCB 0xE7 */
{
    cpu->cycles += 8;
    cpu->b.A = set8(cpu, 4, cpu->b.A);
}

static void set_5_b(cpuZ80 *cpu) /* 0xCB 0xE8 */
{
    cpu->cycles += 8;
    cpu->b.B = set8(cpu, 5, cpu->b.B);
}

static void set_5_c(cpuZ80 *cpu) /* 0xCB 0xE9 */
{
    cpu->cycles += 8;
    cpu->b.C = set8(cpu, 5, cpu->b.C);
}

static void set_5_d(cpuZ80 *cpu) /* 0xCB 0xEA */
{
    cpu->cycles += 8;
    cpu->b.D = set8(cpu, 5, cpu->b.D);
}

static void set_5_e(cpuZ80 *cpu) /* 0xCB 0xEB */
{
    cpu->cycles += 8;
    cpu->b.E = set8(cpu, 5, cpu->b.E);
}

static void set_5_h(cpuZ80 *cpu) /* 0xCB 0xEC */
{
    cpu->cycles += 8;
    cpu->b.H = set8(cpu, 5, cpu->b.H);
}

static void set_5_l(cpuZ80 *cpu) /* 0xCB 0xED */
{
    cpu->cycles += 8;
    cpu->b.L = set8(cpu, 5, cpu->b.L);
}

static void set_5_mhl(cpuZ80 *cpu) /* 0xCB 0xEE */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, set8(cpu, 5, read8(cpu, cpu->w.HL)));
}

static void set_5_a(cpuZ80 *cpu) /* 0xCB 0xEF */
{
    cpu->cycles += 8;
    cpu->b.A = set8(cpu, 5, cpu->b.A);
}

static void set_6_b(cpuZ80 *cpu) /* 0xCB 0xF0 */
{
    cpu->cycles += 8;
    cpu->b.B = set8(cpu, 6, cpu->b.B);
}

static void set_6_c(cpuZ80 *cpu) /* 0xCB 0xF1 */
{
    cpu->cycles += 8;
    cpu->b.C = set8(cpu, 6, cpu->b.C);
}

static void set_6_d(cpuZ80 *cpu) /* 0xCB 0xF2 */
{
    cpu->cycles += 8;
    cpu->b.D = set8(cpu, 6, cpu->b.D);
}

static void set_6_e(cpuZ80 *cpu) /* 0xCB 0xF3 */
{
    cpu->cycles += 8;
    cpu->b.E = set8(cpu, 6, cpu->b.E);
}

static void set_6_h(cpuZ80 *cpu) /* 0xCB 0xF4 */
{
    cpu->cycles += 8;
    cpu->b.H = set8(cpu, 6, cpu->b.H);
}

static void set_6_l(cpuZ80 *cpu) /* 0xCB 0xF5 */
{
    cpu->cycles += 8;
    cpu->b.L = set8(cpu, 6, cpu->b.L);
}

static void set_6_mhl(cpuZ80 *cpu) /* 0xCB 0xF6 */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, set8(cpu, 6, read8(cpu, cpu->w.HL)));
}

static void set_6_a(cpuZ80 *cpu) /* 0xCB 0xF7 */
{
    cpu->cycles += 8;
    cpu->b.A = set8(cpu, 6, cpu->b.A);
}

static void set_7_b(cpuZ80 *cpu) /* 0xCB 0xF8 */
{
    cpu->cycles += 8;
    cpu->b.B = set8(cpu, 7, cpu->b.B);
}

static void set_7_c(cpuZ80 *cpu) /* 0xCB 0xF9 */
{
    cpu->cycles += 8;
    cpu->b.C = set8(cpu, 7, cpu->b.C);
}

static void set_7_d(cpuZ80 *cpu) /* 0xCB 0xFA */
{
    cpu->cycles += 8;
    cpu->b.D = set8(cpu, 7, cpu->b.D);
}

static void set_7_e(cpuZ80 *cpu) /* 0xCB 0xFB */
{
    cpu->cycles += 8;
    cpu->b.E = set8(cpu, 7, cpu->b.E);
}

static void set_7_h(cpuZ80 *cpu) /* 0xCB 0xFC */
{
    cpu->cycles += 8;
    cpu->b.H = set8(cpu, 7, cpu->b.H);
}

static void set_7_l(cpuZ80 *cpu) /* 0xCB 0xFD */
{
    cpu->cycles += 8;
    cpu->b.L = set8(cpu, 7, cpu->b.L);
}

static void set_7_mhl(cpuZ80 *cpu) /* 0xCB 0xFE */
{
    cpu->cycles += 15;
    write8(cpu, cpu->w.HL, set8(cpu, 7, read8(cpu, cpu->w.HL)));
}

static void set_7_a(cpuZ80 *cpu) /* 0xCB 0xFF */
{
    cpu->cycles += 8;
    cpu->b.A = set8(cpu, 7, cpu->b.A);
}

opcode opcodes_cb[256] = {
    { rlc_b,        DASM("RLC B")       /* 0xCB 0x00 */ },
    { rlc_c,        DASM("RLC C")       /* 0xCB 0x01 */ },
    { rlc_d,        DASM("RLC D")       /* 0xCB 0x02 */ },
    { rlc_e,        DASM("RLC E")       /* 0xCB 0x03 */ },
    { rlc_h,        DASM("RLC H")       /* 0xCB 0x04 */ },
    { rlc_l,        DASM("RLC L")       /* 0xCB 0x05 */ },
    { rlc_mhl,      DASM("RLC (HL)")    /* 0xCB 0x06 */ },
    { rlc_a,        DASM("RLC A")       /* 0xCB 0x07 */ },
    { rrc_b,        DASM("RRC B")       /* 0xCB 0x08 */ },
    { rrc_c,        DASM("RRC C")       /* 0xCB 0x09 */ },
    { rrc_d,        DASM("RRC D")       /* 0xCB 0x0A */ },
    { rrc_e,        DASM("RRC E")       /* 0xCB 0x0B */ },
    { rrc_h,        DASM("RRC H")       /* 0xCB 0x0C */ },
    { rrc_l,        DASM("RRC L")       /* 0xCB 0x0D */ },
    { rrc_mhl,      DASM("RRC (HL)")    /* 0xCB 0x0E */ },
    { rrc_a,        DASM("RRC A")       /* 0xCB 0x0F */ },
    { rl_b,         DASM("RL B")        /* 0xCB 0x10 */ },
    { rl_c,         DASM("RL C")        /* 0xCB 0x11 */ },
    { rl_d,         DASM("RL D")        /* 0xCB 0x12 */ },
    { rl_e,         DASM("RL E")        /* 0xCB 0x13 */ },
    { rl_h,         DASM("RL H")        /* 0xCB 0x14 */ },
    { rl_l,         DASM("RL L")        /* 0xCB 0x15 */ },
    { rl_mhl,       DASM("RL (HL)")     /* 0xCB 0x16 */ },
    { rl_a,         DASM("RL A")        /* 0xCB 0x17 */ },
    { rr_b,         DASM("RR B")        /* 0xCB 0x18 */ },
    { rr_c,         DASM("RR C")        /* 0xCB 0x19 */ },
    { rr_d,         DASM("RR D")        /* 0xCB 0x1A */ },
    { rr_e,         DASM("RR E")        /* 0xCB 0x1B */ },
    { rr_h,         DASM("RR H")        /* 0xCB 0x1C */ },
    { rr_l,         DASM("RR L")        /* 0xCB 0x1D */ },
    { rr_mhl,       DASM("RR (HL)")     /* 0xCB 0x1E */ },
    { rr_a,         DASM("RR A")        /* 0xCB 0x1F */ },
    { sla_b,        DASM("SLA B")       /* 0xCB 0x20 */ },
    { sla_c,        DASM("SLA C")       /* 0xCB 0x21 */ },
    { sla_d,        DASM("SLA D")       /* 0xCB 0x22 */ },
    { sla_e,        DASM("SLA E")       /* 0xCB 0x23 */ },
    { sla_h,        DASM("SLA H")       /* 0xCB 0x24 */ },
    { sla_l,        DASM("SLA L")       /* 0xCB 0x25 */ },
    { sla_mhl,      DASM("SLA (HL)")    /* 0xCB 0x26 */ },
    { sla_a,        DASM("SLA A")       /* 0xCB 0x27 */ },
    { sra_b,        DASM("SRA B")       /* 0xCB 0x28 */ },
    { sra_c,        DASM("SRA C")       /* 0xCB 0x29 */ },
    { sra_d,        DASM("SRA D")       /* 0xCB 0x2A */ },
    { sra_e,        DASM("SRA E")       /* 0xCB 0x2B */ },
    { sra_h,        DASM("SRA H")       /* 0xCB 0x2C */ },
    { sra_l,        DASM("SRA L")       /* 0xCB 0x2D */ },
    { sra_mhl,      DASM("SRA (HL)")    /* 0xCB 0x2E */ },
    { sra_a,        DASM("SRA A")       /* 0xCB 0x2F */ },
    { sll_b,        DASM("SLL B")       /* 0xCB 0x30 */ },
    { sll_c,        DASM("SLL C")       /* 0xCB 0x31 */ },
    { sll_d,        DASM("SLL D")       /* 0xCB 0x32 */ },
    { sll_e,        DASM("SLL E")       /* 0xCB 0x33 */ },
    { sll_h,        DASM("SLL H")       /* 0xCB 0x34 */ },
    { sll_l,        DASM("SLL L")       /* 0xCB 0x35 */ },
    { sll_mhl,      DASM("SLL (HL)")    /* 0xCB 0x36 */ },
    { sll_a,        DASM("SLL A")       /* 0xCB 0x37 */ },
    { srl_b,        DASM("SRL B")       /* 0xCB 0x38 */ },
    { srl_c,        DASM("SRL C")       /* 0xCB 0x39 */ },
    { srl_d,        DASM("SRL D")       /* 0xCB 0x3A */ },
    { srl_e,        DASM("SRL E")       /* 0xCB 0x3B */ },
    { srl_h,        DASM("SRL H")       /* 0xCB 0x3C */ },
    { srl_l,        DASM("SRL L")       /* 0xCB 0x3D */ },
    { srl_mhl,      DASM("SRL (HL)")    /* 0xCB 0x3E */ },
    { srl_a,        DASM("SRL A")       /* 0xCB 0x3F */ },
    { bit_0_b,      DASM("BIT 0,B")     /* 0xCB 0x40 */ },
    { bit_0_c,      DASM("BIT 0,C")     /* 0xCB 0x41 */ },
    { bit_0_d,      DASM("BIT 0,D")     /* 0xCB 0x42 */ },
    { bit_0_e,      DASM("BIT 0,E")     /* 0xCB 0x43 */ },
    { bit_0_h,      DASM("BIT 0,H")     /* 0xCB 0x44 */ },
    { bit_0_l,      DASM("BIT 0,L")     /* 0xCB 0x45 */ },
    { bit_0_mhl,    DASM("BIT 0,(HL)")  /* 0xCB 0x46 */ },
    { bit_0_a,      DASM("BIT 0,A")     /* 0xCB 0x47 */ },
    { bit_1_b,      DASM("BIT 1,B")     /* 0xCB 0x48 */ },
    { bit_1_c,      DASM("BIT 1,C")     /* 0xCB 0x49 */ },
    { bit_1_d,      DASM("BIT 1,D")     /* 0xCB 0x4A */ },
    { bit_1_e,      DASM("BIT 1,E")     /* 0xCB 0x4B */ },
    { bit_1_h,      DASM("BIT 1,H")     /* 0xCB 0x4C */ },
    { bit_1_l,      DASM("BIT 1,L")     /* 0xCB 0x4D */ },
    { bit_1_mhl,    DASM("BIT 1,(HL)")  /* 0xCB 0x4E */ },
    { bit_1_a,      DASM("BIT 1,A")     /* 0xCB 0x4F */ },
    { bit_2_b,      DASM("BIT 2,B")     /* 0xCB 0x50 */ },
    { bit_2_c,      DASM("BIT 2,C")     /* 0xCB 0x51 */ },
    { bit_2_d,      DASM("BIT 2,D")     /* 0xCB 0x52 */ },
    { bit_2_e,      DASM("BIT 2,E")     /* 0xCB 0x53 */ },
    { bit_2_h,      DASM("BIT 2,H")     /* 0xCB 0x54 */ },
    { bit_2_l,      DASM("BIT 2,L")     /* 0xCB 0x55 */ },
    { bit_2_mhl,    DASM("BIT 2,(HL)")  /* 0xCB 0x56 */ },
    { bit_2_a,      DASM("BIT 2,A")     /* 0xCB 0x57 */ },
    { bit_3_b,      DASM("BIT 3,B")     /* 0xCB 0x58 */ },
    { bit_3_c,      DASM("BIT 3,C")     /* 0xCB 0x59 */ },
    { bit_3_d,      DASM("BIT 3,D")     /* 0xCB 0x5A */ },
    { bit_3_e,      DASM("BIT 3,E")     /* 0xCB 0x5B */ },
    { bit_3_h,      DASM("BIT 3,H")     /* 0xCB 0x5C */ },
    { bit_3_l,      DASM("BIT 3,L")     /* 0xCB 0x5D */ },
    { bit_3_mhl,    DASM("BIT 3,(HL)")  /* 0xCB 0x5E */ },
    { bit_3_a,      DASM("BIT 3,A")     /* 0xCB 0x5F */ },
    { bit_4_b,      DASM("BIT 4,B")     /* 0xCB 0x60 */ },
    { bit_4_c,      DASM("BIT 4,C")     /* 0xCB 0x61 */ },
    { bit_4_d,      DASM("BIT 4,D")     /* 0xCB 0x62 */ },
    { bit_4_e,      DASM("BIT 4,E")     /* 0xCB 0x63 */ },
    { bit_4_h,      DASM("BIT 4,H")     /* 0xCB 0x64 */ },
    { bit_4_l,      DASM("BIT 4,L")     /* 0xCB 0x65 */ },
    { bit_4_mhl,    DASM("BIT 4,(HL)")  /* 0xCB 0x66 */ },
    { bit_4_a,      DASM("BIT 4,A")     /* 0xCB 0x67 */ },
    { bit_5_b,      DASM("BIT 5,B")     /* 0xCB 0x68 */ },
    { bit_5_c,      DASM("BIT 5,C")     /* 0xCB 0x69 */ },
    { bit_5_d,      DASM("BIT 5,D")     /* 0xCB 0x6A */ },
    { bit_5_e,      DASM("BIT 5,E")     /* 0xCB 0x6B */ },
    { bit_5_h,      DASM("BIT 5,H")     /* 0xCB 0x6C */ },
    { bit_5_l,      DASM("BIT 5,L")     /* 0xCB 0x6D */ },
    { bit_5_mhl,    DASM("BIT 5,(HL)")  /* 0xCB 0x6E */ },
    { bit_5_a,      DASM("BIT 5,A")     /* 0xCB 0x6F */ },
    { bit_6_b,      DASM("BIT 6,B")     /* 0xCB 0x70 */ },
    { bit_6_c,      DASM("BIT 6,C")     /* 0xCB 0x71 */ },
    { bit_6_d,      DASM("BIT 6,D")     /* 0xCB 0x72 */ },
    { bit_6_e,      DASM("BIT 6,E")     /* 0xCB 0x73 */ },
    { bit_6_h,      DASM("BIT 6,H")     /* 0xCB 0x74 */ },
    { bit_6_l,      DASM("BIT 6,L")     /* 0xCB 0x75 */ },
    { bit_6_mhl,    DASM("BIT 6,(HL)")  /* 0xCB 0x76 */ },
    { bit_6_a,      DASM("BIT 6,A")     /* 0xCB 0x77 */ },
    { bit_7_b,      DASM("BIT 7,B")     /* 0xCB 0x78 */ },
    { bit_7_c,      DASM("BIT 7,C")     /* 0xCB 0x79 */ },
    { bit_7_d,      DASM("BIT 7,D")     /* 0xCB 0x7A */ },
    { bit_7_e,      DASM("BIT 7,E")     /* 0xCB 0x7B */ },
    { bit_7_h,      DASM("BIT 7,H")     /* 0xCB 0x7C */ },
    { bit_7_l,      DASM("BIT 7,L")     /* 0xCB 0x7D */ },
    { bit_7_mhl,    DASM("BIT 7,(HL)")  /* 0xCB 0x7E */ },
    { bit_7_a,      DASM("BIT 7,A")     /* 0xCB 0x7F */ },
    { res_0_b,      DASM("RES 0,B")     /* 0xCB 0x80 */ },
    { res_0_c,      DASM("RES 0,C")     /* 0xCB 0x81 */ },
    { res_0_d,      DASM("RES 0,D")     /* 0xCB 0x82 */ },
    { res_0_e,      DASM("RES 0,E")     /* 0xCB 0x83 */ },
    { res_0_h,      DASM("RES 0,H")     /* 0xCB 0x84 */ },
    { res_0_l,      DASM("RES 0,L")     /* 0xCB 0x85 */ },
    { res_0_mhl,    DASM("RES 0,(HL)")  /* 0xCB 0x86 */ },
    { res_0_a,      DASM("RES 0,A")     /* 0xCB 0x87 */ },
    { res_1_b,      DASM("RES 1,B")     /* 0xCB 0x88 */ },
    { res_1_c,      DASM("RES 1,C")     /* 0xCB 0x89 */ },
    { res_1_d,      DASM("RES 1,D")     /* 0xCB 0x8A */ },
    { res_1_e,      DASM("RES 1,E")     /* 0xCB 0x8B */ },
    { res_1_h,      DASM("RES 1,H")     /* 0xCB 0x8C */ },
    { res_1_l,      DASM("RES 1,L")     /* 0xCB 0x8D */ },
    { res_1_mhl,    DASM("RES 1,(HL)")  /* 0xCB 0x8E */ },
    { res_1_a,      DASM("RES 1,A")     /* 0xCB 0x8F */ },
    { res_2_b,      DASM("RES 2,B")     /* 0xCB 0x90 */ },
    { res_2_c,      DASM("RES 2,C")     /* 0xCB 0x91 */ },
    { res_2_d,      DASM("RES 2,D")     /* 0xCB 0x92 */ },
    { res_2_e,      DASM("RES 2,E")     /* 0xCB 0x93 */ },
    { res_2_h,      DASM("RES 2,H")     /* 0xCB 0x94 */ },
    { res_2_l,      DASM("RES 2,L")     /* 0xCB 0x95 */ },
    { res_2_mhl,    DASM("RES 2,(HL)")  /* 0xCB 0x96 */ },
    { res_2_a,      DASM("RES 2,A")     /* 0xCB 0x97 */ },
    { res_3_b,      DASM("RES 3,B")     /* 0xCB 0x98 */ },
    { res_3_c,      DASM("RES 3,C")     /* 0xCB 0x99 */ },
    { res_3_d,      DASM("RES 3,D")     /* 0xCB 0x9A */ },
    { res_3_e,      DASM("RES 3,E")     /* 0xCB 0x9B */ },
    { res_3_h,      DASM("RES 3,H")     /* 0xCB 0x9C */ },
    { res_3_l,      DASM("RES 3,L")     /* 0xCB 0x9D */ },
    { res_3_mhl,    DASM("RES 3,(HL)")  /* 0xCB 0x9E */ },
    { res_3_a,      DASM("RES 3,A")     /* 0xCB 0x9F */ },
    { res_4_b,      DASM("RES 4,B")     /* 0xCB 0xA0 */ },
    { res_4_c,      DASM("RES 4,C")     /* 0xCB 0xA1 */ },
    { res_4_d,      DASM("RES 4,D")     /* 0xCB 0xA2 */ },
    { res_4_e,      DASM("RES 4,E")     /* 0xCB 0xA3 */ },
    { res_4_h,      DASM("RES 4,H")     /* 0xCB 0xA4 */ },
    { res_4_l,      DASM("RES 4,L")     /* 0xCB 0xA5 */ },
    { res_4_mhl,    DASM("RES 4,(HL)")  /* 0xCB 0xA6 */ },
    { res_4_a,      DASM("RES 4,A")     /* 0xCB 0xA7 */ },
    { res_5_b,      DASM("RES 5,B")     /* 0xCB 0xA8 */ },
    { res_5_c,      DASM("RES 5,C")     /* 0xCB 0xA9 */ },
    { res_5_d,      DASM("RES 5,D")     /* 0xCB 0xAA */ },
    { res_5_e,      DASM("RES 5,E")     /* 0xCB 0xAB */ },
    { res_5_h,      DASM("RES 5,H")     /* 0xCB 0xAC */ },
    { res_5_l,      DASM("RES 5,L")     /* 0xCB 0xAD */ },
    { res_5_mhl,    DASM("RES 5,(HL)")  /* 0xCB 0xAE */ },
    { res_5_a,      DASM("RES 5,A")     /* 0xCB 0xAF */ },
    { res_6_b,      DASM("RES 6,B")     /* 0xCB 0xB0 */ },
    { res_6_c,      DASM("RES 6,C")     /* 0xCB 0xB1 */ },
    { res_6_d,      DASM("RES 6,D")     /* 0xCB 0xB2 */ },
    { res_6_e,      DASM("RES 6,E")     /* 0xCB 0xB3 */ },
    { res_6_h,      DASM("RES 6,H")     /* 0xCB 0xB4 */ },
    { res_6_l,      DASM("RES 6,L")     /* 0xCB 0xB5 */ },
    { res_6_mhl,    DASM("RES 6,(HL)")  /* 0xCB 0xB6 */ },
    { res_6_a,      DASM("RES 6,A")     /* 0xCB 0xB7 */ },
    { res_7_b,      DASM("RES 7,B")     /* 0xCB 0xB8 */ },
    { res_7_c,      DASM("RES 7,C")     /* 0xCB 0xB9 */ },
    { res_7_d,      DASM("RES 7,D")     /* 0xCB 0xBA */ },
    { res_7_e,      DASM("RES 7,E")     /* 0xCB 0xBB */ },
    { res_7_h,      DASM("RES 7,H")     /* 0xCB 0xBC */ },
    { res_7_l,      DASM("RES 7,L")     /* 0xCB 0xBD */ },
    { res_7_mhl,    DASM("RES 7,(HL)")  /* 0xCB 0xBE */ },
    { res_7_a,      DASM("RES 7,A")     /* 0xCB 0xBF */ },
    { set_0_b,      DASM("SET 0,B")     /* 0xCB 0xC0 */ },
    { set_0_c,      DASM("SET 0,C")     /* 0xCB 0xC1 */ },
    { set_0_d,      DASM("SET 0,D")     /* 0xCB 0xC2 */ },
    { set_0_e,      DASM("SET 0,E")     /* 0xCB 0xC3 */ },
    { set_0_h,      DASM("SET 0,H")     /* 0xCB 0xC4 */ },
    { set_0_l,      DASM("SET 0,L")     /* 0xCB 0xC5 */ },
    { set_0_mhl,    DASM("SET 0,(HL)")  /* 0xCB 0xC6 */ },
    { set_0_a,      DASM("SET 0,A")     /* 0xCB 0xC7 */ },
    { set_1_b,      DASM("SET 1,B")     /* 0xCB 0xC8 */ },
    { set_1_c,      DASM("SET 1,C")     /* 0xCB 0xC9 */ },
    { set_1_d,      DASM("SET 1,D")     /* 0xCB 0xCA */ },
    { set_1_e,      DASM("SET 1,E")     /* 0xCB 0xCB */ },
    { set_1_h,      DASM("SET 1,H")     /* 0xCB 0xCC */ },
    { set_1_l,      DASM("SET 1,L")     /* 0xCB 0xCD */ },
    { set_1_mhl,    DASM("SET 1,(HL)")  /* 0xCB 0xCE */ },
    { set_1_a,      DASM("SET 1,A")     /* 0xCB 0xCF */ },
    { set_2_b,      DASM("SET 2,B")     /* 0xCB 0xD0 */ },
    { set_2_c,      DASM("SET 2,C")     /* 0xCB 0xD1 */ },
    { set_2_d,      DASM("SET 2,D")     /* 0xCB 0xD2 */ },
    { set_2_e,      DASM("SET 2,E")     /* 0xCB 0xD3 */ },
    { set_2_h,      DASM("SET 2,H")     /* 0xCB 0xD4 */ },
    { set_2_l,      DASM("SET 2,L")     /* 0xCB 0xD5 */ },
    { set_2_mhl,    DASM("SET 2,(HL)")  /* 0xCB 0xD6 */ },
    { set_2_a,      DASM("SET 2,A")     /* 0xCB 0xD7 */ },
    { set_3_b,      DASM("SET 3,B")     /* 0xCB 0xD8 */ },
    { set_3_c,      DASM("SET 3,C")     /* 0xCB 0xD9 */ },
    { set_3_d,      DASM("SET 3,D")     /* 0xCB 0xDA */ },
    { set_3_e,      DASM("SET 3,E")     /* 0xCB 0xDB */ },
    { set_3_h,      DASM("SET 3,H")     /* 0xCB 0xDC */ },
    { set_3_l,      DASM("SET 3,L")     /* 0xCB 0xDD */ },
    { set_3_mhl,    DASM("SET 3,(HL)")  /* 0xCB 0xDE */ },
    { set_3_a,      DASM("SET 3,A")     /* 0xCB 0xDF */ },
    { set_4_b,      DASM("SET 4,B")     /* 0xCB 0xE0 */ },
    { set_4_c,      DASM("SET 4,C")     /* 0xCB 0xE1 */ },
    { set_4_d,      DASM("SET 4,D")     /* 0xCB 0xE2 */ },
    { set_4_e,      DASM("SET 4,E")     /* 0xCB 0xE3 */ },
    { set_4_h,      DASM("SET 4,H")     /* 0xCB 0xE4 */ },
    { set_4_l,      DASM("SET 4,L")     /* 0xCB 0xE5 */ },
    { set_4_mhl,    DASM("SET 4,(HL)")  /* 0xCB 0xE6 */ },
    { set_4_a,      DASM("SET 4,A")     /* 0xCB 0xE7 */ },
    { set_5_b,      DASM("SET 5,B")     /* 0xCB 0xE8 */ },
    { set_5_c,      DASM("SET 5,C")     /* 0xCB 0xE9 */ },
    { set_5_d,      DASM("SET 5,D")     /* 0xCB 0xEA */ },
    { set_5_e,      DASM("SET 5,E")     /* 0xCB 0xEB */ },
    { set_5_h,      DASM("SET 5,H")     /* 0xCB 0xEC */ },
    { set_5_l,      DASM("SET 5,L")     /* 0xCB 0xED */ },
    { set_5_mhl,    DASM("SET 5,(HL)")  /* 0xCB 0xEE */ },
    { set_5_a,      DASM("SET 5,A")     /* 0xCB 0xEF */ },
    { set_6_b,      DASM("SET 6,B")     /* 0xCB 0xF0 */ },
    { set_6_c,      DASM("SET 6,C")     /* 0xCB 0xF1 */ },
    { set_6_d,      DASM("SET 6,D")     /* 0xCB 0xF2 */ },
    { set_6_e,      DASM("SET 6,E")     /* 0xCB 0xF3 */ },
    { set_6_h,      DASM("SET 6,H")     /* 0xCB 0xF4 */ },
    { set_6_l,      DASM("SET 6,L")     /* 0xCB 0xF5 */ },
    { set_6_mhl,    DASM("SET 6,(HL)")  /* 0xCB 0xF6 */ },
    { set_6_a,      DASM("SET 6,A")     /* 0xCB 0xF7 */ },
    { set_7_b,      DASM("SET 7,B")     /* 0xCB 0xF8 */ },
    { set_7_c,      DASM("SET 7,C")     /* 0xCB 0xF9 */ },
    { set_7_d,      DASM("SET 7,D")     /* 0xCB 0xFA */ },
    { set_7_e,      DASM("SET 7,E")     /* 0xCB 0xFB */ },
    { set_7_h,      DASM("SET 7,H")     /* 0xCB 0xFC */ },
    { set_7_l,      DASM("SET 7,L")     /* 0xCB 0xFD */ },
    { set_7_mhl,    DASM("SET 7,(HL)")  /* 0xCB 0xFE */ },
    { set_7_a,      DASM("SET 7,A")     /* 0xCB 0xFF */ }
};


void opcode_cb(cpuZ80 *cpu)
{
    word pc = cpu->PC-1;

    int op = (int)read8(cpu, cpu->PC++);

    if(opcodes_cb[op].func==NULL) {
        log4me_error(LOG_EMU_Z80, "0x%04X : opcode = 0xCB 0x%02X not implemented\n", pc, op);
        exit(EXIT_FAILURE);
    }

    cpu->R++;
    opcodes_cb[op].func(cpu);
}

#ifdef DEBUG
void dasmopcode_cb(cpuZ80 *cpu, word addr, char *buffer, int len)
{
    int op = (int)read8(cpu, addr);
    dasmopcode(cpu, opcodes_cb[op].dasm, addr+1, buffer, len);
}
#endif /* DEBUG */
