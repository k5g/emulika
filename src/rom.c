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

#include <stdlib.h>
#include <zip.h>

#include "environ.h"
#include "rom.h"
#include "snapshot.h"
#include "misc/exit.h"
#include "misc/sha1sum.h"
#include "misc/string.h"
#include "misc/xml.h"

struct _romspecs {
    string filename;
    string romfilename;
    string digest;

    xmlDocPtr doc; // XML document
    video_mode vmode;
    tmachine machine;

    byte zippedrom;
    byte deleterom;
    segalicense license;
};

static int fzip2buffer(struct zip *fzip, int index, byte **buffer)
{
    struct zip_stat fstat;
    struct zip_file *file = NULL;

    *buffer = NULL;

    zip_stat_index(fzip, index, 0, &fstat);
    *buffer = malloc(fstat.size);

    // Read zipped file
    if((file = zip_fopen(fzip, fstat.name, ZIP_FL_UNCHANGED))==NULL)
        goto error;
    if(zip_fread(file, *buffer, fstat.size)!=fstat.size)
        goto error;
    zip_fclose(file); file = NULL;

    return fstat.size;

error:
    free(*buffer); *buffer = NULL;
    if(file)
        zip_fclose(file);

    return -1;
}

static int fzip2file(struct zip *fzip, int index, const string filename)
{
    FILE *f = NULL;
    byte *buffer = NULL;
    size_t buflen;

    if((buflen=fzip2buffer(fzip, index, &buffer))<0)
        goto error;

    // Write unzipped file
    if((f = fopen(CSTR(filename), "wb"))==NULL)
        goto error;
    fwrite(buffer, buflen, 1, f);
    fclose(f); f = NULL;

    free(buffer);
    return 1;

error:
    free(buffer);
    if(f)
        fclose(f);

    return 0;
}

static int readzippedrom(struct zip *fzip, int index, romspecs *rspecs)
{
    const char *filename;

    filename = zip_get_name(fzip, index, ZIP_FL_UNCHANGED);
    rspecs->romfilename = pathcombc(gettemppath(), filename);
    rspecs->deleterom = 1;

    return fzip2file(fzip, index, rspecs->romfilename);
}

static int readzippedsnapshot(struct zip *fzip, int index, romspecs *rspecs)
{
    int buflen;
    byte *buffer = NULL;

    if((buflen=fzip2buffer(fzip, index, &buffer))<0)
        return 0;

    rspecs->doc = xmlReadMemory((char*)buffer, buflen, SNAPSHOT, NULL, 0);

    free(buffer);
    if(rspecs->doc==NULL)
        return 0;

    xmlNode *node = xmlDocGetRootElement((xmlDocPtr)rspecs->doc);
    assert(strcasecmp("machine", (const char*)node->name)==0);
    assert(node->next==NULL);

    for(node = node->children; node; node = node->next) {
        XML_ELEMENT_CONTENT("country", node,
            if(xmlStrCaseCmp(content, "japan"))
                rspecs->machine = JAPAN;
            else
            if(xmlStrCaseCmp(content, "export"))
                rspecs->machine = EXPORT;
        );

        XML_ELEMENT_CONTENT("videomode", node,
            if(xmlStrCaseCmp(content, "ntsc"))
                rspecs->vmode = VM_NTSC;
            else
            if(xmlStrCaseCmp(content, "pal"))
                rspecs->vmode = VM_PAL;
        );

        XML_ELEMENT_CONTENT("romfilename", node,
            rspecs->romfilename = strcrec((char*)content);
        );

        // Read disgest
    }

    return 1;
}

static void readzipfile(romspecs *rspecs)
{
    struct zip *fzip = NULL;
    const char *filename;
    int i, numfiles;
    int error = 0;
    int findrom = 0;
    int snapshot = 0;

    if((fzip = zip_open(CSTR(rspecs->filename), ZIP_CHECKCONS, NULL))==NULL) {
        log4me_error(LOG_EMU_SMS, "Could not open file : %s\n", CSTR(rspecs->filename));
        exit(EXIT_FAILURE);
    }

    numfiles = zip_get_num_files(fzip);
    for(i=0; i<numfiles; i++) {
        filename = zip_get_name(fzip, i, ZIP_FL_UNCHANGED);

        if(strcasecmp(strrchr(filename, '.'), ".sms")==0) {
            findrom = 1;
            error |= (readzippedrom(fzip, i, rspecs)==0);
            break;
        }

        if(strcasecmp(filename, SNAPSHOT)==0) {
            snapshot = 1;
            error |= (readzippedsnapshot(fzip, i, rspecs)==0);
            findrom = (rspecs->romfilename!=NULL);
            break;
        }
    }

    zip_close(fzip);

    if(error) {
        log4me_error(LOG_EMU_MAIN, snapshot ? "Failed to parse snapshot file in the archive : %s\n" :
            "Unable to extract files from the zip archive : %s\n", CSTR(rspecs->filename));
        exit(EXIT_FAILURE);
    }

    if(!findrom) {
        log4me_error(LOG_EMU_MAIN, "Could not find ROM or snapshot in the zip archive : %s\n", CSTR(rspecs->filename));
        exit(EXIT_FAILURE);
    }

    if(snapshot && (strcasecmp(strrchr(CSTR(rspecs->romfilename), '.'), ".zip")==0)) {
        strfree(rspecs->filename);
        rspecs->filename = rspecs->romfilename;
        rspecs->romfilename = NULL;
        readzipfile(rspecs);
    }
}

static void freeromspecs(romspecs *rom)
{
    if(rom->deleterom) {
        assert(rom->zippedrom && rom->romfilename);
        remove(CSTR(rom->romfilename));
    }

    if(rom->doc) {
        xmlFreeDoc(rom->doc);
        xmlCleanupParser();
        xmlMemoryDump();
    }

    strfree(rom->digest);
    strfree(rom->romfilename);
    strfree(rom->filename);
}

const string getromfilename(const romspecs *rom)
{
    return rom->romfilename;
}

const string getrommainfilename(const romspecs *rom)
{
    return rom->filename;
}

tmachine getrommachine(const romspecs *rom)
{
    return rom->machine;
}

video_mode getromvideomode(const romspecs *rom)
{
    return rom->vmode;
}

int romhavesnapshot(const romspecs *rom)
{
    return rom->doc!=NULL;
}

void *getromsnapshot(const romspecs *rom)
{
    return rom->doc;
}

const string getromdigest(const romspecs *rom)
{
    return rom->digest;
}

segalicense getromlicense(const romspecs *rom)
{
    return rom->license;
}

romspecs *getromspecs(const char *filename, tmachine defmachine, video_mode defvmode, int defcodemasters)
{
    romspecs *rom = calloc(1, sizeof(romspecs));
    pushobject(rom, freeromspecs);

    rom->license = SL_SEGA;
    rom->filename = strcrec(filename);
    rom->zippedrom = (strcasecmp(strrchr(filename, '.'), ".zip")==0);
    rom->machine = defmachine;
    rom->vmode = defvmode;
    if(defcodemasters)
        rom->license = SL_CODEMASTERS;

    if(rom->zippedrom)
        readzipfile(rom);
    else
        rom->romfilename = strdups(rom->filename);

    rom->digest = sha1sum(rom->romfilename);

    if(rom->digest && (
      (strcmp(CSTR(rom->digest), "f46f716dd34a1a5013a2d8a59769f6ef7536a567")==0) /* Cosmic Spacehead (UE) [!] */ ||
      (strcmp(CSTR(rom->digest), "425621f350d011fd021850238c6de9999625fd69")==0) /* Micro Machines (E) [!] */ ||
      (strcmp(CSTR(rom->digest), "4202ce26832046c7ca8209240f097a8a0a84d981")==0) /* Fantastic Dizzy */ ))
        rom->license = SL_CODEMASTERS;

    return rom;
}

