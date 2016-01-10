#ifndef _utils_h
#define _utils_h 1

#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>

void		err_quit (char *, ...);
void		err_sys (char *, ...);
void		write_error (int, char *, va_list);
float		mod (float);
char *		get_username (void);

#endif /* _utils_h */
