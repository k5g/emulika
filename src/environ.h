#ifndef ENVIRON_H_INCLUDED
#define ENVIRON_H_INCLUDED

#include "misc/string.h"

#ifdef WIN32
#define CHR_PATH_SEPARATOR  '\\'
#define STR_PATH_SEPARATOR  "\\"
#else
#define CHR_PATH_SEPARATOR  '/'
#define STR_PATH_SEPARATOR  "/"
#endif

string pathcombs(string s1, const string s2);
string pathcombc(string s1, const char *s2);

string gettemppath(void);
string gettimedfilename(const char *filename, const char *ext);

string gethomedir(void);
void createdirectory(const string dir);

#endif // ENVIRON_H_INCLUDED
