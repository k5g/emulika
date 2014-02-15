#ifndef XML_H_INCLUDED
#define XML_H_INCLUDED

#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "types.h"

#define xmlStrCaseCmp(a,b)  (strcasecmp((const char*)(a), (const char*)(b))==0 ? 1 : 0)

#define XML_ELEMENT(element,node,subroutine) { \
            if((node->type==XML_ELEMENT_NODE) && xmlStrCaseCmp(element, node->name)) { \
                subroutine; \
            } \
        }


#define XML_ELEMENT_CONTENT(element,node,subroutine) { \
            if((node->type==XML_ELEMENT_NODE) && xmlStrCaseCmp(element, node->name) && node->children && (node->children->type==XML_TEXT_NODE)) { \
                xmlChar *content = xmlNodeGetContent(node->children); \
                subroutine; \
            } \
        }


#define XML_ENUM_CHILD(node,subnode,subroutine) { \
            xmlNode *subnode; \
            for(subnode=node->children; subnode; subnode=subnode->next) { \
                subroutine; \
            } \
        }


#define XML_ELEMENT_ENUM_CHILD(element,node,child,subroutine) { \
            XML_ELEMENT(element, node, \
                XML_ENUM_CHILD(node,child, \
                    subroutine; \
                ) \
            ) \
        }

void xmlTextWriterWriteArray(xmlTextWriterPtr writer, char *name, byte *array, int len);
int xmlNodeReadArray(xmlNode *root, byte *buffer, int size);

#endif // XML_H_INCLUDED
