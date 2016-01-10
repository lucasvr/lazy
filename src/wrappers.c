/*
 * ---------------------------------------------------------------------
 *  Lazy - play a CD and print actual track informations into stdout
 * ---------------------------------------------------------------------
 *
 * File: wrappers.c
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

int Mlock(addr, len)
const void *addr;
size_t len;
{
    int retval;

    retval = mlock(addr, len);
    if (retval < 0)
	perror("mlock");
    return retval;
}

int Munlock(addr, len)
const void *addr;
size_t len;
{
    int retval;

    retval = munlock(addr, len);
    if (retval < 0)
	perror("munlock");
    return retval;
}

void *Malloc(size)
size_t size;
{
    void *retval = NULL;

    retval = malloc(size);
    if (!retval) {
	perror("malloc");
	free_globals();
	exit(EXIT_FAILURE);
    }

    return retval;
}


void *Calloc(nmemb, size)
size_t nmemb, size;
{
    void *retval = NULL;

    retval = calloc(nmemb, size);
    if (!retval) {
	perror("calloc");
	free_globals();
	exit(EXIT_FAILURE);
    }

    return retval;
}

void *Realloc(ptr, size)
void *ptr;
size_t size;
{
    void *retval = NULL;

    retval = realloc(ptr, size);
    if (!retval) {
	perror("realloc");
	free_globals();
	exit(EXIT_FAILURE);
    }
    return retval;
}

void Free(ptr)
void *ptr;
{
    if (ptr)
	free(ptr);
    return;
}

ssize_t Read(fd, buf, count)
int fd;
void *buf;
size_t count;
{
    ssize_t retval;

    retval = read(fd, buf, count);
    if (retval < 0) {
	fprintf(stdout, "fd %d: read: %s\n", fd, strerror(errno));
	free_globals();
	exit(EXIT_FAILURE);
    }

    return retval;
}

ssize_t Read_n(fd, buf, count)
int fd;
void *buf;
size_t count;
{
    ssize_t retval = 0, n = 0;

    do {
	retval = read(fd, buf + n, count - n);
	if (retval < 0) {
	    fprintf(stdout, "fd %d: read: %s\n", fd, strerror(errno));
	    free_globals();
	    exit(EXIT_FAILURE);
	}

	n += retval;
    }
    while (n < count);

    return n;
}

ssize_t Write(fd, buf, count)
int fd;
const void *buf;
size_t count;
{
    ssize_t retval;

    retval = write(fd, buf, count);
    if (retval < 0) {
	fprintf(stdout, "fd %d: write: %s\n", fd, strerror(errno));
	free_globals();
	exit(EXIT_FAILURE);
    }

    return retval;
}
