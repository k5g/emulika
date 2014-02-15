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
#include <string.h>

#include "string.h"

struct _string {
    char *str;
    size_t len; // without '\0'
};

void strfree(string s)
{
    if(s) {
        free(s->str);
        free(s);
    }
}

string straddc(string s1, const char *s2)
{
    s1->len += strlen(s2);
    s1->str = realloc(s1->str, s1->len + 1);
    strcat(s1->str, s2);
    return s1;
}

string stradds(string s1, const string s2)
{
    s1->len += s2->len;
    s1->str = realloc(s1->str, s1->len + 1);
    strcat(s1->str, s2->str);
    return s1;
}

const char *strgetc(const string s)
{
    return s->str;
}

char *strdupc(const string s)
{
    char *sr = calloc(s->len + 1, sizeof(char));
    strcpy(sr, s->str);
    return sr;
}

string strdups(const string str)
{
    string s = malloc(sizeof(*s));
    s->len = str->len;
    s->str = calloc(s->len + 1, sizeof(char));
    strcpy(s->str, str->str);
    return s;
}

int endswithchar(const string s, char c)
{
    if(s->len==0) return 0;
    return (s->str[s->len-1]==c);
}

int strlens(const string s)
{
    return s->len;
}

string strcrec(const char *str)
{
    string s = malloc(sizeof(*s));
    s->len = strlen(str);
    s->str = calloc(s->len + 1, sizeof(char));
    strcpy(s->str, str);
    return s;
}

string strcren()
{
    return strcrec("");
}

