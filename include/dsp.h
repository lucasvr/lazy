#ifndef _dsp_h
#define _dsp_h 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>

#if defined(__Linux__)
#include <linux/soundcard.h>
#elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__qnx__) || defined(sun)
#include <machine/soundcard.h>
#elif defined(__NetBSD__) || defined(HAVE_SOUNDCARD_H)
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif

/* some globals and defines */
int 	AUDIO_FORMAT;
int		SAMPLING_RATE;
#define AMPLITUDE                 0
#define FREQUENCY                 1
#define MONO                      1
#define STEREO                    2

/* globals */
int		audio_fd;


/* prototypes */
void	open_dsp (void);
void	close_dsp (void);
char	*audio_format_str (void);
int		random_value (int, int);

#endif /* _dsp_h */
