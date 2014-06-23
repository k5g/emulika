#ifndef ROM_H_INCLUDED
#define ROM_H_INCLUDED

#include "emul.h"
#include "misc/string.h"

struct _romspecs;
typedef struct _romspecs romspecs;

romspecs *getromspecs(const char *filename, tmachine defmachine, video_mode defvmode, int defcodemasters);

const string getromfilename(const romspecs *rom);
const string getrommainfilename(const romspecs *rom);
tmachine getrommachine(const romspecs *rom);
video_mode getromvideomode(const romspecs *rom);
void *getromsnapshot(const romspecs *rom);
const string getromdigest(const romspecs *rom);
int romhavesnapshot(const romspecs *rom);
memorymapper getrommemorymapper(const romspecs *rom);
gameconsole getromgameconsole(const romspecs *rom);
long getromlength(const romspecs *rom);
size_t readromdata(const romspecs *rom, long offset, byte *buffer, size_t buflen);
string getromname(const romspecs *rom);

#endif // ROM_H_INCLUDED
