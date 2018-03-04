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

#include "emuconfig.h"
#include "emul.h"
#include "misc/list.h"
#include "misc/log4me.h"
#include "misc/xml.h"

#define SET_PROFILE_PROP(profile, prop, value) { \
    switch(profile->base.type) { \
        case INP_KEYBOARD: \
            profile->kbd.prop = SDL_GetScancodeFromName((value)); \
            break; \
        case INP_JOYPAD: \
            profile->pad.prop = strtoul((value), NULL, 16); \
            break; \
    } \
}

void initconfig(emuconfig *config)
{
    config->fullscreen = 0;
    config->noaspectratio = 0;
    config->scale = DEFAULT_SCALE;
    config->bezelfilename = NULL;
    config->overlayfilename = NULL;
    config->volume = 100;
    config->nosound = 0;
}

void readconfig(const string configfilename, emuconfig *config, iprofile **player1, iprofile **player2)
{
    xmlDocPtr doc;
    glist *listprofiles;
    iprofile *profile;
    string player1profilename = NULL;
    string player2profilename = NULL;
    int i;

    assert(player1);
    assert(player2);

    *player1 = *player2 = NULL;

    doc = xmlReadFile(CSTR(configfilename), NULL, 0);
    if(doc==NULL) {
        log4me_error(LOG_EMU_MAIN, "Unable to open config file : %s.\n", CSTR(configfilename));
        exit(EXIT_FAILURE);
    }

    listprofiles = lcreate();

    xmlNode *rootnode = xmlDocGetRootElement(doc);
    assert(strcasecmp("config", (const char*)rootnode->name)==0);
    assert(rootnode->next==NULL);

    XML_ENUM_CHILD(rootnode, node,
        XML_ELEMENT_ENUM_CHILD("input_profiles", node, profile,
            XML_ELEMENT("profile", profile,
                xmlChar *name = xmlGetProp(profile, BAD_CAST "name");
                iprofile *inp = (iprofile*)calloc(1, sizeof(iprofile));
                lpush(listprofiles, inp);
                inp->base.name = strcrec((const char*)name);
                inp->base.type = (strcasecmp((const char*)xmlGetProp(profile, BAD_CAST "type"), "pad")==0) ? INP_JOYPAD : INP_KEYBOARD;
                //log4me_print("Input profile : %s\n", (const char*)name);
                XML_ENUM_CHILD(profile, node,
                    XML_ELEMENT_CONTENT("up", node,
                        SET_PROFILE_PROP(inp, up, (const char*)content)
                    )
                    XML_ELEMENT_CONTENT("down", node,
                        SET_PROFILE_PROP(inp, down, (const char*)content)
                    )
                    XML_ELEMENT_CONTENT("left", node,
                        SET_PROFILE_PROP(inp, left, (const char*)content)
                    )
                    XML_ELEMENT_CONTENT("right", node,
                        SET_PROFILE_PROP(inp, right, (const char*)content)
                    )
                    XML_ELEMENT_CONTENT("btn_a", node,
                        SET_PROFILE_PROP(inp, btn_a, (const char*)content)
                    )
                    XML_ELEMENT_CONTENT("btn_b", node,
                        SET_PROFILE_PROP(inp, btn_b, (const char*)content)
                    )
                    XML_ELEMENT_CONTENT("btn_start", node,
                        SET_PROFILE_PROP(inp, btn_start, (const char*)content)
                    )
                )
            )
        )

        XML_ELEMENT_CONTENT("player1", node,
             player1profilename = strcrec((const char*)content);
        )

        XML_ELEMENT_CONTENT("player2", node,
             player2profilename = strcrec((const char*)content);
        )

        XML_ELEMENT_ENUM_CHILD("video", node, video,
            XML_ELEMENT_CONTENT("fullscreen", video,
                config->fullscreen = atoi((const char*)content);
            )
            XML_ELEMENT_CONTENT("noasppectratio", video,
                config->noaspectratio = atoi((const char*)content);
            )
            XML_ELEMENT_CONTENT("scale", video,
                config->scale = atof((const char*)content);
            )
            XML_ELEMENT_CONTENT("overlay", video,
                config->overlayfilename = strcrec((const char*)content);
            )
            XML_ELEMENT_CONTENT("bezel", video,
                config->bezelfilename = strcrec((const char*)content);
            )
        )

        XML_ELEMENT_ENUM_CHILD("audio", node, audio,
            XML_ELEMENT_CONTENT("volume", audio,
                config->volume = atoi((const char*)content);
                if(config->volume>100) config->volume = 100;
            )
            XML_ELEMENT_CONTENT("nosound", audio,
                config->nosound = atoi((const char*)content);
            )
        )
    )

    xmlFreeDoc(doc);

    for(i=lcount(listprofiles)-1; i>=0; i--) {
        profile = lget(listprofiles, i);

        if(player1profilename && (strcasecmp(CSTR(profile->base.name), CSTR(player1profilename))==0)) {
            *player1 = profile;
            lremove(listprofiles, profile);
            continue;
        }

        if(player2profilename && (strcasecmp(CSTR(profile->base.name), CSTR(player2profilename))==0)) {
            *player2 = profile;
            lremove(listprofiles, profile);
            continue;
        }
    }

    strfree(player1profilename);
    strfree(player2profilename);

    while((profile = (iprofile*)lpop(listprofiles))!=NULL)
        input_freeprofile(profile);

    ldelete(listprofiles);
}

void freeconfig(emuconfig *config)
{
    strfree(config->bezelfilename);
    strfree(config->overlayfilename);
}
