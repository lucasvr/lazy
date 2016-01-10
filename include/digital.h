/*
 * ---------------------------------------------------------------------
 *  Lazy - play a CD and print actual track informations into stdout
 * ---------------------------------------------------------------------
 *
 * File: digital.h
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
#ifndef _digital_h
#define _digital_h 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>		/* read() / write() */
#include <sys/stat.h>		/* read() / write() */
#include <sys/ioctl.h>		/* for ioctl operations */
#include "cdromtypes.h"

#define IFRAMESIZE 	(CD_FRAMESIZE_RAW / sizeof (int))

typedef struct {
	int		read_buffer_size;
	int		keylen;
	int		ofs;		/* offset */
	int		retries;	/* number of retries done on jitter correction */
	int		retrys;		/* max number of jitter corrections */
	int		overlap;
	int		blocks;
	int		bufsize;
	int		ibufsize;
	int		bufstep;
	int		min;
	int		max;
	int		*starts;	/* LBA */
	char	*types;
} cdrom_t;


/* prototypes */
void	calculate_cdrom_globals (int);
void	read_cdaudio (int, int, char *, int);
int		read_cdtrack (int, int, int);
int		cd_jc (int *, int *);
int		cd_jc1 (int *, int *);
void	specific_read_toc_entry (int, struct CDROM_TOC_ENTRY *);
void	specific_read_leadout (int, struct CDROM_TOC_ENTRY *);


/* globals */
int			*p1;
cdrom_t		cdrom_info;				/* contain information needed to read RAW data from the CDROM device */
struct CDROM_TOC_ENTRY	toc_entry;	/* contain the minutes, seconds and offset frame of each track */

#endif /* _digital_h */
