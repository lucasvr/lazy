/*
 * ---------------------------------------------------------------------
 *  Lazy - play a CD and print actual track informations into stdout
 * ---------------------------------------------------------------------
 *
 * File: cddb_get.c
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
#include "playcd.h"
#include "socket.h"
#include "utils.h"
#include "cddb_get.h"


/*
 * read_cdtoc - search for the number of tracks in the CD.
 * return the number of tracks and put the correct information into cdtoc[], or -1 on error.
 */
int read_cdtoc(device)
int device;
{
    int first_track = 0, last_track = 0;
    int i, temp;
    struct CDROM_TOC_HEADER toc_hdr;

    /* get the total tracks from the CD */
#if defined(__linux__) || defined(sun)
    temp = ioctl(device, CDROMREADTOCHDR, &toc_hdr);
    if (temp < 0) {
	perror("CDROMREADTOCHDR");
	exit(EXIT_FAILURE);
    }

    first_track = toc_hdr.cdth_trk0;
    last_track = toc_hdr.cdth_trk1;

    /* set the CD format and the lead_out TOC */
    toc_entry.cdte_format = CDROM_MSF;
    toc_entry.cdte_track = CDROM_LEADOUT;
    if (ioctl(device, CDROMREADTOCENTRY, &toc_entry) < 0) {
	perror("CDROMREADTOCENTRY");
	exit(EXIT_FAILURE);
    }
    cdtoc[last_track].min = toc_entry.cdte_addr.msf.minute;
    cdtoc[last_track].sec = toc_entry.cdte_addr.msf.second;
    cdtoc[last_track].frame = toc_entry.cdte_addr.msf.frame;

    /* start putting the CD info into the cdtoc[] array */
    for (i = last_track; i >= first_track; i--) {
	/* set the cdte_track and cdte_format info */
	toc_entry.cdte_track = i;
	toc_entry.cdte_format = CDROM_MSF;

	if (ioctl(device, CDROMREADTOCENTRY, &toc_entry) < 0) {
	    perror("CDROMREADTOCENTRY");
	    exit(EXIT_FAILURE);
	}

	cdtoc[i - 1].min = toc_entry.cdte_addr.msf.minute;
	cdtoc[i - 1].sec = toc_entry.cdte_addr.msf.second;
	cdtoc[i - 1].frame = toc_entry.cdte_addr.msf.frame;
    }
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__qnx__)
    temp = ioctl(device, CDIOREADTOCHEADER, &toc_hdr);
    if (temp < 0) {
	perror("CDIOREADTOCHEADER");
	exit(EXIT_FAILURE);
    }

    first_track = toc_hdr.starting_track;
    last_track = toc_hdr.ending_track;

    /* set the CD format and the lead_out TOC */
    toc_entry.track = CDROM_LEADOUT;
    toc_entry.address_format = CD_MSF_FORMAT;
    if (ioctl(device, CDIOREADTOCENTRY, &toc_entry) < 0) {
	perror("CDIOREADTOCENTRY");
	exit(EXIT_FAILURE);
    }
    cdtoc[last_track].min = toc_entry.entry.addr.msf.minute;
    cdtoc[last_track].sec = toc_entry.entry.addr.msf.second;
    cdtoc[last_track].frame = toc_entry.entry.addr.msf.frame;

    /* start putting the CD info into the cdtoc[] array */
    for (i = last_track; i >= first_track; i--) {
	/* set the cdte_track and cdte_format info */
	toc_entry.track = i;
	toc_entry.address_format = CD_MSF_FORMAT;

	if (ioctl(device, CDIOREADTOCENTRY, &toc_entry) < 0) {
	    perror("CDIOREADTOCENTRY");
	    exit(EXIT_FAILURE);
	}

	cdtoc[i - 1].min = toc_entry.entry.addr.msf.minute;
	cdtoc[i - 1].sec = toc_entry.entry.addr.msf.second;
	cdtoc[i - 1].frame = toc_entry.entry.addr.msf.frame;
    }
#endif
    return (last_track);
}


/*
 * cddb_disc_id - generate the disc ID from the total music time
 * (routine token from the cddb.howto)
 */
unsigned int cddb_disc_id(total_tracks)
int total_tracks;
{
    int i, t = 0, n = 0;


    /* n == total music time in seconds */
    i = 0;
    while (i < total_tracks) {
	n = n + cddb_sum((cdtoc[i].min * 60) + cdtoc[i].sec);
	i++;
    }

    /* t == lead-out offset seconds - 1st music total time, in seconds */
    t = ((cdtoc[total_tracks].min * 60) + cdtoc[total_tracks].sec)
	- ((cdtoc[0].min * 60) + cdtoc[0].sec);

    /*
     *  mod 'n' with FFh and left-shift it by 24.
     * 't' is left-shifted by 8.
     * the disc_id is then returned as these operations + total tracks 'OR'ed together.
     */
    return ((n % 0xff) << 24 | t << 8 | total_tracks);
}


/*
 * cddb_sum - adds the value of each digit in the decimal string representation of the number.
 * (routine token from the cddb.howto)
 */
int cddb_sum(n)
int n;
{
    int ret = 0;

    while (n > 0) {
	ret = ret + (n % 10);
	n = n / 10;
    }

    return (ret);
}


/*
 * get_hd_name - search for the CD info in the hard disk CDDB, returning
 * NULL on error or the filename on success.
 */
char *get_hd_name(path, cd_id)
char *path;
unsigned cd_id;
{
    int i, number, fd;
    char *name;
    char cdrom_id[9];
    struct dirent **directory;

    printf("Searching for CDDB entries on %s ... ", path);
    fflush(stdout);

    /* try to open the given directory */
    if (!(opendir(path))) {
	printf("directory not found, check your config file!\n");
	return (NULL);
    }

    /* get the number of subdirectories in the 'path' dir */
    number = scandir(path, &directory, 0, alphasort);
    if (number < 0) {
	fprintf(stderr, "scandir %s: %s\n", path, strerror(errno));
	return (NULL);
    }

    /* set the cdrom_id */
    sprintf(cdrom_id, "%08x", cd_id);
    cdrom_id[9] = '\0';

    for (i = 0; i < number; i++) {
	/* ignore '.' and '..' directories */
	if ((strcmp(directory[i]->d_name, "."))
	    && (strcmp(directory[i]->d_name, ".."))) {
	    name =
		Malloc((strlen(path) + strlen(directory[i]->d_name) +
			15) * sizeof(char));
	    sprintf(name, "%s", path);
	    strcat(name, "/");
	    strncat(name, directory[i]->d_name,
		    strlen(directory[i]->d_name));
	    strcat(name, "/");
	    strncat(name, cdrom_id, 8);
	    if ((fd = open(name, O_RDONLY)) >= 0) {
		printf("OK\n");
		close(fd);
		return (name);
	    }
	    Free(name);
	}
    }

    printf("not found\n");
    return (NULL);
}


/*
 * get_inet_name - search for the song in the CDDB given address/port, returning
 * it's name, or NULL if not found.
 */
char *get_inet_name(address, char_port, discID, tracks)
char *address, *char_port;
int discID, tracks;
{
    int port = atoi(char_port);
    int server_fd, i, j, n, backup, key;
    int total_secs = 0, counter = 0;
    char *answer = NULL, *username, *filename, categ[20], newID[9];
    char msg[BUFFER_SIZE], offsets[BUFFER_SIZE], tmpbuf[BUFFER_SIZE];
    char hostname[MAXHOSTNAMELEN], server[80];


    /* try to create a socket to the server */
    printf("Opening Connection to %s:%d ... ", address, port);
    fflush(stdout);

    /* get the server fd from the create_socket function */
    server_fd = create_socket((unchar *) address, port);
    if (server_fd < 0)
	return (NULL);
    else
	printf("OK\n");


    /* get the initial message from the server */
    n = read(server_fd, server, BUFFER_SIZE);
    server[n - 2] = '\0';

    if (VERBOSE) {
	printf("\n<- %s\n", server);
	printf("Saying HELLO to CDDB server ...\n");
    }

    /* set some settings before saying HELLO to the CDDB server */
    username = get_username();
    n = gethostname(hostname, sizeof(hostname));
    if (n < 0 || !strlen(hostname)) {
	if (VERBOSE)
	    fprintf(stderr, "Could not get hostname: %s\n",
		    strerror(errno));
	snprintf(hostname, sizeof(hostname), "unknown");
    }

    snprintf(msg, sizeof(msg), "cddb hello %s %s Lazy %s\r\n", username,
	     hostname, LAZY_VERSION);
    answer = send_to_server(server_fd, msg);
    if (!answer) {
	printf("bad response from the server");
	close(server_fd);
	return (NULL);
    }

    /* set another settings before querying the CDDB database */
    tmpbuf[0] = '\0';
    for (i = 0; i < tracks; i++) {
	/* put the frame offset of the starting location of each track in a string */
	snprintf(offsets, sizeof(offsets), "%s %d ", tmpbuf,
		 cdtoc[i].frame +
		 (75 * (cdtoc[i].sec + (60 * cdtoc[i].min))));
	strcpy(tmpbuf, offsets);
	counter +=
	    cdtoc[i].frame + (75 * cdtoc[i].sec + (60 * cdtoc[i].min));
    }

    total_secs = cdtoc[tracks].sec + (cdtoc[tracks].min * 60);

    /* send it */
    snprintf(msg, sizeof(msg), "cddb query %08x %d %s %d\r\n", discID,
	     tracks, offsets, total_secs);
    answer = send_to_server(server_fd, msg);
    if (!answer) {
	printf("bad response from the server\n");
	close(server_fd);
	return (NULL);
    }

    /*
     * if answer == "200...", found exact match
     * if answer == "211...", found too many matches
     * if answer == "202...", found no matches
     */
    i = 0;
    if (!(strncmp(answer, "211", 3))) {
	/* seek the 2nd line */
	while (answer[i] != '\n')
	    ++i;

	/* copy the 1st match to the category */
	j = 0;
	i++;
	while (answer[i] != ' ')
	    categ[j++] = answer[i++];
	categ[j++] = '\0';

	/* get the new cdID given from the CDDB */
	j = 0;
	i++;
	while (answer[i] != ' ')
	    newID[j++] = answer[i++];
	newID[j++] = '\0';

    } else if (!(strncmp(answer, "200", 3))) {
	/* get it from the 1st line */
	while (answer[i] != ' ')
	    i++;
	i++;

	/* copy the match to the category */
	j = 0;
	while (answer[i] != ' ')
	    categ[j++] = answer[i++];
	categ[j++] = '\0';

	/* copy the new cdID */
	j = 0;
	i++;
	while (answer[i] != ' ')
	    newID[j++] = answer[i++];
	newID[j++] = '\0';
    } else {
	printf("Could not find any matches for %08x\n\n", discID);
	return (NULL);

	/* TODO: ask for the user write data to the server */
	do {
	    printf
		("Do you want to write information about this album to the server [y/n]? ");
	    fflush(stdout);
	    key = fgetc(stdin);	/* read the option    */
	    if (key != 10)	/* if option != ENTER */
		fgetc(stdin);	/* read the ENTER key */
	    fflush(stdin);	/* flush stdin        */
	}
	while ((toupper(key) != 'Y') && (toupper(key) != 'N'));

	/* 
	 * get information from the user -- this will read information, save the info into 
	 * the disk and send the information to the CDDB server.
	 */
	if (toupper(key) == 'Y') {
	    close(server_fd);
	    filename =
		get_from_user(discID, tracks, offsets, total_secs, server,
			      address);
	    return (filename);
	}

	close(server_fd);
	return (NULL);
    }

    /* read from the server */
    backup = VERBOSE;		/* create a backup of our current VERBOSE option */
    VERBOSE = 0;		/* disable VERBOSE because of the great 'cddb read' flood */

    snprintf(msg, sizeof(msg), "cddb read %s %s\r\n", categ, newID);
    answer = send_to_server(server_fd, msg);
    if (!answer) {
	printf("could not receive the informations from %s\n", address);
	close(server_fd);
	return (NULL);
    }
    VERBOSE = backup;		/* restore VERBOSE option. */

    /* save the output into the disc */
    if (VERBOSE) {
	printf("Saving CDDB information into %s/%s ...\n", REAL_PATH,
	       newID);
	printf("save_to_disk(%s)\n", answer);
    }

    filename = save_to_disk(categ, discID, answer);
    if (!filename) {
	printf("could not create the file %s/%s, check permission\n",
	       categ, newID);
	close(server_fd);
	return (NULL);
    }

    if (VERBOSE)
	puts("");

    /* close the fd */
    close(server_fd);

    return (filename);
}


/*
 * get_from_user - read information about the disc from the user and send it to the server.
 * 		           return NULL on error, else return the filename.
 */
char *get_from_user(discID, tracks, offsets, total_secs, server_name,
		    address)
int discID, tracks, total_secs;
char *offsets, *server_name, *address;
{
    int info, i, server_fd;
    char artist[61], album[61], email[61], tmp[61], categ[11];
    char header[BUFFER_SIZE], body[BUFFER_SIZE];
    char *filename, *token, **t, *answer;


    /* 
     * read the artist name -- and repeat, case the user just press ENTER 
     */
    printf
	("---- WARNING ----\nThis function has not been tested yet! It still has to be implemented!\n");
    printf("\nPlease correctly fill the following data\n");
    do {
	printf("Artist Name:      ");
	fgets(artist, 60, stdin);
	fflush(stdin);
    }
    while (strlen(artist) == 1);

    for (i = strlen(artist); i > 0; i--)
	if (artist[i] == '\n')
	    artist[i] = '\0';

    /* read the album name */
    do {
	printf("Album name:       ");
	fgets(album, 60, stdin);
	fflush(stdin);
    }
    while (strlen(album) == 1);

    do {
	printf("Category:\n"
	       "\t[01]-blues\n"
	       "\t[02]-classical\n"
	       "\t[03]-country\n"
	       "\t[04]-data\n"
	       "\t[05]-folk\n"
	       "\t[06]-jazz\n"
	       "\t[07]-misc\n"
	       "\t[08]-newage\n"
	       "\t[09]-reggae\n" "\t[10]-rock\n" "\t[11]-soundtrack\n");
	printf("Album Category:   ");
	scanf("%d", &info);
    }
    while ((info < 1) || (info > 11));

    getchar();
    fflush(stdin);

    switch (info) {
    case 1:
	strcpy(categ, "blues");
	break;
    case 2:
	strcpy(categ, "classical");
	break;
    case 3:
	strcpy(categ, "country");
	break;
    case 4:
	strcpy(categ, "data");
	break;
    case 5:
	strcpy(categ, "folk");
	break;
    case 6:
	strcpy(categ, "jazz");
	break;
    case 7:
	strcpy(categ, "misc");
	break;
    case 8:
	strcpy(categ, "newage");
	break;
    case 9:
	strcpy(categ, "reggae");
	break;
    case 10:
	strcpy(categ, "rock");
	break;
    case 11:
	strcpy(categ, "soundtrack");
    default:
	break;
    }

    /* get the tracks names */
    t = (char **) Malloc(sizeof(char *) * tracks);
    printf("Track names (%d)\n", tracks);
    for (i = 0; i < tracks; i++) {
	do {
	    printf("track %02d: ", i + 1);
	    fgets(tmp, 60, stdin);
	    fflush(stdin);
	}
	while ((strlen(tmp) == 1)
	       || ((strstr(tmp, "track")) && (strlen(tmp) < 9)
		   && (strlen(tmp) > 5)));

	t[i] = strdup(tmp);
    }

    do {
	printf("Enter your email: ");
	fflush(stdout);
	fgets(email, 60, stdin);
	fflush(stdin);
    }
    while (strlen(email) == 1);

    /* ask the user to confirm the typed data */
    printf("Confirm typed information [Y/N]? ");
    fflush(stdout);
    fgets(tmp, 60, stdin);
    fflush(stdin);
    if (toupper(tmp[0]) != 'Y')
	return (NULL);

    /*
     * generate the file body, using the information typed from the user
     */
    token = strtok(offsets, " ");
    snprintf(body, sizeof(body),
	     "# xmcd CD database file\n#\n# Track frame offsets:\n");
    while (token) {
	strcat(body, "#     ");
	strcat(body, token);
	strcat(body, "\n");
	token = strtok(NULL, " \t");
    }

    Free(token);
    strcat(body, "# \n# Disc length: ");
    snprintf(tmp, sizeof(tmp), "%d", total_secs);
    strcat(body, tmp);
    strcat(body, " seconds\n# Revision: 1\n# Processed by: cddbd ");

    server_name = strtok(server_name, " \t");
    for (i = 0; i < 4; i++)
	server_name = strtok(NULL, " \t");
    strcat(body, server_name);
    strcat(body,
	   " Copyright (c) 1996-1997 Steve Scherf\n# Submitted via: Lazy ");
    strcat(body, LAZY_VERSION);
    strcat(body, "\n#\nDISCID=");
    sprintf(tmp, "%08x", discID);
    strcat(body, tmp);
    strcat(body, "\nDTITLE=");
    strcat(body, artist);
    strcat(body, " / ");
    strcat(body, album);
    for (i = 0; i < tracks; i++) {
	snprintf(tmp, sizeof(tmp), "TTITLE%d=%s", i, t[i]);
	strcat(body, tmp);
    }
    strcat(body, "EXTD=\n");
    for (i = 0; i < tracks; i++) {
	Free(t[i]);		/* free t[i]'s memory area */
	sprintf(tmp, "EXTT%d=\n", i);
	strcat(body, tmp);
    }
    Free(t);
    strcat(body, "PLAYORDER=\n");

    printf("\nSaving information to the disk ... ");
    fflush(stdout);
    filename = save_to_disk(categ, discID, body);
    if (!filename) {
	printf("ERROR\n");
	printf("could not create the file %s/%08x, check permission\n",
	       categ, discID);
	return (NULL);
    } else
	printf("OK\n");


    printf("Sending information to the server (%s)... \n", address);
    server_fd = create_socket((unchar *) address, 80);
    if (server_fd < 0) {
	printf("FAILED\n");
	return (NULL);
    }

    /* 'header' holds the request to the cddb server */
    snprintf(header, sizeof(header), "POST /~cddb/submit.cgi HTTP/1.0\n"
	     "Category: %s\nDiscid: %08x\nUser-Email: %s"
	     "Submit-Mode: test\nContent-Length: %d\n",
	     categ, discID, email, strlen(body) * sizeof(char));
    snprintf(header, sizeof(header), "%s\n%s", header, body);

    answer = send_to_server(server_fd, header);
    if (!answer) {
	if (VERBOSE)
	    printf("%s\n", answer);
	return (NULL);
    }

    /*
     * This output is really big.. I don't think this will be useful for final users.
     * So, I added an #ifdef DEBUG to it.
     */
#ifdef DEBUG
    if (VERBOSE)
	printf("<- %s\n", answer);
#endif

    /* since the content of the CD is saved, return the filename generated */
    return (filename);
}


/*
 * save_to_disk - receive the subdir, cdID and the message, and save the information into the cddb 
 *                directory. This function returns the filename on success, or NULL on error.
 */
char *save_to_disk(subdir, cdID, message)
char *subdir, *message;
int cdID;
{
    FILE *destination;
    char *path, *retval;
    char new[strlen(message)], filename[strlen(message) + 9];
    int i;

    /* check if we already have the subdir created */
    path =
	(char *) Malloc((2 + strlen(subdir) + strlen(REAL_PATH)) *
			sizeof(char));
    sprintf(path, "%s/%s", REAL_PATH, subdir);

    /* check if we have the directory in the disk */
    if (!(opendir(path))) {
	/* given directory doesn't exist */
	if (VERBOSE)
	    printf("directory %s doesn't exist, trying to create it.\n",
		   path);

	/* try to create it.. */
	if ((mkdir(path, 0744)) < 0) {
	    fprintf(stderr, "mkdir %s: %s\n", path, strerror(errno));
	    return (NULL);
	} else {
	    if (VERBOSE)
		printf("directory created successfully\n");
	}
    }

    /* save it into the disc */
    snprintf(filename, sizeof(filename), "%s/%s/%08x", REAL_PATH, subdir,
	     cdID);
    retval = strdup(filename);

    /* create the file */
    destination = fopen(filename, "w");
    if (!destination) {
	fprintf(stderr, "fopen %s: %s\n", filename, strerror(errno));
	return (NULL);
    }

    /* copy the new string content into the file */
    i = 0;
    while (message[i] != '\n')
	i++;
    i++;
    memcpy(new, &message[i], strlen(message) - i);
    fputs(new, destination);

    /* close the file */
    fclose(destination);

    return (retval);
}
