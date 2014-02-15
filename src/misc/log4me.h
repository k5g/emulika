#ifndef LOG4ME_H_INCLUDED
#define LOG4ME_H_INCLUDED

#include <stdint.h>

typedef struct {
    int id;
    char *stext;
} logitem;

void log4me_init(uint64_t items);
void log4me_redirectoutput(const char *fileout);

void log4me_print(const char *s, ...);
void log4me_error(uint64_t mask, const char *module, const char *s, ...);

#define log4me_info(m,...)   _log4me_info(m,__VA_ARGS__)
void _log4me_info(uint64_t mask, const char *module, const char *s, ...);

#ifdef DEBUG

#define log4me_debug(m,...)     _log4me_debug(m,__VA_ARGS__)
#define log4me_warning(m,...)   _log4me_warning(m,__VA_ARGS__)

int log4me_enabled(uint64_t mask);
void _log4me_warning(uint64_t mask, const char *module, const char *s, ...);
void _log4me_debug(uint64_t mask, const char *module, const char *s, ...);

#else
#define log4me_debug(m,...)     (void)0
#define log4me_warning(m,...)   (void)0
#endif

#endif // LOG4ME_H_INCLUDED
