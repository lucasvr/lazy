#ifndef _wrappers_h
#define _wrappers_h 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif


ssize_t 	Read (int, void *, size_t);
ssize_t 	Read_n (int, void *, size_t);
ssize_t 	Write (int, const void *, size_t);
void		*Malloc (size_t);
void		*Calloc (size_t, size_t);
void		*Realloc (void *, size_t);
void		Free (void *);
int 		Mlock (const void *, size_t);
int 		Munlock (const void *, size_t);


#endif /* _wrappers_h */
