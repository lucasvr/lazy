/*
 * ---------------------------------------------------------------------
 *  Lazy - play a CD and print actual track informations into stdout
 * ---------------------------------------------------------------------
 *
 * File: socket.c
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

#include "playcd.h"
#include "socket.h"


/*
 * create_socket - create a socket to communicate with the remote server
 * return the fd' int on success, or -1 on error.
 */
int create_socket(unchar_address, int_port)
unchar *unchar_address;
int int_port;
{
    int sock, len;
    struct hostent *remote;
    struct sockaddr_in server;
    ushort port = (ushort) (int_port);
    ulong address, temp;


    /* get the *remote* server information */
    remote = gethostbyname(unchar_address);
    if (!remote) {
	printf("%s\n", strerror(errno));
	return (-1);
    }
    bcopy((char *) remote->h_addr, (char *) &temp, remote->h_length);

    /* convert the 32bit long value from *network byte order* to the *host byte order* */
    address = ntohl(temp);

    /* create the address of the CDDB server, filling the server's mem_area with 0 values */
    len = sizeof(struct sockaddr_in);
    memset(&server, 0, len);
    server.sin_family = AF_INET;	/* set the address as being in the internet domain      */
    server.sin_port = htons(port);	/* set the port address of the server                           */
    server.sin_addr.s_addr = htonl(address);

    /* create a socket in the INET domain */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
	perror("socket");
	return (-1);
    }

    /* connect to the server */
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
	printf("%s\n", strerror(errno));
	return (-1);
    }

    return (sock);
}


/*
 * sent_to_server - send a message to the server, and return the server response on 
 * 					success, or NULL on error
 */
char *send_to_server(server_fd, message)
int server_fd;
char *message;
{
    ssize_t total;
    int len = BUFFER_SIZE * 8;
    char *response, temp[len];

    /* write 'message' to the server */
    if (send(server_fd, message, strlen(message), MSG_DONTWAIT) < 0) {
	printf("%s: %s", message, strerror(errno));
	return (NULL);
    }

    /* if VERBOSE is ON, print the message sent to the server */
    if (VERBOSE)
	printf("-> %s", message);

    /* read the response from the server */
    total = 0;
    do {
	total += read(server_fd, temp + total, len - total);
	if (total < 0) {
	    printf("%s\n", strerror(errno));
	    return (NULL);
	}
    }
    while (total > 2 && temp[total - 2] != '\r');

    temp[total - 2] = '\0';	/* temp[total-1] == \r; temp[total] == \n       */
    response = strdup(temp);	/* duplicate the response from the server       */

    /* if the VERBOSE mode is turned ON, print the response from the server */
    if (VERBOSE)
	printf("<- %s\n", response);

    return (response);
}
