#ifndef SNAPSHOT_H_INCLUDED
#define SNAPSHOT_H_INCLUDED

#include "emul.h"
#include "misc/string.h"

struct snapshot_ {
    void *doc; // XML document
    video_mode vmode;
    tmachine machine;
    char *romfilename;
};
typedef struct snapshot_ snapshot;

struct _mastersystem;
typedef struct _mastersystem mastersystem;

int issnapshotfile(const char *filename);
snapshot *readsnapshot(const char *filename);
void closesnapshot(snapshot *snp);
void takesnapshot(mastersystem *sms, const string snapshotsdir);

#endif // SNAPSHOT_H_INCLUDED
