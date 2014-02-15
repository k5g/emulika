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
#include <zip.h>

#include "snapshot.h"

#include "emul.h"
#include "environ.h"
#include "screenshot.h"
#include "sms.h"
#include "misc/log4me.h"
#include "misc/xml.h"

#define SNAPSHOT    "snapshot.xml"
#define SCREENSHOT  "screenshot.bmp"

int issnapshotfile(const char *filename)
{
    if(strcasecmp(strrchr(filename, '.'), ".zip")!=0) return 0;

    struct zip *fzip = NULL;
    int i, numfiles;

    if((fzip = zip_open(filename, ZIP_CHECKCONS, NULL))==NULL) {
        log4me_error(LOG_EMU_SNAPSHOT, "Could not open file : %s\n", filename);
        exit(EXIT_FAILURE);
    }

    numfiles = zip_get_num_files(fzip);
    for(i=0;i<numfiles;i++) {
        filename = zip_get_name(fzip, i, ZIP_FL_UNCHANGED);
        if(strcasecmp(filename, SNAPSHOT)==0)
            break;
    }

    zip_close(fzip);

    return numfiles && (i<numfiles);
}

snapshot *readsnapshot(const char *filename)
{
    xmlDocPtr doc;
    struct zip *fzip = NULL;
    struct zip_stat fstat;
    struct zip_file *file = NULL;
    int i, numfiles;
    char *buffer = NULL;

    if((fzip = zip_open(filename, ZIP_CHECKCONS, NULL))==NULL) {
        log4me_error(LOG_EMU_SNAPSHOT, "Could not open file : %s\n", filename);
        exit(EXIT_FAILURE);
    }

    numfiles = zip_get_num_files(fzip);
    for(i=0;i<numfiles;i++) {
        filename = zip_get_name(fzip, i, ZIP_FL_UNCHANGED);
        if(strcasecmp(filename, SNAPSHOT)==0) {
            assert(buffer==NULL);
            zip_stat_index(fzip, i, 0, &fstat);
            buffer = malloc(fstat.size);

            file = zip_fopen(fzip, fstat.name, ZIP_FL_UNCHANGED);
            zip_fread(file, buffer, fstat.size);
            zip_fclose(file);
        }
    }

    zip_close(fzip);

    if(buffer==NULL) { // XML snapshot not find
        log4me_error(LOG_EMU_SNAPSHOT, "XML snapshot not find in archive : %s\n", filename);
        exit(EXIT_FAILURE);
    }

    doc = xmlReadMemory(buffer, fstat.size, SNAPSHOT, NULL, 0);
    if (doc == NULL) {
        log4me_error(LOG_EMU_SNAPSHOT, "Failed to parse snapshot file in archive : %s\n", filename);
        exit(EXIT_FAILURE);
    }
    free(buffer);

    xmlNode *node = xmlDocGetRootElement(doc);
    assert(strcasecmp("machine", (const char*)node->name)==0);
    assert(node->next==NULL);

    snapshot *snp = calloc(1, sizeof(snapshot));
    snp->doc = doc;
    snp->machine = JAPAN;
    snp->vmode = VM_NTSC;
    snp->romfilename = NULL;

    for(node = node->children; node; node = node->next) {
        XML_ELEMENT_CONTENT("country", node,
            if(xmlStrCaseCmp(content, "japan"))
                snp->machine = JAPAN;
            else
            if(xmlStrCaseCmp(content, "export"))
                snp->machine = EXPORT;
        );

        XML_ELEMENT_CONTENT("videomode", node,
            if(xmlStrCaseCmp(content, "ntsc"))
                snp->vmode = VM_NTSC;
            else
            if(xmlStrCaseCmp(content, "pal"))
                snp->vmode = VM_PAL;
        );

        XML_ELEMENT_CONTENT("romfilename", node,
            snp->romfilename = (char*)content;
        );
    }

    if(!snp->romfilename) {
        log4me_error(LOG_EMU_SNAPSHOT, "Shapshot doesn't contain ROM filename.\n");
        exit(EXIT_FAILURE);
    }

    return snp;
}

void closesnapshot(snapshot *snp)
{
    xmlFreeDoc(snp->doc);
    xmlCleanupParser();
    xmlMemoryDump();
    free(snp);
}

void takesnapshot(mastersystem *sms, const string snapshotsdir)
{
    xmlTextWriterPtr writer;
    string snapshotfilename;

    ms_pause(sms, 1);

    snapshotfilename = pathcombc(gettemppath(), SNAPSHOT);
    writer = xmlNewTextWriterFilename(CSTR(snapshotfilename), 0);
    if (writer == NULL) {
        log4me_error(LOG_EMU_SNAPSHOT, "Can not create snapshot file\n");
        exit(EXIT_FAILURE);
    }
    xmlTextWriterSetIndent(writer, 1);
    xmlTextWriterStartDocument(writer, NULL, NULL, NULL);
    sms_takesnapshot(sms, writer);
    xmlTextWriterEndDocument(writer);
    xmlFreeTextWriter(writer);

    string screenshotfilename = pathcombc(gettemppath(), SCREENSHOT);
    savescreenshot(sms, screenshotfilename);

    string filename = gettimedfilename(sms->romname, NULL);
    string zipfilename = pathcombs(strdups(snapshotsdir), filename);
    straddc(zipfilename, ".zip");

    remove(CSTR(zipfilename));

    struct zip *fzip = NULL;
    struct zip_source *src = NULL;

    if((fzip=zip_open(CSTR(zipfilename), ZIP_CREATE|ZIP_EXCL, NULL))==NULL) {
        log4me_error(LOG_EMU_SNAPSHOT, "Could not create file : %s\n", CSTR(zipfilename));
        exit(EXIT_FAILURE);
    }

    src = zip_source_file(fzip, CSTR(snapshotfilename), (off_t)0, (off_t)0);
    zip_add(fzip, SNAPSHOT, src);

    src = zip_source_file(fzip, CSTR(screenshotfilename), (off_t)0, (off_t)0);
    zip_add(fzip, SCREENSHOT, src);

    zip_close(fzip);
    strfree(zipfilename);

    remove(CSTR(screenshotfilename));
    strfree(screenshotfilename);

    log4me_print("Snaphot %s ! => %s\n", "successful", CSTR(filename));
    remove(CSTR(snapshotfilename));
    strfree(snapshotfilename);
    strfree(filename);

    ms_pause(sms, 0);
}
