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

static void rlc_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x06 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, rlc8(cpu, read8(cpu, addr)));
}

static void rrc_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x0E */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, rrc8(cpu, read8(cpu, addr)));
}

static void rl_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x16 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, rl8(cpu, read8(cpu, addr)));
}

static void rr_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x1E */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, rr8(cpu, read8(cpu, addr)));
}

static void sla_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x26 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, sla8(cpu, read8(cpu, addr)));
}

static void sra_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x2E */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, sra8(cpu, read8(cpu, addr)));
}

static void sll_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x36 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, sll8(cpu, read8(cpu, addr)));
}

static void srl_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x3E */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, srl8(cpu, read8(cpu, addr)));
}

static void bit_0_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x46 */
{
    cpu->cycles += 20;
    bit8(cpu, 0, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void bit_1_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x4E */
{
    cpu->cycles += 20;
    bit8(cpu, 1, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void bit_2_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x56 */
{
    cpu->cycles += 20;
    bit8(cpu, 2, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void bit_3_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x5E */
{
    cpu->cycles += 20;
    bit8(cpu, 3, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void bit_4_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x66 */
{
    cpu->cycles += 20;
    bit8(cpu, 4, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void bit_5_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x6E */
{
    cpu->cycles += 20;
    bit8(cpu, 5, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void bit_6_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x76 */
{
    cpu->cycles += 20;
    bit8(cpu, 6, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void bit_7_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x7E */
{
    cpu->cycles += 20;
    bit8(cpu, 7, read8(cpu, cpu->IY+sread8(cpu, cpu->PC++)));
}

static void res_0_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x86 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, res8(cpu, 0, read8(cpu, addr)));
}

static void res_1_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x8E */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, res8(cpu, 1, read8(cpu, addr)));
}

static void res_2_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x96 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, res8(cpu, 2, read8(cpu, addr)));
}

static void res_3_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0x9E */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, res8(cpu, 3, read8(cpu, addr)));
}

static void res_4_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xA6 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, res8(cpu, 4, read8(cpu, addr)));
}

static void res_5_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xAE */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, res8(cpu, 5, read8(cpu, addr)));
}

static void res_6_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xB6 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, res8(cpu, 6, read8(cpu, addr)));
}

static void res_7_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xBE */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, res8(cpu, 7, read8(cpu, addr)));
}

static void set_0_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xC6 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, set8(cpu, 0, read8(cpu, addr)));
}

static void set_1_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xCE */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, set8(cpu, 1, read8(cpu, addr)));
}

static void set_2_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xD6 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, set8(cpu, 2, read8(cpu, addr)));
}

static void set_3_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xDE */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, set8(cpu, 3, read8(cpu, addr)));
}

static void set_4_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xE6 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, set8(cpu, 4, read8(cpu, addr)));
}

static void set_5_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xEE */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, set8(cpu, 5, read8(cpu, addr)));
}

static void set_6_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xF6 */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, set8(cpu, 6, read8(cpu, addr)));
}

static void set_7_miyd(cpuZ80 *cpu) /* 0xFD 0xCB d 0xFE */
{
    cpu->cycles += 23;
    word addr = cpu->IY + sread8(cpu, cpu->PC++);
    write8(cpu, addr, set8(cpu, 7, read8(cpu, addr)));
}


opcode opcodes_fdcb[256] = {
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x00 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x01 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x02 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x03 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x04 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x05 */ },
    { rlc_miyd,     DASM("RLC (IY+#d)") /* 0xFD 0xCB d 0x06 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x07 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x08 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x09 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x0A */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x0B */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x0C */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x0D */ },
    { rrc_miyd,     DASM("RRC (IY+#d)") /* 0xFD 0xCB d 0x0E */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x0F */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x10 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x11 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x12 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x13 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x14 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x15 */ },
    { rl_miyd,      DASM("RL (IY+#d)")  /* 0xFD 0xCB d 0x16 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x17 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x18 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x19 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x1A */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x1B */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x1C */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x1D */ },
    { rr_miyd,      DASM("RR (IY+#d)")  /* 0xFD 0xCB d 0x1E */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x1F */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x20 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x21 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x22 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x23 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x24 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x25 */ },
    { sla_miyd,     DASM("SLA (IY+#d)") /* 0xFD 0xCB d 0x26 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x27 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x28 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x29 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x2A */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x2B */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x2C */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x2D */ },
    { sra_miyd,     DASM("SRA (IY+#d)") /* 0xFD 0xCB d 0x2E */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x2F */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x30 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x31 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x32 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x33 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x34 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x35 */ },
    { sll_miyd,     DASM("SLL (IY+#d)") /* 0xFD 0xCB d 0x36 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x37 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x38 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x39 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x3A */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x3B */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x3C */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x3D */ },
    { srl_miyd,     DASM("SRL (IY+#d)") /* 0xFD 0xCB d 0x3E */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x3F */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x40 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x41 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x42 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x43 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x44 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x45 */ },
    { bit_0_miyd,   DASM("BIT 0,(IY+#d)")/* 0xFD 0xCB d 0x46 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x47 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x48 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x49 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x4A */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x4B */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x4C */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x4D */ },
    { bit_1_miyd,   DASM("BIT 1,(IY+#d)")/* 0xFD 0xCB d 0x4E */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x4F */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x50 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x51 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x52 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x53 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x54 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x55 */ },
    { bit_2_miyd,   DASM("BIT 2,(IY+#d)")/* 0xFD 0xCB d 0x56 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x57 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x58 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x59 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x5A */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x5B */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x5C */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x5D */ },
    { bit_3_miyd,   DASM("BIT 3,(IY+#d)")/* 0xFD 0xCB d 0x5E */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x5F */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x60 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x61 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x62 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x63 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x64 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x65 */ },
    { bit_4_miyd,   DASM("BIT 4,(IY+#d)")/* 0xFD 0xCB d 0x66 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x67 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x68 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x69 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x6A */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x6B */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x6C */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x6D */ },
    { bit_5_miyd,   DASM("BIT 5,(IY+#d)")/* 0xFD 0xCB d 0x6E */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x6F */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x70 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x71 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x72 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x73 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x74 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x75 */ },
    { bit_6_miyd,   DASM("BIT 6,(IY+#d)")/* 0xFD 0xCB d 0x76 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x77 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x78 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x79 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x7A */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x7B */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x7C */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x7D */ },
    { bit_7_miyd,   DASM("BIT 7,(IY+#d)")/* 0xFD 0xCB d 0x7E */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x7F */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x80 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x81 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x82 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x83 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x84 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x85 */ },
    { res_0_miyd,   DASM("RES 0,(IY+#d)")/* 0xFD 0xCB d 0x86 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x87 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x88 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x89 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x8A */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x8B */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x8C */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x8D */ },
    { res_1_miyd,   DASM("RES 1,(IY+#d)")/* 0xFD 0xCB d 0x8E */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x8F */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x90 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x91 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x92 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x93 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x94 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x95 */ },
    { res_2_miyd,   DASM("RES 2,(IY+#d)")/* 0xFD 0xCB d 0x96 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x97 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x98 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x99 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x9A */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x9B */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x9C */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x9D */ },
    { res_3_miyd,   DASM("RES 3,(IY+#d)")/* 0xFD 0xCB d 0x9E */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0x9F */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xA0 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xA1 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xA2 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xA3 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xA4 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xA5 */ },
    { res_4_miyd,   DASM("RES 4,(IY+#d)")/* 0xFD 0xCB d 0xA6 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xA7 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xA8 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xA9 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xAA */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xAB */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xAC */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xAD */ },
    { res_5_miyd,   DASM("RES 5,(IY+#d)")/* 0xFD 0xCB d 0xAE */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xAF */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xB0 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xB1 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xB2 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xB3 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xB4 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xB5 */ },
    { res_6_miyd,   DASM("RES 6,(IY+#d)")/* 0xFD 0xCB d 0xB6 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xB7 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xB8 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xB9 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xBA */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xBB */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xBC */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xBD */ },
    { res_7_miyd,   DASM("RES 7,(IY+#d)")/* 0xFD 0xCB d 0xBE */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xBF */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xC0 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xC1 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xC2 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xC3 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xC4 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xC5 */ },
    { set_0_miyd,   DASM("SET 0,(IY+#d)")/* 0xFD 0xCB d 0xC6 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xC7 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xC8 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xC9 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xCA */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xCB */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xCC */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xCD */ },
    { set_1_miyd,   DASM("SET 1,(IY+#d)")/* 0xFD 0xCB d 0xCE */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xCF */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xD0 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xD1 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xD2 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xD3 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xD4 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xD5 */ },
    { set_2_miyd,   DASM("SET 2,(IY+#d)")/* 0xFD 0xCB d 0xD6 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xD7 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xD8 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xD9 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xDA */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xDB */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xDC */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xDD */ },
    { set_3_miyd,   DASM("SET 3,(IY+#d)")/* 0xFD 0xCB d 0xDE */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xDF */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xE0 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xE1 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xE2 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xE3 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xE4 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xE5 */ },
    { set_4_miyd,   DASM("SET 4,(IY+#d)")/* 0xFD 0xCB d 0xE6 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xE7 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xE8 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xE9 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xEA */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xEB */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xEC */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xED */ },
    { set_5_miyd,   DASM("SET 5,(IY+#d)")/* 0xFD 0xCB d 0xEE */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xEF */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xF0 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xF1 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xF2 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xF3 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xF4 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xF5 */ },
    { set_6_miyd,   DASM("SET 6,(IY+#d)")/* 0xFD 0xCB d 0xF6 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xF7 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xF8 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xF9 */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xFA */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xFB */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xFC */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xFD */ },
    { set_7_miyd,   DASM("SET 7,(IY+#d)")/* 0xFD 0xCB d 0xFE */ },
    { NULL,         DASM("")            /* 0xFD 0xCB d 0xFF */ }
};

void opcode_fdcb(cpuZ80 *cpu)
{
    word pc = cpu->PC-2;

    int op = (int)read8(cpu, cpu->PC+1);

    if(opcodes_fdcb[op].func==NULL) {
        log4me_error(LOG_EMU_Z80, "0x%04X : opcode = 0xFD 0xCB d 0x%02X not implemented\n", pc, op);
        exit(EXIT_FAILURE);
    }

    opcodes_fdcb[op].func(cpu);
    cpu->PC++;
    //cpu->R++;
}

#ifdef DEBUG
void dasmopcode_fdcb(cpuZ80 *cpu, word addr, char *buffer, int len)
{
    int op = (int)read8(cpu, addr+1);
    dasmopcode(cpu, opcodes_fdcb[op].dasm, addr, buffer, len);
}
#endif /* DEBUG */
