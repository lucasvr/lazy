#include "utils.h"
#include "playcd.h"

void err_sys(char *message, ...)
{
    va_list list;

    va_start(list, message);
    write_error(1, message, list);
    va_end(list);
    exit(EXIT_SUCCESS);
}


void err_quit(char *message, ...)
{
    va_list list;


    va_start(list, message);
    write_error(0, message, list);
    va_end(list);

    exit(EXIT_SUCCESS);
}


void write_error(errnoflag, message, list)
int errnoflag;
char *message;
va_list list;
{
    int err;
    char info[strlen(message) + strlen(strerror(errno)) + 5];


    err = errno;
    vsprintf(info, message, list);
    if (errnoflag) {
	strcat(info, ": ");
	strcat(info, strerror(err));
    }
    strcat(info, "\n");
    fflush(stdout);		/* case stdout and stderr are the same */
    fputs(info, stderr);
    fflush(stderr);
}


float mod(value)
float value;
{
    if (value < 0.0)
	return (value * -1.0);
    else
	return (value);
}

char *get_username(void)
{
    uid_t uid;
    struct passwd *pwp = NULL;

    uid = getuid();
    pwp = getpwuid(uid);
    if (!pwp) {
	perror("getpwuid");
	return NULL;
    }

    return (pwp->pw_name);
}
