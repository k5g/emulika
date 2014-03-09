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
#include "misc/log4me.h"
#include "misc/xml.h"

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
