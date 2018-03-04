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

/*
SN76489 implementation based on :
    - http://www.smspower.org/Development/SN76489
*/

#include <SDL.h>

#include "sn76489.h"
#include "misc/log4me.h"

#define SN7_CMD_MASK    0x90
#define SN7_VOLUME_CMD  0x90
#define SN7_FREQ_CMD    0x80

int _volume = 256;

short volume_table[16]={
   32767/4, 26028/4, 20675/4, 16422/4, 13045/4, 10362/4,  8231/4,  6568/4,
    5193/4,  4125/4,  3277/4,  2603/4,  2067/4,  1642/4,  1304/4,     0
};

void audio_setvolume(int volume)
{
    if(volume>100) volume = 100;
    if(volume<0) volume = 0;

    _volume = (volume * 256) / 100;
}

static void sn76489_SDLmixaudio(void *userdata, Uint8 *stream, int len)
{
    sn76489 *snd = userdata;
    Uint8 *buffer = (Uint8*)snd->buffer;
    int buflen = (snd->curpos << 1);

    if(len>buflen) {
        memset(stream, 0, len);
		memcpy(stream+(len-buflen), buffer, buflen);
		snd->curpos = 0;
        log4me_warning(LOG_EMU_SN76489, "not enough data (%d/%d)\n", buflen, len);
        return;
    }

    memcpy(stream, buffer, len);
    memmove(buffer, buffer+len, buflen-len);
    snd->curpos -= (len >> 1);
}

void sn76489_init(sn76489 *snd, int clock, int scanlinespersecond, soundchannel channels, sound_onoff playsound)
{
    SDL_AudioSpec fmt, fmthave;
    int i;

	snd->dev = 0;
	snd->channels = channels;

    for(i=0;i<NCHANNELS;i++) snd->volume[i] = 0xF; // volume off
    memset(snd->frequency, 0, sizeof(snd->frequency));

    snd->noise = 0;
    snd->lregister = 0;

    snd->lfsr = 0x1111;
    snd->whitenoise = 0;
    snd->freqch3_from_ch2 = 0;
    snd->distribution = 0xFF;

    snd->playsound = playsound;

    // => http://www.smspower.org/Development/SN76489 / How the SN76489 makes sound
    // The SN76489 is connected to a clock signal, which is commonly 3579545Hz for NTSC systems and 3546893Hz for PAL/SECAM systems
    // (these are based on the associated TV colour subcarrier frequencies, and are common master clock speeds for many systems).
    // It divides this clock by 16 to get its internal clock. The datasheets specify a maximum of 4MHz.
    snd->step = ((uint64_t)clock << 12) / SND_FREQUENCY; //  ((clock / 16) / SND_FREQUENCY) * 65536
    snd->cpuclock = clock;

    snd->samplerate = 0;

    memset(snd->compute, 0, sizeof(snd->compute));
    memset(snd->polarity, 0, sizeof(snd->polarity));

    memset(snd->buffer, 0, sizeof(snd->buffer));
    snd->curpos = 0;

    /* Set 16-bit mono audio at 22Khz */
	memset(&fmt, 0, sizeof(SDL_AudioSpec));
    fmt.freq = SND_FREQUENCY;
    fmt.format = AUDIO_S16;
    fmt.channels = channels;
    fmt.samples = SDL_SAMPLES;
    fmt.callback = sn76489_SDLmixaudio;
    fmt.userdata = snd;

    snd->scanlinespersecond = scanlinespersecond;
    snd->scanlines = 0;

    if(snd->playsound==SND_OFF) return;

	/*for (i = 0; i < SDL_GetNumAudioDrivers(); ++i) {
		log4me_info("Audio driver %d: %s\n", i, SDL_GetAudioDriver(i));
	}
	int count = SDL_GetNumAudioDevices(0);
	for (i = 0; i < count; ++i) {
		log4me_info("Audio device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
	}*/

	if((snd->dev=SDL_OpenAudioDevice(NULL, 0, &fmt, &fmthave, SDL_AUDIO_ALLOW_ANY_CHANGE))==0) {
        log4me_error(LOG_EMU_SN76489, "Unable to open audio: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

	if(fmthave.samples>fmt.samples)
        log4me_info(LOG_EMU_SN76489, "WARNING !! Possible latency detected.\n");

    snd->lkpspps = malloc(sizeof(int)*snd->scanlinespersecond);
    for(i=0;i<snd->scanlinespersecond;i++)
        snd->lkpspps[i] = (((uint64_t)SND_FREQUENCY*(i+1)) / snd->scanlinespersecond) - (((uint64_t)SND_FREQUENCY*i) / snd->scanlinespersecond);
}


void sn76489_free(sn76489 *snd)
{
    if(snd->playsound==SND_OFF) return;

    free(snd->lkpspps);
    SDL_PauseAudioDevice(snd->dev, 1);
    SDL_CloseAudioDevice(snd->dev);
}

void sn76489_write(sn76489 *snd, byte port, byte data)
{
    int channel;

    if(bit7_is_set(data)) snd->lregister = data & 0x70;

    channel = ((snd->lregister & 0x60) >> 5);

    if(bit4_is_set(snd->lregister)) {
        /* => SEGA MASTER SYSTEM TECHNICAL INFORMATION by Richard Talbot-Watkins
        To set the volume of a channel, the SN76489 is programmed thus:
            bit  7 6 5 4 3 2 1 0
                 1 r r 1 d d d d
        where rr = 00: set channel 0 volume, 01: set channel 1 volume, 10: set channel 2 volume, 11: set channel 3 volume
        and dddd is a 4-bit value representing the desired volume.  0000 is loudest and 1111 is quietest (off). */
        snd->volume[channel] = data & 0x0F;
        log4me_debug(LOG_EMU_SN76489, "set volume channel %d : 0x%02X\n", channel, data & 0x0F);
    } else {
        if(channel==3) {
            /* => SEGA MASTER SYSTEM TECHNICAL INFORMATION by Richard Talbot-Watkins
            The noise channel, channel 3, is programmed in the following way:
                bit  7 6 5 4 3 2 1 0
                     1 1 1 0 x f n n
            where nn determines the frequency of noise produced:
                nn =    00: high pitch noise (clock/16 = 7800Hz)
                        01: medium pitch noise (clock/32 = 3900Hz)
                        10: low pitch noise (clock/64 = 1950Hz)
                        11: frequency is taken from that of channel 2.
            and f is the 'feedback' bit:
                f = 0: produce periodic (synchronous) noise [this sounds like a tuned tone,
                       but reaching much lower pitches than the square wave generators and with a very different timbre]
                    1: produce white noise. */
            snd->noise = data & 0x7;

            switch(data & 0x3) {
                case 0:
                    snd->frequency[channel] = 0x10;
                    snd->freqch3_from_ch2 = 0;
                    break;
                case 0x1:
                    snd->frequency[channel] = 0x20;
                    snd->freqch3_from_ch2 = 0;
                    break;
                case 0x2:
                    snd->frequency[channel] = 0x40;
                    snd->freqch3_from_ch2 = 0;
                    break;
                case 0x3:
                    snd->frequency[channel] = snd->frequency[2];
                    snd->freqch3_from_ch2 = 1;
                    break;
            }

            snd->whitenoise = (data & 3) ? 1 : 0;
            log4me_debug(LOG_EMU_SN76489, "channel 3 produce %s noise, frequency=0x%02X (%d Hz)\n", snd->noise & 0x04 ? "white" : "periodic", snd->noise & 0x03, 3579545 / ( 2 * (snd->frequency[channel]==0 ? 1 : snd->frequency[channel]) * 16));
        } else {
            /* => SEGA MASTER SYSTEM TECHNICAL INFORMATION by Richard Talbot-Watkins
            Setting the frequency of a tone channel usually requires two writes and is done in the following way:
               bit  7 6 5 4 3 2 1 0        bit  7 6 5 4 3 2 1 0
                    1 r r 0 f f f f             0 x h h h h h h
            where rr = 00: set channel 0 frequency, rr = 01: set channel 1 frequency, rr = 02: set channel 2 frequency
            and hhhhhhffff is a 10-bit value representing the desired frequency, ffff being the least significant 4 bits
            and hhhhhh being the most significant 6 bits.  x is an unused bit. */
            if(bit7_is_set(data))
                snd->frequency[channel] = (snd->frequency[channel] & 0x3F0) | (data & 0x0F);
            else
                snd->frequency[channel] = ((data & 0x3F) << 4) | (snd->frequency[channel] & 0x0F);
            if((channel==2) && snd->freqch3_from_ch2) snd->frequency[3] = snd->frequency[channel];
            log4me_debug(LOG_EMU_SN76489, "set frequency channel %d : 0x%04X (%d Hz)\n", channel, snd->frequency[channel], snd->cpuclock / ( 2 * (snd->frequency[channel]==0 ? 1 : snd->frequency[channel]) * 16));
        }
    }
}

void sn76489_writestereo(sn76489 *snd, byte port, byte data)
{
    log4me_debug(LOG_EMU_SN76489, "set control the stereo output %02X\n", data);
    snd->distribution = data;
}

void sn76489_pause(sn76489 *snd, int value)
{
    if(snd->playsound==SND_OFF) return;

    SDL_PauseAudioDevice(snd->dev, value);
}

static byte parity(word val)
{
     // => Code from http://www.smspower.org/Development/SN76489 / How the SN76489 makes sound
     val^=val>>8;
     val^=val>>4;
     val^=val>>2;
     val^=val>>1;
     return val & 1;
}

static void sn76489_internalmixaudio(sn76489 *snd)
{
    int i, ch;
    short sound = 0;
    byte distribution = snd->distribution;

    assert((snd->channels==1) || (snd->channels==2));
    if((snd->curpos + snd->channels - 1) >= SDL_BUF_SIZE) {
        snd->curpos = 0;
        log4me_warning(LOG_EMU_SN76489, "audio buffer is full\n");
    }

    for(i=0;i<NCHANNELS;i++) {
        // Recalculate compute if frequency channel changed
        if(snd->frequency[i]!=0) snd->compute[i] %= (snd->frequency[i] << 16);
        else snd->compute[i] = 0;

        //
        snd->compute[i] += snd->step;
        if(snd->compute[i]>=(snd->frequency[i] << 16)) {
            snd->compute[i] -= (snd->frequency[i] << 16);
            if(i==3) {
                // => Code from http://www.smspower.org/Development/SN76489 / How the SN76489 makes sound
                snd->lfsr=(snd->lfsr>>1) | ((snd->whitenoise ? parity(snd->lfsr & 0x9):snd->lfsr & 1)<<15);
                snd->polarity[3] =(snd->lfsr & 1);
            } else
                snd->polarity[i] ^= 1;
        }
    }

    SDL_LockAudioDevice(snd->dev);
    for(ch=0; ch<snd->channels; ch++) {
        for(i=0,sound=0; i<NCHANNELS; i++,distribution>>=1) {
            if((distribution & 1)==0) continue;
            if(i==3) {
                sound += snd->polarity[i]==1 ? volume_table[snd->volume[i]] : 0;
            } else {
                // => http://www.smspower.org/Development/SN76489 / How the SN76489 makes sound
                // If the register value is zero or one then the output is a constant value of +1. This is often used for sample playback on the SN76489.
                if(snd->frequency[i]<=1) snd->polarity[i] = 1;

                sound += (snd->polarity[i] ? -volume_table[snd->volume[i]] : volume_table[snd->volume[i]]);
            }
        }
        snd->buffer[snd->curpos++] = ((int)sound * _volume) >> 8;
    }
    SDL_UnlockAudioDevice(snd->dev);

    snd->samplerate++;
}

void sn76489_execute(sn76489 *snd)
{
    int i;

    if(snd->playsound==SND_OFF) return;

    for(i=0;i<snd->lkpspps[snd->scanlines];i++)
        sn76489_internalmixaudio(snd);

    if(++snd->scanlines>=snd->scanlinespersecond)
        snd->scanlines = 0;
}

void sn76489_takesnapshot(sn76489 *snd, xmlTextWriterPtr writer)
{
    int i;

    xmlTextWriterStartElement(writer, BAD_CAST "sn76489");
        xmlTextWriterStartElement(writer, BAD_CAST "channels");
            for(i=0;i<NCHANNELS;i++) {
                xmlTextWriterStartElement(writer, BAD_CAST "channel");
                xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "id", "%d", i);
                    xmlTextWriterStartElement(writer, BAD_CAST "volume");
                    xmlTextWriterWriteFormatString(writer, "%02X", snd->volume[i]);
                    xmlTextWriterEndElement(writer); /* volume */

                    xmlTextWriterStartElement(writer, BAD_CAST "frequency");
                    xmlTextWriterWriteFormatString(writer, "%08X", snd->frequency[i]);
                    xmlTextWriterEndElement(writer); /* frequency */
                xmlTextWriterEndElement(writer); /* channel */
            }
        xmlTextWriterEndElement(writer); /* channels */

        xmlTextWriterStartElement(writer, BAD_CAST "lregister");
        xmlTextWriterWriteFormatString(writer, "%02X", snd->lregister);
        xmlTextWriterEndElement(writer); /* lregister */

        xmlTextWriterStartElement(writer, BAD_CAST "noise");
        xmlTextWriterWriteFormatString(writer, "%02X", snd->noise);
        xmlTextWriterEndElement(writer); /* noise */

        xmlTextWriterStartElement(writer, BAD_CAST "lfsr");
        xmlTextWriterWriteFormatString(writer, "%04X", snd->lfsr);
        xmlTextWriterEndElement(writer); /* lfsr */

        xmlTextWriterStartElement(writer, BAD_CAST "whitenoise");
        xmlTextWriterWriteFormatString(writer, "%02X", snd->whitenoise);
        xmlTextWriterEndElement(writer); /* whitenoise */

        xmlTextWriterStartElement(writer, BAD_CAST "freqch3_from_ch2");
        xmlTextWriterWriteFormatString(writer, "%02X", snd->freqch3_from_ch2);
        xmlTextWriterEndElement(writer); /* freqch3_from_ch2 */

        xmlTextWriterStartElement(writer, BAD_CAST "distribution");
        xmlTextWriterWriteFormatString(writer, "%02X", snd->distribution);
        xmlTextWriterEndElement(writer); /* distribution */

        if(snd->playsound==SND_ON)
            xmlTextWriterWriteArray(writer, "buffer", (byte*)snd->buffer, snd->curpos*2);

    xmlTextWriterEndElement(writer); /* sn76489 */
}

void sn76489_loadsnapshot(sn76489 *snd, xmlNode *sndnode)
{
    int nchan;

    XML_ENUM_CHILD(sndnode, node,
        XML_ELEMENT_ENUM_CHILD("channels", node, channel,
            XML_ELEMENT("channel", channel,
                if((nchan=atoi((const char*)xmlGetProp(channel, BAD_CAST "id")))>=NCHANNELS) continue;
                XML_ENUM_CHILD(channel, info,
                    XML_ELEMENT_CONTENT("volume", info,
                        snd->volume[nchan] = (byte)strtoul((const char*)content, NULL, 16);
                    )
                    XML_ELEMENT_CONTENT("frequency", info,
                        snd->frequency[nchan] = (dword)strtoul((const char*)content, NULL, 16);
                    )
                )
            )
        )

        XML_ELEMENT_CONTENT("lregister", node,
            snd->lregister = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("noise", node,
            snd->noise = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("lfsr", node,
            snd->lfsr = (word)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("whitenoise", node,
            snd->whitenoise = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("freqch3_from_ch2", node,
            snd->freqch3_from_ch2 = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT_CONTENT("distribution", node,
            snd->distribution = (byte)strtoul((const char*)content, NULL, 16);
        )

        XML_ELEMENT("buffer", node,
            snd->curpos = xmlNodeReadArray(node, (byte*)snd->buffer, 2*SDL_BUF_SIZE) / 2;
        )
    )
}
