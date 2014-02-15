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

#include "ym2413.h"

void ym2413_init(ym2413 *cpn)
{
    cpn->data = 0;
}


void ym2413_free()
{

}

void ym2413_write(ym2413 *cpn, byte data)
{
    if(cpn->data==0) {
        cpn->data++;
        log4me_warning(LOG_EMU_YM2413, "YM2413 not implemented\n");
    }
}
