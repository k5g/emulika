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

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#ifndef WIN32
#include <pwd.h>
#else
#include <windows.h>
#include <shlobj.h>
#endif /* WIN32 */

#include "environ.h"
#include "emul.h"
#include "misc/log4me.h"
#include "misc/exit.h"

#define TMP_TEMPLATE	"SMSXXXXXX"

string _tempdir = NULL;

string pathcombs(string s1, const string s2)
{
    if(!endswithchar(s1, CHR_PATH_SEPARATOR)) straddc(s1, STR_PATH_SEPARATOR);
    return stradds(s1, s2);
}

string pathcombc(string s1, const char *s2)
{
    string s = strcrec(s2);
    pathcombs(s1, s);
    strfree(s);
    return s1;
}

static void freetemppath()
{
    remove(CSTR(_tempdir)); // TODO: Directory not remove under MinGW32
    strfree(_tempdir);
}

string gettemppath()
{
#ifdef WIN32
	char tmp[MAX_PATH];
	if(_tempdir==NULL) {
		if(GetTempPath(MAX_PATH, tmp)==0) {
			log4me_error(LOG_EMU_MAIN, "Unable to locate the temporary directory.");
			exit(EXIT_FAILURE);
		}
		_tempdir = pathcombc(strcrec(tmp), TMP_TEMPLATE);
		createdirectory(_tempdir);
	}
	return strdups(_tempdir);
#else
    if(_tempdir==NULL) {
        string dir;
        char *tmppath = getenv("TMP");

        if(tmppath==NULL) tmppath = "/tmp";

        dir = pathcombc(strcrec(tmppath), TMP_TEMPLATE);
        tmppath = strdupc(dir);
        _tempdir = strcrec(mkdtemp(tmppath));
        free(tmppath);
        strfree(dir);
        pushexit(freetemppath);
    }

    return strdups(_tempdir);
#endif /* WIN32 */
}

string gettimedfilename(const char *filename, const char *ext)
{
    time_t timestamp;
    char timestr[32];

    timestamp = time(NULL);
    sprintf(timestr, "%08x", (unsigned int)timestamp);

    string name = strcrec(filename);
    straddc(name, ".");
    straddc(name, timestr);
    if(ext) {
        straddc(name, ".");
        straddc(name, ext);
    }
    return name;
}

string gethomedir()
{
#ifdef WIN32
	TCHAR homedir[MAX_PATH];
	if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, homedir)))
		return strcrec(homedir);

	log4me_error(LOG_EMU_MAIN, "Unable to get user home directory.\n");
    exit(EXIT_FAILURE);
#else
    char *homedir;

    if((homedir=getenv("HOME"))!=NULL) return strcrec(homedir);

    struct passwd *pw = getpwuid(getuid());
    if(pw==NULL) {
        log4me_error(LOG_EMU_MAIN, "Unable to get user home directory.\n");
        exit(EXIT_FAILURE);
    }

    return strcrec(pw->pw_dir);
#endif /* WIN32 */
}

void createdirectory(const string dir)
{
    struct stat st;
    if(stat(CSTR(dir), &st)!=-1) return;

#ifdef WIN32
	if(mkdir(CSTR(dir))==-1) {
#else
    if(mkdir(CSTR(dir), S_IRWXU)==-1) {
#endif /* WIN32 */
        log4me_error(LOG_EMU_MAIN, "Unable to create directory %s.", CSTR(dir));
        exit(EXIT_FAILURE);
    }
}
