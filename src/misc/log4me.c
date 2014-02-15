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

#include <stdio.h>
#include <stdarg.h>

#include "log4me.h"
#include "exit.h"

uint64_t _items;
FILE *_rstdout = NULL;

void log4me_free()
{
    _items = 0;
	if(_rstdout!=stdout)
		fclose(_rstdout);
}

void log4me_init(uint64_t items)
{
    _items = items;
	_rstdout = stdout;

	pushexit(log4me_free);
}

void log4me_redirectoutput(const char *fileout)
{
	FILE *f;
	if((f = fopen(fileout, "wt"))!=NULL)
		_rstdout = f;
}

void log4me_print(const char *s, ...)
{
    va_list argp;

    va_start(argp, s);
	vfprintf(_rstdout, s, argp);
    va_end(argp);
}

void log4me_info(uint64_t mask, const char *module, const char *s, ...)
{
    va_list argp;

    fprintf(_rstdout, "[%s] ", module);

    va_start(argp, s);
    vfprintf(_rstdout, s, argp);
    va_end(argp);
}

void log4me_error(uint64_t mask, const char *module, const char *s, ...)
{
    va_list argp;
    FILE *ferr = _rstdout==stdout ? stderr : _rstdout;

    fprintf(ferr, "[%s] ", module);

    va_start(argp, s);
    vfprintf(ferr, s, argp);
    va_end(argp);
}

#ifdef DEBUG
void _log4me_warning(uint64_t mask, const char *module, const char *s, ...)
{
    va_list argp;

    fprintf(_rstdout, "[%s] ", module);

    va_start(argp, s);
    vfprintf(_rstdout, s, argp);
    va_end(argp);
}

void _log4me_debug(uint64_t mask, const char *module, const char *s, ...)
{
    va_list argp;

    if(_items & mask) {
        fprintf(_rstdout, "[%s] ", module);

        va_start(argp, s);
        vfprintf(_rstdout, s, argp);
        va_end(argp);
    }
}

int log4me_enabled(uint64_t mask)
{
    return _items & mask ? 1 : 0;
}
#endif
