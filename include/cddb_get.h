/*
 * ---------------------------------------------------------------------
 *  Lazy - play a CD and print actual track informations into stdout
 * ---------------------------------------------------------------------
 *
 * File: cddb_get.h
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

#ifndef _cddb_get_h
#define _cddb_get_h 1

#include <sys/types.h>		/* read() / write() / readdir()		*/
#include <sys/stat.h>		/* also for read() / write()		*/
#include <fcntl.h>			/* for fd operations, too			*/
#include <unistd.h>			/* for close()						*/
#include <errno.h>			/* errno..							*/
#include <sys/ioctl.h>		/* for ioctl operations				*/
#include "cdromtypes.h"

#include <dirent.h>			/* for directory management			*/
#include <string.h>
#include <ctype.h>


/* function prototypes */
char	*get_from_user (int, int, char *, int, char *, char *);
char	*get_hd_name (char *, unsigned);
char	*get_inet_name (char *, char *, int, int);
char    *save_to_disk (char *, int, char *);
int		cddb_sum (int);
int		read_cdtoc (int);
unsigned cddb_disc_id (int);


/* global track structure (do you know a cd with more than 100 tracks?) :) */
struct 
{
	int	min;	/* track minutes */
	int	sec;	/* track seconds */
	int	frame;	/* frame offset (starting location of track) */
} cdtoc[100];

/* global structure which would contain the minutes, seconds and offset frame of each track */
struct	CDROM_TOC_ENTRY	toc_entry;

#endif /* _cddb_get_h */
