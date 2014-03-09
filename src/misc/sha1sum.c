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

#include "sha1sum.h"

#include "sha1.h"
#include "types.h"

#define READBUFSIZE 1024

string sha1sum(const string filename)
{
    SHA1_CTX context;
    char digest[SHA1_DIGEST_SIZE*2 + 1];
    byte buffer[READBUFSIZE];
    size_t r;
    int i;
    FILE *f;

    if((f=fopen(CSTR(filename), "rb"))==NULL)
        return NULL;

    SHA1_Init(&context);
    while(!feof(f)) {
        r = fread(&buffer, sizeof(byte), READBUFSIZE, f);
        SHA1_Update(&context, buffer, r);
    }

    SHA1_Final(&context, buffer);
    fclose(f);

    for(i=0; i<SHA1_DIGEST_SIZE; i++)
        sprintf(digest+i*2, "%02x", buffer[i]);

    return strcrec(digest);
}
