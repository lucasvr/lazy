/*
 * ---------------------------------------------------------------------
 *  Lazy - play a CD and print actual track informations into stdout
 * ---------------------------------------------------------------------
 *
 * File: playcd.h
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
#ifndef _playcd_h
#define _playcd_h 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include "cdromtypes.h"

#define	UNRECOGNIZED 	"unrecognized song"
#define LAZY_VERSION	"0.24e"
#define LAZY_DATE		"2006/Oct/13"
#define	CONFIG_FILE		"lazyrc"
#define ABORT_CONSTANT  9999999
#define BUFFER_SIZE     4096

typedef  unsigned char	unchar;
typedef struct 
{
	char	*name;
} MUSIC;

/* function prototypes */
int     set_global_vars (void);
void	free_globals (void);
void    play_cd (int, int, int);
void    close_tray (int);
void	show_intro (void);
void	suicide (int);
void    get_info (char *);
void	skip_track (int);
void	do_nop (void);
struct CDROM_MSF_STRUCT *calculate_offsets (int, int *, int *);

MUSIC	music[100];		/* set the max album musics								*/
char	*artist;		/* global variable which stores the artist name			*/
char	*album;			/* global variable which stores the album name			*/
char	*AUDIO_DEVICE;	/* the audio device (usually /dev/dsp)					*/
char	*CDDEV;			/* the CD device (usually /dev/cdrom)					*/
char	*ADDRESS;		/* CDDB server address									*/
char	*PORT;			/* CDDB server port										*/
char	*CFIG_PATH;		/* path given in the config file						*/
char	*REAL_PATH;		/* the real pathname to the CFIG_PATH					*/
int		VERBOSE;		/* the initial verbose option							*/
int		STOP_MUSIC;		/* stop the music when the program exit?				*/
int		CLOSE_TRAY;		/* close the cd tray before read the CD?				*/
int		SHOW_TIME;		/* show music time?										*/
int 	SHOW_REMAINING;	/* show elapsed time instead of remaining				*/
int	 	RANDOM;			/* don't play random by default.						*/
int 	PLAYLIST;		/* don't have a desirable playlist by default			*/
int		SHOWONLY;		/* don't wanna only show the tracks names by default	*/
int		GLOBAL_TIME;	/* needed to skip tracks when displaying time, too 		*/
int 	DIGITAL_EXTRACTION;	/* use digital extraction? */

#endif /* _playcd_h */
