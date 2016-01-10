/*
 * File: dsp.c
 *
 * Author: Lucas Correia Villa Real <lucasvr@gobolinux.org>
 *
 * ---------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * ---------------------------------------------------------------------
 */
#include "playcd.h"
#include "utils.h"
#include "dsp.h"


char *audio_format_str(void)
{
    switch (AUDIO_FORMAT) {
    case AFMT_U8:
	return ("AFMT_U8");
	break;
    case AFMT_S8:
	return ("AFMT_S8");
	break;
    case AFMT_S16_LE:
	return ("AFMT_S16_LE");
	break;
    case AFMT_S16_BE:
	return ("AFMT_S16_BE");
	break;
    case AFMT_U16_LE:
	return ("AFMT_U16_LE");
	break;
    case AFMT_U16_BE:
	return ("AFMT_U16_BE");
	break;
    }
    return ("unknown (error)");
}


/*
 * open_dsp - open the dsp device
 */
void open_dsp(void)
{
    int format, value, speed, channels, mask, option;


    /* try to open the device as Write-Only */
    audio_fd = open(AUDIO_DEVICE, O_WRONLY);
    if (audio_fd < 0) {
	/* disable plugin support and show a warning */
	err_sys("open %s", AUDIO_DEVICE);
	return;
    }

    /* use the best supported audio formats */
    value = ioctl(audio_fd, SNDCTL_DSP_GETFMTS, &mask);
    if (value < 0)
	err_sys("SNDCTL_DSP_GETFMTS");

    if (mask & AFMT_U16_BE)
	AUDIO_FORMAT = AFMT_U16_BE;
    else if (mask & AFMT_U16_LE)
	AUDIO_FORMAT = AFMT_U16_LE;
    else if (mask & AFMT_S16_BE)
	AUDIO_FORMAT = AFMT_S16_BE;
    else if (mask & AFMT_S16_LE)
	AUDIO_FORMAT = AFMT_S16_LE;
    else if (mask & AFMT_S8)
	AUDIO_FORMAT = AFMT_S8;
    else if (mask & AFMT_U8)
	AUDIO_FORMAT = AFMT_U8;

    /* select the audio sample format */
    format = AUDIO_FORMAT;
    value = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format);
    if (value < 0) {
	/* disable plugin support and show a warning */
	err_sys("SNDCTL_DSP_SETFMT");
    }
    if (format != AUDIO_FORMAT) {
	/* use the unsigned 8-bit default format, used in mostly PC soundcards */
	printf
	    ("Sample format 0x%x not supported by soundcard, using default AFMT_U8\n",
	     AUDIO_FORMAT);
	AUDIO_FORMAT = AFMT_U8;
	format = AUDIO_FORMAT;
	value = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format);
	if (value < 0) {
	    /* disable plugin support and show a warning */
	    err_sys("SNDCTL_DSP_SPEED");
	}
    }

    /* set the number of channels (mono/stereo) */
    channels = STEREO;
    value = ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &channels);

    option = 16;
    ioctl(audio_fd, SNDCTL_DSP_SAMPLESIZE, &option);
    option = 1;
    ioctl(audio_fd, SNDCTL_DSP_STEREO, &option);

    /* set the sampling rate to 8Khz (8000), if going to play from .WAV files */
    SAMPLING_RATE = 44100;
    speed = SAMPLING_RATE;
    value = ioctl(audio_fd, SNDCTL_DSP_SPEED, &speed);
    if (value < 0) {
	/* disable plugin support and show a warning */
	err_sys("SNDCTL_DSP_SPEED");
    }

    /* dump soundcard information into stdout */
#ifdef DEBUG
    printf("Audio format:\t%s\nChannels:\t%s\nSampling Rate:\t%d\n\n",
	   audio_format_str(),
	   channels == MONO ? "MONO" : "STEREO", SAMPLING_RATE);
#endif
}


/*
 * close_dsp - just close the dsp device
 */
void close_dsp(void)
{
    int value;

    value = close(audio_fd);
    if (value < 0)
	printf("Could not close the audio_fd device\n");
}


int random_value(min, max)
int min, max;
{
    int value = min + (int) ((float) max * rand() / (RAND_MAX + 1.0));
    return (value);
}
