/*
 * ---------------------------------------------------------------------
 *  Lazy - play a CD and print actual track informations into stdout
 * ---------------------------------------------------------------------
 *
 * File: cdromtypes.h
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
#ifndef _cdromtypes_h
#define _cdromtypes_h 1

#ifdef __linux__
#include <linux/cdrom.h>	/* for some cd-rom TOC structures	*/
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__qnx__) || defined(sun)
#include <sys/cdio.h>
#endif


#if defined(__linux__) || defined(sun)
# define CDROM_TOC_HEADER cdrom_tochdr
# define CDROM_TOC_ENTRY  cdrom_tocentry
# define CDROM_MSF_STRUCT cdrom_msf
# define CDROM_READ_AUDIO cdrom_read_audio
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__qnx__)
# define CDROM_TOC_HEADER ioc_toc_header
# define CDROM_TOC_ENTRY  ioc_read_toc_single_entry
# define CDROM_MSF_STRUCT ioc_play_msf
# define CDROM_READ_AUDIO ioc_read_audio
#endif

#ifndef CDROM_LEADOUT
# define CDROM_LEADOUT    0xAA		/* The leadout track is always 0xAA, regardless of # of tracks on disc */
#endif
#ifndef CDROM_DATA_TRACK
# define CDROM_DATA_TRACK 0x04
#endif
#ifndef CD_SECS
# define CD_SECS          60        /* seconds per minute */
#endif
#ifndef CD_FRAMES
#define CD_FRAMES         75        /* frames per second */
#endif
#ifndef CD_FRAMESIZE_RAW
# define CD_FRAMESIZE_RAW 2352
#endif

#endif /* _cdromtypes_h */
