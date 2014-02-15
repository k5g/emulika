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

#include <string.h>
#include "xml.h"

#define XML_ARRAY_STEP  0x20

void xmlTextWriterWriteArray(xmlTextWriterPtr writer, char *name, byte *array, int len)
{
    int i, j, l;
    char dump[XML_ARRAY_STEP*2+1];

    xmlTextWriterStartElement(writer, BAD_CAST name);
    for(j=0,l=0;j<len/XML_ARRAY_STEP;j++) {
        xmlTextWriterStartElement(writer, BAD_CAST "dump");
        xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "offset", "%08X", j*XML_ARRAY_STEP);

        for(i=0; (i<XML_ARRAY_STEP) && (l<len); i++,l++,array++) {
            sprintf(dump+i*2, "%02X", *array);
        }
        xmlTextWriterWriteFormatString(writer, "%s", dump);
        xmlTextWriterEndElement(writer);
    }
    xmlTextWriterEndElement(writer);
}

int xmlNodeReadArray(xmlNode *root, byte *buffer, int size)
{
    xmlNode *node;
    const char *content;
    char hex[3] = { 0, 0, 0 };
    int offset = 0;

    for(node=root->children; node; node=node->next) {
        if((node->type!=XML_ELEMENT_NODE)  ||
            (strcasecmp("dump", (const char*)node->name)!=0) ||
            ((content = (const char*)xmlGetProp(node, BAD_CAST "offset"))==NULL)) continue;

        offset = (int)strtoul((const char*)content, NULL, 16);
        content = (const char*)xmlNodeGetContent(node->children);
        while(*content!=0) {
            if(offset>=size) break;
            hex[0] = *content++;
            hex[1] = *content++;
            buffer[offset++] = (byte)strtoul(hex, NULL, 16);
        }
    }
    return offset;
}
