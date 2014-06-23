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

#define SMS_HEADER_LEN  16

struct _romspecs {
    string filename;
    string romfilename;
    string digest;

    FILE *f;
    long length;
    long skipheader;
    char *header;

    xmlDocPtr doc; // XML document
    video_mode vmode;
    tmachine machine;

    byte zippedrom;
    byte deleterom;
    memorymapper mmapper;
    gameconsole gconsole;
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

        if((strcasecmp(strrchr(filename, '.'), ".sms")==0) ||
           (strcasecmp(strrchr(filename, '.'), ".gg" )==0)) {
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

static void readheader(romspecs *rom)
{
    assert(rom->f);

    fseek(rom->f, 0, SEEK_END);
    rom->length = ftell(rom->f);

    rom->skipheader = rom->length % MS_PAGE_SIZE;
    rom->length -= rom->skipheader;

    fseek(rom->f, rom->skipheader + (rom->length<=MS_PAGE_SIZE ? 0x3FF0 : 0x7FF0), SEEK_SET);
    rom->header = malloc(SMS_HEADER_LEN);

    if(fread(rom->header, sizeof(char), SMS_HEADER_LEN, rom->f)!=SMS_HEADER_LEN) {
        free(rom->header);
        rom->header = NULL;
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

    free(rom->header);

    if(rom->f)
        fclose(rom->f);

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

memorymapper getrommemorymapper(const romspecs *rom)
{
    return rom->mmapper;
}

gameconsole getromgameconsole(const romspecs *rom)
{
    return rom->gconsole;
}

long getromlength(const romspecs *rom)
{
    return rom->length;
}

size_t readromdata(const romspecs *rom, long offset, byte *buffer, size_t buflen)
{
    fseek(rom->f, rom->skipheader + offset, SEEK_SET);
    return fread(buffer, sizeof(byte), buflen, rom->f);
}

string getromname(const romspecs *rom)
{
    char *rname, *rext;
    char *romfilename = strdupc(rom->romfilename);
    string romname;

    rname = (rname = strrchr(romfilename, CHR_PATH_SEPARATOR))==NULL ? romfilename : rname + 1;
    if((rext = strrchr(rname, '.'))!=NULL) *rext = '\0';

    romname = strcrec(rname);
    free(romfilename);

    return romname;
}

romspecs *getromspecs(const char *filename, tmachine defmachine, video_mode defvmode, int defcodemasters)
{
    romspecs *rom = calloc(1, sizeof(romspecs));
    pushobject(rom, freeromspecs);

    rom->mmapper = MM_SEGA;
    rom->filename = strcrec(filename);
    rom->zippedrom = (strcasecmp(strrchr(filename, '.'), ".zip")==0);
    rom->machine = defmachine;
    rom->vmode = defvmode;
    if(defcodemasters)
        rom->mmapper = MM_CODEMASTERS;

    if(rom->zippedrom)
        readzipfile(rom);
    else
        rom->romfilename = strdups(rom->filename);

    rom->digest = sha1sum(rom->romfilename);

    // Detect game console
    rom->gconsole = GC_UNDEFINED;

    const char *ext = strrchr(CSTR(rom->romfilename), '.');
    if(strcasecmp(ext, ".sms")==0) rom->gconsole = GC_SMS; else
    if(strcasecmp(ext, ".gg" )==0) rom->gconsole = GC_GG;

    // Read header
    if((rom->f = fopen(CSTR(rom->romfilename), "rb"))==NULL) {
        log4me_error(LOG_EMU_MAIN, "File not found : %s\n", CSTR(rom->romfilename));
        exit(EXIT_FAILURE);
    }
    readheader(rom);

    switch(rom->gconsole) {
        case GC_UNDEFINED:
            break;
        case GC_SMS:
            rom->machine = defmachine==UNDEFINED ? JAPAN : defmachine;
            rom->vmode = defvmode==UNDEFINED ? VM_NTSC : defvmode;
            break;
        case GC_GG:
            if(rom->header) {
                // => Sega Master System and Game Gear Architecture by Marat Fayzullin
                // The GG cartridges also have a data byte which contains information on
                // the ROM size and the market for which cartridge has been manufactured.
                // This byte is located at address 7FFFh (3FFFh in page #1 for mapped
                // cartridges) and its bits have the following meaning:
                // bit   Function
                // 7-4   Country Code
                //       0101  Japan
                //       0110  Japan, USA, Europe
                //       0111  USA, Europe
                switch(rom->header[0xF] >> 4) {
                    case 0x5: // Japan
                        rom->machine = JAPAN;
                        break;
                    case 0x6: // Japan, USA, Europe
                    case 0x7: // USA, Europe
                        rom->machine = EXPORT;
                        break;
                    default:
                        rom->machine = JAPAN;
                        break;
                }
            } else
                rom->machine = JAPAN;

            if(defmachine!=UNDEFINED)
                rom->machine = defmachine;

            rom->vmode = VM_NTSC; // Force NTSC with Game Gear games
            break;
    }

    const tmachine machexport = EXPORT;
    const video_mode vmntsc = VM_NTSC;
    const video_mode vmpal = VM_PAL;
    const memorymapper mmcodemasters = MM_CODEMASTERS;
    const memorymapper mmsegaeeprom = MM_SEGA_EEPROM;
    const struct {
        const tmachine *machine;
        const video_mode *vmode;
        const memorymapper *mmapper;
        const char *digest;
    } games[] = {
        { &machexport   , &vmpal    , NULL          , "3fc6ccc556a1e4eb376f77eef8f16b1ff76a17d0" },     /* SMS - Addams Family, The (UE) [!]        */
        { NULL          , NULL      , &mmcodemasters, "f46f716dd34a1a5013a2d8a59769f6ef7536a567" },     /* SMS - Cosmic Spacehead (UE) [!]          */
        { NULL          , NULL      , &mmcodemasters, "4202ce26832046c7ca8209240f097a8a0a84d981" },     /* SMS - Fantastic Dizzy                    */
        { &machexport   , &vmpal    , &mmcodemasters, "425621f350d011fd021850238c6de9999625fd69" },     /* SMS - Micro Machines (E) [!]             */
        { &machexport   , &vmpal    , NULL          , "3bcffd47294f25b25cccb7f42c3a9c3f74333d73" },     /* SMS - Power Strike II (Europe)           */
        { NULL          , &vmntsc   , &mmcodemasters, "b78bc3fe6bfbc7d6f5e85d59d79be19bf7372bcc" },     /* GG  - Drop Zone (U) [!]                  */
        { NULL          , &vmntsc   , &mmsegaeeprom , "76f3c504d717067134a88f7f1d0ef38bd6698e50" },     /* GG  - Majors Pro Baseball (UE) [!]       */
        { NULL          , &vmntsc   , &mmcodemasters, "b167fda9f0e7a6a47027f782a08149e8d4f46e0c" },     /* GG  - Man Overboard (UE) [!]             */
        { NULL          , &vmntsc   , &mmcodemasters, "7b2d39b68622e18eac6c27fd961133b1d002342b" },     /* GG  - Micro Machines                     */
        { NULL          , &vmntsc   , &mmcodemasters, "ab9b4c833207824efb14b712b4ec82721c8436bc" },     /* GG  - Pete Sampras Tennis (E) [!]        */
        { NULL          , &vmntsc   , &mmsegaeeprom , "66d31ed6bb6dfedd769bf5e6c5dcbf899f2f2c8c" },     /* GG  - World Series Baseball '95 (UE) [!] */
        { NULL          , &vmntsc   , &mmsegaeeprom , "333dd99f11781d6de5720043530a460a409d652b" },     /* GG  - World Series Baseball (UE) [a1][!] */
        { NULL          , &vmntsc   , &mmsegaeeprom , "ccfd03edf130f28e6bb4c2764df7ace7bbe9e159" },     /* GG  - World Series Baseball (UE) [!]     */
        { NULL          , NULL      , NULL          , NULL                                       }
    };

    if(rom->digest) {
        int i;
        for(i=0; games[i].digest; i++) {
            if(strcmp(CSTR(rom->digest), games[i].digest)!=0) continue;

            if(games[i].machine!=NULL)
                rom->machine = *games[i].machine;
            if(games[i].vmode!=NULL)
                rom->vmode = *games[i].vmode;
            if(games[i].mmapper!=NULL)
                rom->mmapper = *games[i].mmapper;
        }
    }

    return rom;
}

