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

#include "screenshot.h"
#include "environ.h"

int savescreenshot(const mastersystem *sms, const string filename)
{
    int res;
    SDL_Surface *image = tms9918a_takescreenshot(&sms->vdp);

    res = SDL_SaveBMP(image, CSTR(filename));

    SDL_FreeSurface(image);
    return res;
}

void takescreenshot(const mastersystem *sms, const string screenshotsdir)
{
    string filename = gettimedfilename(CSTR(sms->romname), "bmp");
    string fullfilename = pathcombs(strdups(screenshotsdir), filename);

    int res = savescreenshot(sms, fullfilename);
    log4me_print("Screenshot %s ! => %s\n", res==0 ? "successful" : "failed", CSTR(filename));

    strfree(fullfilename);
    strfree(filename);
}
