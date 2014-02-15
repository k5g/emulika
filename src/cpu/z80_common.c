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

#include "z80_common.h"

//--------------------------------------------------------------------------------------------------

byte inc8(cpuZ80 *cpu, byte value)
{
    cpu->b.F &= FLAG_CARRY;
    cpu->b.F |= (value==0x7F ? FLAG_OVERFLOW : 0);
    cpu->b.F |= ((value&0x0F)==0x0F ? FLAG_HALF_CARRY : 0);
    cpu->b.F |= flags_sz(++value);
    return value;
}

byte dec8(cpuZ80 *cpu, byte value)
{
    register byte v = value--;
    cpu->b.F &= FLAG_CARRY;
    cpu->b.F |= (FLAG_ADD_SUB | flags_sz(value));
    cpu->b.F |= v==0x80 ? FLAG_OVERFLOW : 0;
    cpu->b.F |= v&0x0F ? 0 : FLAG_HALF_CARRY;
    return value;
}

byte add8(cpuZ80 *cpu, byte v1, byte v2)
{
    word v = v1 + v2;
    cpu->b.F = flags_sz(v & 0xFF) | ((v&0x100) ? FLAG_CARRY : 0); // Sign/Zero/Carry
    //cpu->b.F |= ((v ^ v1 ^ v2) & 0x80) ? FLAG_OVERFLOW : 0; // Overflow
    cpu->b.F |= ((v1 ^ v2 ^ 0x80) & (v1 ^ v) & 0x80) ? FLAG_OVERFLOW : 0; // Overflow
    cpu->b.F |= ((v1 & 0xF) + (v2 & 0xF)) & 0x10; assert(FLAG_HALF_CARRY==0x10); // Half carry
    return v & 0xFF;
}

byte adc8(cpuZ80 *cpu, byte v1, byte v2)
{
    byte carry = cpu->b.F & FLAG_CARRY; assert(FLAG_CARRY==1);
    word v = v1 + v2 + carry;
    cpu->b.F = flags_sz(v & 0xFF) | ((v&0x100) ? FLAG_CARRY : 0); // Sign/Zero/Carry
    cpu->b.F |= ((v1 ^ v2 ^ 0x80) & (v1 ^ v) & 0x80) ? FLAG_OVERFLOW : 0; // Overflow
    cpu->b.F |= ((v1 & 0xF) + (v2 & 0xF) + carry) & 0x10; assert(FLAG_HALF_CARRY==0x10); // Half carry
    return v & 0xFF;
}

byte sub8(cpuZ80 *cpu, byte v1, byte v2)
{
    word v = v1 - v2;
    cpu->b.F = FLAG_ADD_SUB | flags_sz(v & 0xFF) | ((v&0x100) ? FLAG_CARRY : 0); // AddSub/Sign/Zero/Carry
    cpu->b.F |= ((v1 ^ v2) & (v1 ^ v) & 0x80) ? FLAG_OVERFLOW : 0; // Overflow
    cpu->b.F |= ((v1 & 0xF) - (v2 & 0xF)) & 0x10; assert(FLAG_HALF_CARRY==0x10); // Half carry
    return v & 0xFF;
}

byte sbc8(cpuZ80 *cpu, byte v1, byte v2)
{
    byte carry = cpu->b.F & FLAG_CARRY; assert(FLAG_CARRY==1);
    word v = v1 - v2 - carry;
    cpu->b.F = FLAG_ADD_SUB | flags_sz(v & 0xFF) | ((v&0x100) ? FLAG_CARRY : 0); // AddSub/Sign/Zero/Carry
    cpu->b.F |= ((v1 ^ v2) & (v1 ^ v) & 0x80) ? FLAG_OVERFLOW : 0; // Overflow
    cpu->b.F |= ((v1 & 0xF) - (v2 & 0xF) - carry) & 0x10; assert(FLAG_HALF_CARRY==0x10); // Half carry
    return v & 0xFF;
}

byte and8(cpuZ80 *cpu, byte v1, byte v2)
{
    v1 &= v2;
    cpu->b.F = flags_szp(v1) | FLAG_HALF_CARRY;
    return v1;
}

byte xor8(cpuZ80 *cpu, byte v1, byte v2)
{
    v1 ^= v2;
    cpu->b.F = flags_szp(v1);
    return v1;
}

byte or8(cpuZ80 *cpu, byte v1, byte v2)
{
    v1 |= v2;
    cpu->b.F = flags_szp(v1);
    return v1;
}

void bit8(cpuZ80 *cpu, int b, byte value)
{
    // Sean Young documentation / Chap4 Undocumented Effects
    cpu->b.F &= FLAG_CARRY;
    cpu->b.F |= FLAG_HALF_CARRY;
    if((value & (1 << b))!=0) {
        if(b==7) cpu->b.F |= FLAG_SIGN;
    } else
        cpu->b.F |= (FLAG_ZERO | FLAG_PARITY);
}

byte set8(cpuZ80 *cpu, int b, byte value)
{
    return (value | (1 << b));
}

byte res8(cpuZ80 *cpu, int b, byte value)
{
    return (value & ~(1<<b));
}

byte rlc8(cpuZ80 *cpu, byte value)
{
    cpu->b.F = value&0x80 ? FLAG_CARRY : 0;
    value <<= 1;
    value |= (cpu->b.F ? 1 : 0);
    cpu->b.F |= flags_szp(value);
    return value;
}

byte rrc8(cpuZ80 *cpu, byte value)
{
    cpu->b.F = value&0x01 ? FLAG_CARRY : 0;
    value >>= 1;
    value |= (cpu->b.F ? 0x80 : 0);
    cpu->b.F |= flags_szp(value);
    return value;
}

byte srl8(cpuZ80 *cpu, byte value)
{
    cpu->b.F = value & FLAG_CARRY;
    value >>= 1;
    cpu->b.F |= flags_szp(value);
    return value;
}

byte sla8(cpuZ80 *cpu, byte value)
{
    cpu->b.F = (value&0x80 ? FLAG_CARRY : 0);
    value <<= 1;
    cpu->b.F |= flags_szp(value);
    return value;
}

byte sll8(cpuZ80 *cpu, byte value)
{
    cpu->b.F = (value&0x80 ? FLAG_CARRY : 0);
    value <<= 1;
    value |= 1;
    cpu->b.F |= flags_szp(value);
    return value;
}

byte sra8(cpuZ80 *cpu, byte value)
{
    cpu->b.F = (value&0x01 ? FLAG_CARRY : 0);
    if(value&0x80) {
        value >>= 1;
        value |= 0x80;
    } else
        value >>= 1;
    cpu->b.F |= flags_szp(value);
    return value;
}

byte rl8(cpuZ80 *cpu, byte value)
{
    byte carry = cpu->b.F & FLAG_CARRY; assert(FLAG_CARRY==1);
    cpu->b.F = value & 0x80 ? FLAG_CARRY : 0;
    value <<=1;
    value |= carry;
    cpu->b.F |= flags_szp(value);
    return value;
}

byte rr8(cpuZ80 *cpu, byte value)
{
    byte carry = cpu->b.F & FLAG_CARRY;
    cpu->b.F = value & FLAG_CARRY; assert(FLAG_CARRY==1);
    value >>=1;
    if(carry) value |= 0x80;
    cpu->b.F |= flags_szp(value);
    return value;
}

//--------------------------------------------------------------------------------------------------

word add16(cpuZ80 *cpu, word v1, word v2)
{
    dword v = v1 + v2;
    cpu->b.F &= ~(FLAG_HALF_CARRY | FLAG_ADD_SUB | FLAG_CARRY);
    cpu->b.F |= (v&0xFFFF0000 ? FLAG_CARRY : 0);
    cpu->b.F |= (((v1&0x0FFF)+(v2&0x0FFF))&0xF000 ? FLAG_HALF_CARRY : 0);
    return v & 0xFFFF;
}

word sub16(cpuZ80 *cpu, word v1, word v2)
{
    dword v = v1 - v2;
    cpu->b.F = FLAG_ADD_SUB | flags_sz(v & 0xFF) | ((v&0x100) ? FLAG_CARRY : 0); // AddSub/Sign/Zero/Carry
    cpu->b.F |= ((/*v ^*/ v1 ^ v2) & 0x8000) ? FLAG_OVERFLOW : 0; // Overflow
    cpu->b.F |= (((v1&0x0FFF)-(v2&0x0FFF))&0xF000 ? FLAG_HALF_CARRY : 0); // Half carry
    return v & 0xFFFF;
}

word adc16(cpuZ80 *cpu, word v1, word v2)
{
    byte carry = cpu->b.F & FLAG_CARRY; assert(FLAG_CARRY==1);
    dword v = v1 + v2 + carry;
    cpu->b.F = (v&0xFFFF0000 ? FLAG_CARRY : 0);
    cpu->b.F |= ((v1 ^ v2 ^ 0x8000) & (v1 ^ v) & 0x8000) ? FLAG_OVERFLOW : 0; // Overflow
    cpu->b.F |= (((v1&0x0FFF)+(v2&0x0FFF)+carry)&0xF000 ? FLAG_HALF_CARRY : 0);
    v &= 0xFFFF;
    cpu->b.F |= (v==0 ? FLAG_ZERO : 0);
    cpu->b.F |= (v&0x8000 ? FLAG_SIGN : 0);
    return v;
}

word sbc16(cpuZ80 *cpu, word v1, word v2)
{
    byte carry = cpu->b.F & FLAG_CARRY; assert(FLAG_CARRY==1);
    dword v = v1 - v2 - carry;
    cpu->b.F = (v&0xFFFF0000 ? FLAG_CARRY : 0) | FLAG_ADD_SUB;
    cpu->b.F |= ((v1 ^ v2) & (v1 ^ v) & 0x8000) ? FLAG_OVERFLOW : 0; // Overflow
    cpu->b.F |= (((v1&0x0FFF)-(v2&0x0FFF)-carry)&0xF000 ? FLAG_HALF_CARRY : 0);
    v &= 0xFFFF;
    cpu->b.F |= (v==0 ? FLAG_ZERO : 0);
    cpu->b.F |= (v&0x8000 ? FLAG_SIGN : 0);
    return v;
}

//--------------------------------------------------------------------------------------------------

byte in8(cpuZ80 *cpu, byte port)
{
    register byte value = readio(cpu, port);
    cpu->b.F &= FLAG_CARRY;
    cpu->b.F |= flags_szp(value);
    return value;
}

//--------------------------------------------------------------------------------------------------

#ifdef DEBUG
static char *dasmstrrpc(const char *str, const char *search, const char *replace)
{
  static char buffer[64];
  char *p = strstr(str, search);

  assert(p!=NULL);

  strncpy(buffer, str, p-str);
  sprintf(buffer + (p-str), "%s%s", replace, p + strlen(search));

  return buffer;
}

void dasmopcode(cpuZ80 *cpu, const char *dasm, word addr, char *buffer, size_t len)
{
    if((dasm==NULL) || (*dasm==0)) {
        log4me_error(LOG_EMU_Z80, "No DASM information.\n");
        exit(EXIT_FAILURE);
    }

    char *occ;
    char value[16];

    if((occ=strstr(dasm, "#nn"))!=NULL) {
        sprintf(value, "#%04X", read16(cpu, addr));
        strncpy(buffer, dasmstrrpc(dasm, "#nn", value), len);
    } else
    if((occ=strstr(dasm, "#e"))!=NULL) {
        sprintf(value, "#%04X", addr + 1 + sread8(cpu, addr));
        strncpy(buffer, dasmstrrpc(dasm, "#e", value), len);
    }
    else
    if((occ=strstr(dasm, "#d),#n"))!=NULL) {
        sprintf(value, "#%02X),#%02X", read8(cpu, addr), read8(cpu, addr+1));
        strncpy(buffer, dasmstrrpc(dasm, "#d),#n", value), len);
    }
    else
    if((occ=strstr(dasm, "#d"))!=NULL) {
        sprintf(value, "#%02X", read8(cpu, addr));
        strncpy(buffer, dasmstrrpc(dasm, "#d", value), len);
    }
    else
    if((occ=strstr(dasm, "#n"))!=NULL) {
        sprintf(value, "#%02X", read8(cpu, addr));
        strncpy(buffer, dasmstrrpc(dasm, "#n", value), len);
    }
    else
        strncpy(buffer, dasm, len);
}
#endif

//--------------------------------------------------------------------------------------------------
