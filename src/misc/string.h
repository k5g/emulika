#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#if !defined(HAVE_STRCASECMP) && defined(HAVE__STRCMPI)
#define strcasecmp	_strcmpi
#endif /* */

#define CSTR(s) strgetc((s))

struct _string;
typedef struct _string* string;

const char *strgetc(const string s);
int endswithchar(const string s, char c);
int strlens(const string s);

char *strdupc(const string s);
string strdups(const string s);

string straddc(string s1, const char *s2);
string stradds(string s1, const string s2);

void strfree(string s);

string strcrec(const char *str);
string strcren(void);

#endif // STRING_H_INCLUDED
