/*
 * ---------------------------------------------------------------------
 *  Lazy - play a CD and print actual track informations into stdout
 * ---------------------------------------------------------------------
 *
 * File: playcd.c
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

#include "wrappers.h"
#include "cddb_get.h"
#include "random.h"
#include "digital.h"
#include "dsp.h"
#include "playcd.h"


/* 
 * here begins the scrambled code 
 */
int main(argc, argv)
int argc;
char **argv;
{
    unsigned cd_id;
    extern int optind;		/* from getopt() */
    extern int opterr;		/* from getopt() */
    extern char *optarg;	/* from getopt() */
    DIR *directory;

    char *subdir, *temp, *home, *file_name;
    int i, j, total_tracks, cd_tracks, option, cdrom_fd;
    int track = 0, START_TRACK = 0;
    int song_list[100], playlist_size = -1, x, y, tmp;


    /* set some global variables */
    home = getenv("HOME");
	if ((set_global_vars()) < 0) {
		printf("Could not find/create %s/.%s\n"
	       "Please check if it exist, else copy it from the shipped src/lazyrc file\n",
	       home, CONFIG_FILE);
		exit(EXIT_FAILURE);
    }


    /* check if we're using extra args in the command line */
    while ((option = getopt(argc, argv, "vkcdt::f:rslo:h")) != -1) {
	switch (option) {
	case 'v':
	    /* verbose mode */
	    VERBOSE = 1;
	    break;
	case 'k':
	    /* keep the music playing when the program exit */
	    STOP_MUSIC = 0;
	    break;
	case 'c':
	    /* close the cd tray */
	    CLOSE_TRAY = 1;
	    break;
	case 'd':
	    /* will use digital extraction */
	    DIGITAL_EXTRACTION = 1;
	    open_dsp();
	    break;
	case 'f':
	    if (optarg) {
		if (CDDEV)
		    free(CDDEV);
		CDDEV = strdup(optarg);
	    }
	    break;
	case 't':
	    /* show the seconds played */
	    SHOW_TIME = 1;
	    if (optarg) {
		if (*optarg == 'r')
		    SHOW_REMAINING = 1;
		else if (*optarg == 'e')
		    SHOW_REMAINING = 0;
		else {
		    printf("Error: invalid display mode for time.\n\n");
		    free_globals();
		    exit(EXIT_FAILURE);
		}
	    }
	    break;
	case 'o':
	    /* wanna start playing from another track */
	    START_TRACK = atoi(optarg) - 1;
	    break;
	case 'r':
	    /* wanna listen in a random order */
	    RANDOM = 1;
	    break;
	case 's':
	    /* wanna show the tracks names only */
	    SHOWONLY = 1;
	    break;
	case 'l':
	    /* wanna mount a playlist */
	    PLAYLIST = 1;
	    x = 0;
	    y = optind;

	    /* while has valid arguments, build the playlist */
	    while ((argv[y]) && (!strstr(argv[y], "-"))) {
		song_list[x] = atoi(argv[y]);
		x++;
		y++;
	    }

	    if (!x) {
		printf("Error: you must supply the tracks.\n\n");
		free_globals();
		exit(EXIT_FAILURE);
	    }

	    playlist_size = x;
	    break;
	case 'h':
	    /* show valid syntaxes */
	    show_intro();
	    printf("syntax: %s [options]\n", argv[0]);
	    printf("Available options:\n"
		   "\t-h         : this help\n"
		   "\t-v         : verbose mode\n"
		   "\t-k         : keep the music playing when the program exits\n"
		   "\t-c         : close the tray before playing the disc\n"
		   "\t-d         : use digital extraction\n"
		   "\t-f <device>: overrides the CDROM device specified in ~/.lazyrc\n"
		   "\t-t[r|e]    : shows the track's elapsed/remaining time\n"
		   "\t             e - display elapsed time\n"
		   "\t             r - display remaining time\n"
		   "\t             if none is specified, behave as set in ~/.lazyrc\n"
		   "\t             (will not work with '-d' flag)\n"
		   "\t-r         : play in random mode\n"
		   "\t-s         : just show the track names, without playing them\n"
		   "\t-l <list>  : play only track numbers given by <list>\n"
		   "\t-o <track> : start playing from track <track>\n"
		   "example: $ %s -v -tr -l 4 5 7\n\n", argv[0]);

	    free_globals();
	    exit(EXIT_SUCCESS);
	    break;
	}			/* switch */
    }				/* while  */


    /* get the file descriptor for the CD device */
    cdrom_fd = open(CDDEV, O_RDONLY | O_NONBLOCK);
    if (cdrom_fd < 0) {
	fprintf(stderr, "open %s: %s\n", CDDEV, strerror(errno));
	free_globals();
	exit(EXIT_FAILURE);
    }

    if (DIGITAL_EXTRACTION)
	calculate_cdrom_globals(cdrom_fd);

    /* put some stuff into memory */
    temp = strdup(CFIG_PATH);
    subdir = Calloc(strlen(temp), strlen(temp) * sizeof(char));
    REAL_PATH =
	Calloc(strlen(temp) + strlen(home),
	       (strlen(temp) + strlen(home)) * sizeof(char));

    /* convert any tilde found in the CFIG_PATH to the $HOME string */
    if (temp[0] == '~') {
	for (i = 1, j = 0; i < (strlen(temp)); i++, j++)
	    subdir[j] = temp[i];
	subdir[j + 1] = '\0';
	sprintf(REAL_PATH, "%s%s", home, subdir);
    } else
	REAL_PATH = strdup(temp);
    free(temp);

    /* check if our cddb path really exist */
    directory = opendir(REAL_PATH);
    if (!directory) {
	if (VERBOSE) {
	    printf("\nDirectory %s doesn't exist, trying to create it... ",
		   REAL_PATH);
	    fflush(stdout);
	}

	/* try to create it */
	if ((mkdir(REAL_PATH, 0744)) < 0)
	    fprintf(stderr, "mkdir %s: %s\n", REAL_PATH, strerror(errno));
	else {
	    if (VERBOSE)
		printf("Directory successfully created!\n\n");
	}

    } else
	closedir(directory);

    /* listen for some signals */
    signal(SIGQUIT, suicide);	/* quit program */

    /* show intro */
    show_intro();

    /* close the CD tray */
    if (CLOSE_TRAY)
	close_tray(cdrom_fd);


    /* get the total tracks from the CD */
    total_tracks = read_cdtoc(cdrom_fd);
    if (total_tracks < 0) {
	printf("Error scanning the number of tracks in your CD.\n");
	exit(EXIT_FAILURE);
    }

    /* save the total tracks from the CD into another variable */
    cd_tracks = total_tracks;

    /* check if START_TRACK <= total_tracks */
    if (START_TRACK > total_tracks) {
	printf("Error: Track %02d doesn't exist in this album."
	       "The last track is %02d.\n\n", START_TRACK + 1,
	       total_tracks);
	free_globals();
	exit(EXIT_FAILURE);
    }

    /* get the cd_id */
    cd_id = cddb_disc_id(total_tracks);

    /* try to get the audio info from the hard disk.. */
    file_name = get_hd_name(REAL_PATH, cd_id);
    if (file_name) {
	/* open the 'file_name' and search for album information */
	get_info(file_name);
    } else {
	/* if could not, try to get it from the internet connection.. */
	file_name = get_inet_name(ADDRESS, PORT, cd_id, total_tracks);
	if (file_name) {

	    /* fine, now we have the hard disk access! so, let's use it's information */
	    file_name = get_hd_name(REAL_PATH, cd_id);
	    if (file_name)
		get_info(file_name);
	    else {
		for (track = 1; track <= total_tracks; track++)
		    music[track].name = strdup(UNRECOGNIZED);
	    }

	} else {
	    for (track = 1; track <= total_tracks; track++)
		music[track].name = strdup(UNRECOGNIZED);
	}
    }

    /* if just want to see the tracks names in the CD */
    if (SHOWONLY) {
	for (x = 1; x <= total_tracks; x++) {
	    printf("[%02d] - %s\n", x, music[x].name);
	    free(music[x].name);
	}
	printf("\n");
	free_globals();
	exit(EXIT_SUCCESS);
    }

    /* if have a playlist */
    if (PLAYLIST) {
	/* set the new total_tracks size */
	total_tracks = playlist_size;
    } else {
	/* just fill the playlist array with the normal track order */
	for (x = 0; x < total_tracks; x++)
	    song_list[x] = x + 1;
    }

    /* if we are going to randomize the play order */
    if (RANDOM) {
	/* to make the random list different than the last one! */
	randomize();

	for (x = 0; x < total_tracks; x++) {
	    /* get a track number from the Random() function */
	    y = Random(0, total_tracks);
	    tmp = song_list[x];
	    song_list[x] = song_list[y];
	    song_list[y] = tmp;
	}
    }

    /* check if we are not going to play any invalid track */
    for (x = 0; x < total_tracks; x++) {
	if ((song_list[x] <= 0) || (song_list[x] > cd_tracks)) {
	    printf("Error: invalid track %d\n", song_list[x]);
	    free_globals();
	    for (i = 0; i < total_tracks; i++)
		free(music[i].name);
	    exit(EXIT_FAILURE);
	}
    }


    /* print the CD ID information */
    printf("CD ID: %08x || Total Tracks: %d\n\n", cd_id, total_tracks);

    /* print the playlist order */
    printf("Playlist: ");
    for (x = 0; x < total_tracks; x++)
	printf("%d ", song_list[x]);
    printf("\n");


    /* play the track(s) */
    for (x = START_TRACK; x < total_tracks; x++) {
	track = song_list[x];

	/* print the pretty information */
	printf("Playing CDDA stream from [%02d] - %s ...\n", track,
	       music[track].name);

	/* play the song */
	if (DIGITAL_EXTRACTION)
	    read_cdtrack(track - 1, total_tracks, cdrom_fd);
	else
	    play_cd(track - 1, total_tracks, cdrom_fd);

	free(music[track].name);
    }

    puts("");
    Free(subdir);
    free_globals();
    exit(EXIT_SUCCESS);
}


/*
 * get_info - open the filename and put music title's into the global variable
 */
void get_info(file)
char *file;
{
    char line[BUFFER_SIZE], name[BUFFER_SIZE];
    char *token = NULL, *tmp;
    int i, index = 1;
    FILE *f;

    /* try to open the file */
    f = fopen(file, "r");
    if (!f) {
	fprintf(stderr, "fopen %s: %s\n", file, strerror(errno));
	exit(EXIT_FAILURE);
    }

    /* read it line by line */
    while (!feof(f)) {
	if ((fgets(line, sizeof(line), f))) {
	    if (!(strstr(line, "DTITLE="))) {
		/* check if is the music name.. */
		if ((strstr(line, "TTITLE"))) {
		    token = strtok(line, "=");
		    if (!token) {
			printf("error: TTITLE has no arguments\n");
			continue;
		    }

		    token = strtok(NULL, "=");
		    if (!token) {
			printf("error: TTITLE has no arguments\n");
			continue;
		    }

		    /* seek for the \r character */
		    for (i = 0; i < strlen(token); i++) {
			if ((token[i] == '\n') || (token[i] == '\r'))
			    break;
		    }
		    token[i] = '\0';

		    /* check if the last character is a space */
		    if (artist[strlen(artist)] == ' ')
			snprintf(name, sizeof(name), "%s- %s", artist,
				 token);
		    else
			snprintf(name, sizeof(name), "%s - %s", artist,
				 token);

		    music[index].name = strdup(name);
		    index++;
		}
		continue;
	    } else {
		/* print the album name */
		tmp = strtok(line, "=");
		if (!tmp) {
		    printf("error: no arguments given on %s\n", line);
		    continue;
		}
		tmp = strtok(NULL, "=");
		if (!tmp) {
		    printf("error: no arguments given on %s\n", line);
		    continue;
		}
		tmp = strtok(tmp, "/");
		if (!tmp) {
		    printf("error: no arguments given on %s\n", line);
		    continue;
		}

		artist = strdup(tmp);
		album = strdup(strtok(NULL, "/"));

		/* verify if we have not an empty space in the end of the string */
		if (artist[strlen(artist) - 1] == ' ')
		    artist[strlen(artist) - 1] = '\0';

		printf("Artist: %s   ", artist);
		printf("Album name: %s\n", album);
	    }
	}			/* if */
    }				/* while */
}


/*
 * play_cd - play the 'track_no' track into 'device'
 */
void play_cd(track_no, total_tracks, cdrom_fd)
int track_no, total_tracks, cdrom_fd;
{
    register int music_time = 0, seconds, retval;
    int minutes, length_min, length_sec;
    struct CDROM_MSF_STRUCT *track;
    char *error = NULL;


    /* 
     * listen for the SIGINT signal for skipping tracks and for the 
     * SIGQUIT signal to leave the program 
     */
    if (track_no == (total_tracks - 1))
	signal(SIGINT, suicide);
    else
	signal(SIGINT, skip_track);
    signal(SIGQUIT, suicide);

    /* calculate the offsets */
    track = calculate_offsets(track_no, &length_min, &length_sec);

    /* set the total music time (in seconds) */
    music_time = (length_min * 60) + length_sec;

    /* try to play the 'track_no' track */
#if defined(__linux__) || defined(sun)
    retval = ioctl(cdrom_fd, CDROMPLAYMSF, track);
    error = "CDROMPLAYMSF";
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__qnx__)
    retval = ioctl(cdrom_fd, CDIOCPLAYMSF, track);
    error = "CDIOCPLAYMSF";
#endif
    if (retval < 0) {
	perror(error);
	exit(EXIT_FAILURE);
    }

    /* if wanna show the seconds played */
    if (SHOW_TIME) {
	if (SHOW_REMAINING) {
	    minutes = length_min;
	    seconds = length_sec;
	    for (GLOBAL_TIME = 0; GLOBAL_TIME < music_time; GLOBAL_TIME++) {
		printf("\tRemaining Time: %02d:%02d\r", minutes,
		       seconds--);
		fflush(stdout);
		if (seconds < 0) {
		    --minutes;
		    seconds = 59;
		}

		/* I'm not sleeping 1sec because of the printf + var calculus delay */
		usleep(991000);
	    }
	} else {
	    minutes = 0;
	    seconds = 0;
	    for (GLOBAL_TIME = 0; GLOBAL_TIME < music_time; GLOBAL_TIME++) {
		printf("\tElapsed Time: %02d:%02d\r", minutes, seconds);
		seconds++;
		fflush(stdout);
		if (seconds == 60) {
		    ++minutes;
		    seconds = 0;
		}

		/* I'm not sleeping 1sec because of the printf + var calculus delay */
		usleep(991000);
	    }
	}
    } else {
	/* just sleep the music time */
	sleep(music_time);
    }

    Free(track);
}


struct CDROM_MSF_STRUCT *calculate_offsets(track_no, length_min,
					   length_sec)
int track_no, *length_min, *length_sec;
{
    struct CDROM_MSF_STRUCT *track;
    int length_frame = 0;
    int offset_min = 0, offset_sec = 0, offset_frame = 0;
    int end_min = 0, end_sec = 0, end_frame = 0;
    int tmp1 = 0, tmp2 = 0;


    track =
	(struct CDROM_MSF_STRUCT *)
	malloc(sizeof(struct CDROM_MSF_STRUCT));
    *length_min = *length_sec = 0;

    /* set start min/sec/frame within the informations on the toc_entry struct (from cddb_get.c) */
    offset_min = cdtoc[track_no].min;
    offset_sec = cdtoc[track_no].sec;
    offset_frame = cdtoc[track_no].frame;

    /*  --- calculate the end frame for 'track_no' track --- */
    /* get the total 'track_no' time */
    tmp1 = offset_frame;	/* tmp1 == start frame offset */
    tmp1 += offset_sec * CD_FRAMES;	/* there are CD_FRAMES in 1sec - defined in cdrom.h */
    tmp1 += offset_min * CD_SECS * CD_FRAMES;	/* CD_SECS is also a constant in cdrom.h */

    /* calculate the total time from the next track */
    tmp2 = cdtoc[track_no + 1].frame;
    tmp2 += cdtoc[track_no + 1].sec * CD_FRAMES;
    tmp2 += cdtoc[track_no + 1].min * CD_SECS * CD_FRAMES;

    tmp2 -= tmp1;		/* get the total track length in frames                 */
    *length_min = tmp2 / (CD_SECS * CD_FRAMES);	/* calculate the number of minutes of the track */
    tmp2 %= CD_SECS * CD_FRAMES;	/* get the left-over frames                                             */
    *length_sec = tmp2 / CD_FRAMES;	/* get the number of seconds of the track               */
    length_frame = tmp2 % CD_SECS;	/* get the left-over frames again                               */

    /* get the end_frame offset */
    end_frame += offset_frame + length_frame;
    if (end_frame >= CD_FRAMES) {	/* check if end_frame is in it's valid limits */
	end_sec += end_frame / CD_FRAMES;
	end_frame %= CD_SECS;
    }

    /* get the end_sec offset */
    end_sec += offset_sec + *length_sec;
    if (end_sec >= CD_SECS) {	/* check if end_sec is in it's valid limits */
	end_min += end_sec / CD_SECS;
	end_sec %= CD_SECS;
    }

    /* get the end_min offset */
    end_min += offset_min + *length_min;
    /* --- end of the track frame calculus --- */


    /*
     * set toc_msf info with 'track_no+1' offsets, obtained from the 
     * toc_entry struct + previous calculus
     */
#if defined(__linux__) || defined(sun)
    track->cdmsf_min0 = offset_min;	/* start minute                         */
    track->cdmsf_sec0 = offset_sec;	/* start second                         */
    track->cdmsf_frame0 = offset_frame;	/* start frame offset           */
    track->cdmsf_min1 = end_min;	/* minute offset to end at      */
    track->cdmsf_sec1 = end_sec;	/* second offset to end at      */
    track->cdmsf_frame1 = end_frame;	/* frame  offset to end at      */
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__qnx__)
    track->start_m = offset_min;
    track->start_s = offset_sec;
    track->start_f = offset_frame;
    track->end_m = end_min;
    track->end_s = end_sec;
    track->end_f = end_frame;
#endif

    /* print some information for the 'track_no' track */
    printf("\tTrack time: %d:%02d\n\n", *length_min, *length_sec);

    /* if DEBUG mode is set, print the offsets and extra information, as you can see below */
#ifdef DEBUG
    printf("\tOffsets are at %d minutes, %d seconds and %d frames\n"
	   "\tLength is %d minutes, %d seconds and %d frames\n"
	   "\tEnd is at %d minutes, %d seconds and %d frames\n\n",
	   offset_min, offset_sec, offset_frame,
	   *length_min, *length_sec, length_frame,
	   end_min, end_sec, end_frame);
#endif
    return track;
}


/*
 * close_tray - close the CD tray
 */
void close_tray(cdrom_fd)
int cdrom_fd;
{
    int retval = -1;
#if defined(__linux__) || defined(sun)
    retval = ioctl(cdrom_fd, CDROMCLOSETRAY);
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__qnx__)
    retval = ioctl(cdrom_fd, CDIOCCLOSE);
#endif
    if (retval < 0)
	printf("Could not close the CD tray: %s\n", strerror(errno));
}


/*
 * show_intro - print some information about this software
 */
void show_intro(void)
{
    printf("Lazy - a CD player with track/artist/song information\n"
	   "Version %s (%s) with support for CDDB/FreeDB\n"
	   "Written by Lucas Correia Villa Real <lucasvr@gobolinux.org>\n"
	   "This software is licensed under the GPL\n\n",
	   LAZY_VERSION, LAZY_DATE);
}


/*
 * set_global_vars - set some global variables with informations taken from CONFIG_FILE, 
 * 					 returning 0 on success or -1 on error.
 */
int set_global_vars(void)
{
    FILE *config;
    char temp[strlen(CONFIG_FILE) + strlen(getenv("HOME")) + 3];
    char *token, line[BUFFER_SIZE], bkp_line[BUFFER_SIZE];


    /* set the pointers to the heart of the sun */
    artist = album = ADDRESS = PORT = CFIG_PATH = REAL_PATH = NULL;
#if defined(__linux__) || defined(sun)
    CDDEV = strdup("/dev/cdrom");
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__qnx__)
    CDDEV = strdup("/dev/acd0");
#endif
    AUDIO_DEVICE = strdup("/dev/dsp");
    VERBOSE = 0;
    STOP_MUSIC = 1;
    CLOSE_TRAY = 0;
    SHOW_TIME = 0;
    SHOW_REMAINING = 0;
    RANDOM = 0;
    PLAYLIST = 0;
    SHOWONLY = 0;
    DIGITAL_EXTRACTION = 0;

    /* try to open the config file */
    snprintf(temp, sizeof(temp), "%s/.%s", getenv("HOME"), CONFIG_FILE);
    config = fopen(temp, "r");
    if (!config) {
	/* try to fetch information from /etc/lazyrc */
	snprintf(temp, sizeof(temp), "/etc/%s", CONFIG_FILE);
	config = fopen(temp, "r");
	if (!config) {
	    printf("%s: %s", temp, strerror(errno));
	    return -1;
	}
    }

    /* get the information from it */
    while (!(feof(config))) {
	fgets(line, sizeof(line), config);
	strcpy(bkp_line, line);
	if ((!(strncmp(line, "#", 1))) || (!(strncmp(line, "\n", 1))))
	    continue;
	else if ((strstr(line, "CDDEV"))) {
	    token = strtok(line, "=");
	    if (!token) {
		printf("syntax error on config file: %s\n", bkp_line);
		fclose(config);
		exit(EXIT_FAILURE);
	    }
	    token = strtok(NULL, " \t");
	    if (token)
		CDDEV = strdup(token);
	    else
		printf
		    ("CDDEV has no value in the config. Please check it!\n");

	} else if ((strstr(line, "AUDIODEV"))) {
	    token = strtok(line, "=");
	    if (!token) {
		printf("syntax error on config file: %s\n", bkp_line);
		fclose(config);
		exit(EXIT_FAILURE);
	    }
	    token = strtok(NULL, " \t");
	    if (token)
		AUDIO_DEVICE = strdup(token);
	    else
		printf
		    ("AUDIODEV has no value in the config. Please check it!\n");

	} else if ((strstr(line, "ADDRESS"))) {
	    token = strtok(line, "=");
	    if (!token) {
		printf("syntax error on config file: %s\n", bkp_line);
		fclose(config);
		exit(EXIT_FAILURE);
	    }
	    token = strtok(NULL, " \t");
	    if (token)
		ADDRESS = strdup(token);
	    else
		printf
		    ("ADDRESS has no value in the config. Please check it!\n");

	} else if ((strstr(line, "REMAINING"))) {
	    SHOW_REMAINING = 1;
	} else if ((strstr(line, "ELAPSED"))) {
	    SHOW_REMAINING = 0;
	} else if ((strstr(line, "PORT"))) {
	    token = strtok(line, "=");
	    if (!token) {
		printf("syntax error on config file: %s\n", bkp_line);
		fclose(config);
		exit(EXIT_FAILURE);
	    }
	    token = strtok(NULL, " \t");
	    if (token)
		PORT = strdup(token);
	    else
		printf
		    ("PORT has no value in the config. Please check it!\n");

	} else if ((strstr(line, "CDDB_PATH"))) {
	    token = strtok(line, "=");
	    if (!token) {
		printf("syntax error on config file: %s\n", bkp_line);
		fclose(config);
		exit(EXIT_FAILURE);
	    }
	    token = strtok(NULL, " \t");
	    if (token)
		CFIG_PATH = strdup(token);
	    else
		printf
		    ("CDDB_PATH has no value in the config. Please check it!\n");
	}
    }				/* while */
    return (0);
}


/*
 * free_globals - free the globals alloc'ed in set_global_vars()
 */
void free_globals(void)
{
    if (CDDEV)
	free(CDDEV);
    if (AUDIO_DEVICE)
	free(AUDIO_DEVICE);
    if (ADDRESS)
	free(ADDRESS);
    if (PORT)
	free(PORT);
    if (CFIG_PATH)
	free(CFIG_PATH);
    if (REAL_PATH)
	free(REAL_PATH);
    if (artist)
	free(artist);
    if (album)
	free(album);
}


/*
 * skip_track - when received the SIGINT signal, skip a track. This function doesn't need 
 * 				to have anything on it's body, but a simple information is not a bad idea.
 */
void skip_track(signal)
int signal;
{
    /* call the do_nop function. this function will let us to listen for another signals again */
    printf("Skipping track...             \n\n");
    do_nop();

    /* 
     * change the GLOBAL_TIME to a value bigger than one possible to be created by play_cd()
     * this will cause the function to abort, skipping the track instead of still showing it's
     * remaining time
     */
    GLOBAL_TIME = ABORT_CONSTANT;
}


/*
 * do_nop - this function returns and receive nothing
 */
void do_nop(void)
{
    signal(SIGINT, suicide);
    usleep(300000);
}


/*
 * suicide - if signal == SIGINT, quit the program
 */
void suicide(int signal)
{
    int cdrom, retval = -1;
    char *error = NULL;
    char *device = strdup(CDDEV);

    /* open the device */
    if ((cdrom = open(device, O_RDONLY | O_NONBLOCK)) < 0) {
	fprintf(stderr, "open %s: %s\n", device, strerror(errno));
	exit(EXIT_FAILURE);
    }

    if (STOP_MUSIC) {
	// stop the music
#if defined(__linux__) || defined(sun)
	retval = ioctl(cdrom, CDROMSTOP);
	error = "CDROMSTOP";
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__qnx__)
	retval = ioctl(cdrom, CDIOCSTOP);
	error = "CDIOCSTOP";
#endif
	if (retval < 0) {
	    perror(error);
	    close(cdrom);
	    exit(EXIT_FAILURE);
	}
    }

    /* close the file descriptor */
    if (close(cdrom) < 0) {
	fprintf(stderr, "close: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }

    printf("Oh my God! They killed Kenny!\n");
    free_globals();
    exit(EXIT_SUCCESS);
}
