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

/* Serial EEPROM - Micro Chip 93C46 (beta / not tested) */
#include "seeprom.h"

#define READY   1
#define BUSY    0

typedef enum {
    ES_STANDBY,
    ES_WAITING_START,
    ES_READING_COMMAND,
    ES_EXEC_COMMAND,
    ES_READING_DATA,
    ES_WRITING_DATA
} sestate; /* Serial EEPROM state */

struct _seeprom {
    byte cs;
    byte clk;
    byte din, dout;
    byte readonly;
    sestate state;
    byte command;
    word addr, value;
    int shift;
    word data[64];
};

static void seeprom_setdin(seeprom *cpn, byte data)
{
    assert((data==0) || (data==1));
    cpn->din = data;
}

static void eeprom_setcs(seeprom *cpn, byte newcs)
{
    assert((newcs==0) || (newcs==1));

    if(cpn->cs && !newcs) {
        cpn->state = ES_STANDBY;
        cpn->dout = READY;
        cpn->shift = 7;
        cpn->command = 0;
        log4me_debug(LOG_EMU_EEPROM, "Standby mode\n");
    }

    if(!cpn->cs && newcs) {
        cpn->state = ES_WAITING_START;
        cpn->dout = READY;
        log4me_debug(LOG_EMU_EEPROM, "Waiting start bit\n");
    }

    cpn->cs = newcs;
}

static void eeprom_exec(seeprom *cpn)
{
    log4me_debug(LOG_EMU_EEPROM, "Exec command Opcode : 0x%02X | Addr : 0x%02X\n", cpn->command & 0xC0, cpn->command & 0x3F);
    switch(cpn->command & 0xC0) {
        case 0x00:
            switch(cpn->command & 0x30) {
                case 0x00:
                    log4me_warning(LOG_EMU_EEPROM, "EWDS not tested");
                    log4me_debug(LOG_EMU_EEPROM, "EWDS (Erase/Write disable) readonly=1\n");
                    cpn->readonly = 1;
                    break;
                case 0x10:
                    log4me_warning(LOG_EMU_EEPROM, "WRAL not tested");
                    if(!cpn->readonly) {
                        cpn->state = ES_WRITING_DATA;
                        cpn->value = 0;
                        cpn->shift = 15;
                        cpn->dout = READY;
                        log4me_debug(LOG_EMU_EEPROM, "WRAL\n");
                    } else {
                        log4me_warning(LOG_EMU_EEPROM, "Ready only : WRAL not possible.\n");
                        cpn->dout = BUSY;
                    }
                    break;
                case 0x20:
                    log4me_warning(LOG_EMU_EEPROM, "ERASE ALL not tested");
                    log4me_debug(LOG_EMU_EEPROM, "ERAL\n");
                    if(!cpn->readonly) {
                        memset(cpn->data, 0xFF, sizeof(cpn->data));
                        cpn->dout = READY;
                    } else {
                        log4me_warning(LOG_EMU_EEPROM, "Ready only : ERAL not possible.\n");
                        cpn->dout = BUSY;
                    }
                    break;
                case 0x30:
                    log4me_debug(LOG_EMU_EEPROM, "EWEN (Erase/Write enable) readonly=0\n");
                    cpn->readonly = 0;
                    break;
            }
            break;
        case 0x40:
            if(!cpn->readonly) {
                cpn->state = ES_WRITING_DATA;
                cpn->addr = cpn->command & 0x3F;
                cpn->value = 0;
                cpn->shift = 15;
                cpn->dout = READY;
                log4me_debug(LOG_EMU_EEPROM, "WRITE addr=0x%04X\n", cpn->addr);
            } else {
                cpn->dout = BUSY;
                log4me_warning(LOG_EMU_EEPROM, "Ready only : WRITE not possible.\n");
            }
            break;
        case 0x80:
            cpn->state = ES_READING_DATA;
            cpn->addr = cpn->command & 0x3F;
            cpn->shift = 15;
            log4me_debug(LOG_EMU_EEPROM, "READ addr=0x%04X\n", cpn->addr);
            break;
        case 0xC0:
            if(!cpn->readonly) {
                cpn->addr = cpn->command & 0x3F;
                cpn->data[cpn->addr] = 0xFFFF;
                log4me_debug(LOG_EMU_EEPROM, "ERASE addr=0x%04X\n", cpn->addr);
            } else
                log4me_warning(LOG_EMU_EEPROM, "Ready only : ERASE not possible.\n");
            break;
    }
}

static void eeprom_setclk(seeprom *cpn, byte newclk)
{
    assert((newclk==0) || (newclk==1));

    switch(cpn->state) {
        case ES_STANDBY:
        case ES_EXEC_COMMAND:
            break;
        case ES_WAITING_START:
            if(cpn->cs && !cpn->clk && newclk && cpn->din) {
                cpn->state = ES_READING_COMMAND;
                assert((cpn->shift==7) && (cpn->command==0));
            }
            break;
        case ES_READING_COMMAND:
            if(cpn->cs && !cpn->clk && newclk) {
                cpn->command |= (cpn->din << cpn->shift);
                if(--cpn->shift<0) {
                    cpn->state = ES_EXEC_COMMAND;
                    eeprom_exec(cpn);
                }
            }
            break;
        case ES_READING_DATA:
            if(cpn->cs && !cpn->clk && newclk) {
                assert(cpn->shift>=0);
                cpn->dout = ((cpn->data[cpn->addr] & (1 << cpn->shift)) != 0);
                log4me_debug(LOG_EMU_EEPROM, " bit %d : %d\n", cpn->shift, cpn->dout);
                cpn->shift--;
            }
            break;
        case ES_WRITING_DATA:
            if(cpn->cs && !cpn->clk && newclk) {
                assert(cpn->shift>=0);
                cpn->value |= (cpn->din << cpn->shift);
                if(cpn->shift==0) {
                    if((cpn->command & 0xC0)==0x40)
                        cpn->data[cpn->addr] = cpn->value;
                    if((cpn->command & 0xF0)==0x10) {
                        int i;
                        for(i=0;i<64;i++) cpn->data[i] = cpn->value;
                    }
                }
                log4me_debug(LOG_EMU_EEPROM, " bit %d : %d\n", cpn->shift, cpn->din);
                cpn->shift--;
            }
            break;
    }

    cpn->clk = newclk;
}

void seeprom_getlines(const seeprom *cpn, byte *cs, byte *clk, byte *dout)
{
    if(cs) *cs = cpn->cs;
    if(clk) *clk = cpn->clk;
    if(dout) *dout = cpn->dout;
}

void seeprom_setlines(seeprom *cpn, byte cs, byte clk, byte din)
{
    seeprom_setdin(cpn, din);
    eeprom_setcs(cpn, cs);
    eeprom_setclk(cpn, clk);
}

void seeprom_init(seeprom *cpn)
{
    log4me_debug(LOG_EMU_EEPROM, "initialize\n");
    cpn->cs = 0;
    cpn->clk = 0;
    cpn->din = 0;
    cpn->dout = BUSY;
    cpn->state = ES_STANDBY;
    cpn->readonly = 1;
    memset(cpn->data, 0xFF, sizeof(cpn->data));
}

void seeprom_free(seeprom *cpn)
{
    if(cpn!=NULL) free(cpn);
}

seeprom *seeprom_create()
{
    seeprom *e = calloc(1, sizeof(seeprom));
    seeprom_init(e);
    return e;
}

void seeprom_savedata(const seeprom *cpn, const string filename)
{
    FILE *f;

    if((f = fopen(CSTR(filename), "wb"))==NULL) {
        log4me_error(LOG_EMU_EEPROM, "Save EEPROM data failed !\n");
        return;
    }
    fwrite(cpn->data, sizeof(cpn->data), 1, f);
    fclose(f);
}

void seeprom_loaddata(seeprom *cpn, const string filename)
{
    FILE *f;

    if((f = fopen(CSTR(filename), "rb"))==NULL) {
        //log4me_error(LOG_EMU_SMS, "Load EEPROM data failed !\n");
        return;
    }
    if(fread(cpn->data, sizeof(cpn->data), 1, f)!=sizeof(cpn->data)) /* do nothing :) */;
    fclose(f);
}

void seeprom_takesnapshot(const seeprom *cpn, xmlTextWriterPtr writer)
{
    xmlTextWriterStartElement(writer, BAD_CAST "eeprom");
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "model", "%s", "93c46");

        xmlTextWriterStartElement(writer, BAD_CAST "chipselect");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->cs);
        xmlTextWriterEndElement(writer); /* chipselect */

        xmlTextWriterStartElement(writer, BAD_CAST "clock");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->clk);
        xmlTextWriterEndElement(writer); /* clock */

        xmlTextWriterStartElement(writer, BAD_CAST "datain");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->din);
        xmlTextWriterEndElement(writer); /* datain */

        xmlTextWriterStartElement(writer, BAD_CAST "dataout");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->dout);
        xmlTextWriterEndElement(writer); /* dataout */

        xmlTextWriterStartElement(writer, BAD_CAST "readonly");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->readonly);
        xmlTextWriterEndElement(writer); /* readonly */

        xmlTextWriterStartElement(writer, BAD_CAST "state");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->state);
        xmlTextWriterEndElement(writer); /* state */

        xmlTextWriterStartElement(writer, BAD_CAST "command");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->command);
        xmlTextWriterEndElement(writer); /* command */

        xmlTextWriterStartElement(writer, BAD_CAST "address");
        xmlTextWriterWriteFormatString(writer, "%04X", cpn->addr);
        xmlTextWriterEndElement(writer); /* address */

        xmlTextWriterStartElement(writer, BAD_CAST "value");
        xmlTextWriterWriteFormatString(writer, "%04X", cpn->value);
        xmlTextWriterEndElement(writer); /* value */

        xmlTextWriterStartElement(writer, BAD_CAST "shift");
        xmlTextWriterWriteFormatString(writer, "%02X", cpn->shift);
        xmlTextWriterEndElement(writer); /* shift */

        xmlTextWriterWriteArray(writer, "data", (byte*)cpn->data, sizeof(cpn->data));

    xmlTextWriterEndElement(writer); /* eeprom */
}

void seeprom_loadsnapshot(seeprom *cpn, xmlNode *eepromnode)
{
    XML_ENUM_CHILD(eepromnode, node,

        XML_ELEMENT_CONTENT("chipselect", node,
            cpn->cs = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("clock", node,
            cpn->clk = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("datain", node,
            cpn->din = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("dataout", node,
            cpn->dout = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("readonly", node,
            cpn->readonly = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("state", node,
            cpn->state = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("command", node,
            cpn->command = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("address", node,
            cpn->addr = (word)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("value", node,
            cpn->value = (word)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("shift", node,
            cpn->shift = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT("data", node,
            xmlNodeReadArray(node, (byte*)cpn->data, sizeof(cpn->data));
        )
    )
}
