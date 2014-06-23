#ifndef SEEPROM_H_INCLUDED
#define SEEPROM_H_INCLUDED

#include "emul.h"
#include "misc/string.h"
#include "misc/xml.h"

struct _seeprom;
typedef struct _seeprom seeprom;

seeprom *seeprom_create(void);
void seeprom_free(seeprom *cpn);

void seeprom_init(seeprom *cpn);

void seeprom_getlines(const seeprom *cpn, byte *cs, byte *clk, byte *dout);
void seeprom_setlines(seeprom *cpn, byte cs, byte clk, byte din);

void seeprom_savedata(const seeprom *cpn, const string filename);
void seeprom_loaddata(seeprom *cpn, const string filename);

void seeprom_takesnapshot(const seeprom *cpn, xmlTextWriterPtr writer);
void seeprom_loadsnapshot(seeprom *cpn, xmlNode *eepromnode);

#endif // SEEPROM_H_INCLUDED
