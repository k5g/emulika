#ifndef CLOCK_H_INCLUDED
#define CLOCK_H_INCLUDED

struct _sdlclock;
typedef struct _sdlclock * sdlclock;

void clock_init(void);

sdlclock clock_new(int frequency);
void clock_release(sdlclock c);
void clock_step(sdlclock c);
int clock_elapsed(sdlclock c);
void clock_wait(sdlclock c);
void clock_reset();
void clock_pause(int enable);

#endif // CLOCK_H_INCLUDED
