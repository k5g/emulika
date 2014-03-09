#ifndef SNAPSHOT_H_INCLUDED
#define SNAPSHOT_H_INCLUDED

#include "sms.h"
#include "misc/string.h"

#define SNAPSHOT    "snapshot.xml"
#define SCREENSHOT  "screenshot.bmp"

void takesnapshot(mastersystem *sms, const string snapshotsdir);

#endif // SNAPSHOT_H_INCLUDED
