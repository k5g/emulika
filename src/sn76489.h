#ifndef SN76489_H_INCLUDED
#define SN76489_H_INCLUDED

#include "emul.h"
#include "misc/xml.h"

#define NCHANNELS       4

#define SDL_SAMPLES     512
#define SND_FREQUENCY   22050
#define SDL_BUF_SIZE    (SND_FREQUENCY/2) //SDL_SAMPLES*45

typedef struct {
	SDL_AudioDeviceID dev;

    int volume[NCHANNELS];
    dword frequency[NCHANNELS];
    byte lregister; // latched register
    byte noise;
    int cpuclock;

    word lfsr;
    int whitenoise;
    int freqch3_from_ch2;

    dword step;
    int samplerate;

    dword compute[NCHANNELS];
    int polarity[NCHANNELS];

    short buffer[SDL_BUF_SIZE];
    int curpos;

    sound_onoff playsound;

    int *lkpspps;
    int scanlinespersecond;
    int scanlines;
} sn76489;

void sn76489_init(sn76489 *snd, int clock, int scanlinespersecond, sound_onoff playsound);
void sn76489_free(sn76489 *snd);

void sn76489_write(sn76489 *snd, byte data);
void sn76489_execute(sn76489 *snd);

void sn76489_pause(sn76489 *snd, int value);

void sn76489_takesnapshot(sn76489 *snd, xmlTextWriterPtr writer);
void sn76489_loadsnapshot(sn76489 *snd, xmlNode *sndnode);

#endif // SN76489_H_INCLUDED
