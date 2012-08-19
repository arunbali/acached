/*
 * aUtil.cpp
 *
 *  Created on: May 14, 2011
 *      Author: arun
 */



#include <stdio.h>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

/* Avoid warnings on solaris, where isspace() is an index into an array, and gcc uses signed chars */
#define xisspace(c) isspace((unsigned char)c)
extern pthread_mutex_t logmutex;
#define	IS_UDP(x)	0

void hexdump(char *data, int size, char *caption, int flag) {

}

void hexdump1(char *data, int size, char *caption, int flag) {
	int i; // index in data...
	int j; // index in line...
	char temp[8];
	char buffer[128];
	char *ascii;

	if (!flag)
		return;

	pthread_mutex_lock(&logmutex);

	fprintf(stderr, "---------> %s <--------- (%d bytes from %p)\n", caption,
			size, data);
	if (!(size > 0))
		return;
	memset(buffer, 0, 128);

	// Printing the ruler...
	fprintf(stderr,
			"        +0          +4          +8          +c            0   4   8   c   \n");

	// Hex portion of the line is 8 (the padding) + 3 * 16 = 52 chars long
	// We add another four bytes padding and place the ASCII version...
	ascii = buffer + 58;
	memset(buffer, ' ', 58 + 16);
	buffer[58 + 16] = '\n';
	buffer[58 + 17] = '\0';
	buffer[0] = '+';
	buffer[1] = '0';
	buffer[2] = '0';
	buffer[3] = '0';
	buffer[4] = '0';
	for (i = 0, j = 0; i < size; i++, j++) {
		if (j == 16) {
			fprintf(stderr, "%s", buffer);
			memset(buffer, ' ', 58 + 16);

			sprintf(temp, "+%04x", i);
			memcpy(buffer, temp, 5);

			j = 0;
		}

		sprintf(temp, "%02x", 0xff & data[i]);
		memcpy(buffer + 8 + (j * 3), temp, 2);
		if ((data[i] > 31) && (data[i] < 127))
			ascii[j] = data[i];
		else
			ascii[j] = '.';
	}

	if (j != 0)
		fprintf(stderr, "%s", buffer);
	pthread_mutex_unlock(&logmutex);

}

int new_socket(struct addrinfo *ai) {
	int sfd;
	int flags;

	if ((sfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
		return -1;
	}
	/*
	 if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
	 fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
	 perror("setting O_NONBLOCK");
	 close(sfd);
	 return -1;
	 }
	 */
	return sfd;
}

int setNonblocking(int fd) {
	int flags;

	/* If they have O_NONBLOCK, use the Posix way to do it */

//#if defined(O_NONBLOCK)
	/* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
	//printf("SETTING NON BLOCKING POSIX");
	if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
//#else
	/* Otherwise, use the old way of doing it
	 flags = 1;
	 return ioctl(fd, FIOBIO, &flags);
	 */
//#endif
}

int server_socket(const char *interface, int port, int transport) {
	int sfd;
	struct linger ling = { 0, 0 };
	struct addrinfo *ai;
	struct addrinfo *next;
	struct addrinfo hints;
	char port_buf[NI_MAXSERV];
	int error;
	int success = 0;
	int flags = 1;
	int sendbuff;
	socklen_t optlen;

	memset(&hints, sizeof(struct addrinfo), 0x0);
	hints.ai_socktype = IS_UDP(transport) ? SOCK_DGRAM : SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;

	if (port == -1) {
		port = 0;
	}
	snprintf(port_buf, sizeof(port_buf), "%d", port);
	error = getaddrinfo(interface, port_buf, &hints, &ai);
	if (error != 0) {
		if (error != EAI_SYSTEM)
			fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(error));
		else
			perror("getaddrinfo()");
		return 1;
	}

	for (next = ai; next; next = next->ai_next) {
		//conn *listen_conn_add;
		if ((sfd = new_socket(next)) == -1) {
			/* getaddrinfo can return "junk" addresses,
			 * we make sure at least one works before erroring.
			 */
			if (errno == EMFILE) {
				/* ...unless we're out of fds */
				perror("server_socket");
				exit(1);
			}
			continue;
		}

#ifdef IPV6_V6ONLY
		if (next->ai_family == AF_INET6) {
			error = setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, (char *) &flags,
					sizeof(flags));
			if (error != 0) {
				perror("setsockopt");
				close(sfd);
				continue;
			}
		}
#endif
		fprintf(stderr, " fd is %d \n", sfd);

		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *) &flags,
				sizeof(flags));
		if (IS_UDP(transport)) {
			// maximize_sndbuf(sfd);
		} else {
			error = setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *) &flags,
					sizeof(flags));
			if (error != 0)
				perror("setsockopt");

			error = setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void *) &ling,
					sizeof(ling));
			if (error != 0)
				perror("setsockopt");

			error = setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *) &flags,
					sizeof(flags));
			if (error != 0)
				perror("setsockopt");

			/** Get buffer size
			 optlen = sizeof(sendbuff);
			 error = getsockopt(sfd, SOL_SOCKET, SO_SNDBUF, &sendbuff, &optlen);

			 if(error == -1)
			 printf("Error getsockopt one");
			 else
			 printf("send buffer size = %d\n", sendbuff);
			 **/

		}

		if (bind(sfd, next->ai_addr, next->ai_addrlen) == -1) {
			if (errno != EADDRINUSE) {
				perror("bind()");
				close(sfd);
				freeaddrinfo(ai);
				return 1;
			}
			close(sfd);
			continue;
		} else {
			success++;
			if (!IS_UDP(transport) && listen(sfd, 1024) == -1) {
				perror("listen()");
				close(sfd);
				freeaddrinfo(ai);
				return 1;
			}
		}

	}

	freeaddrinfo(ai);

	/* Return zero iff we detected no errors in starting up connections */
	return sfd;

}

int wait_for_io_or_timeout(int sock, int for_read, int timeout) {
	struct timeval tv, *tvptr;
	fd_set fdset;
	int ret;
	do {
		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);
		if (timeout < 0) {
			tvptr = NULL;
		} else {
			tv.tv_sec = timeout;
			tv.tv_usec = 0;
			tvptr = &tv;
		}
		if (for_read) {
			ret = select(sock + 1, &fdset, NULL, NULL, tvptr);
		} else {
			ret = select(sock + 1, NULL, &fdset, NULL, tvptr);

		}

		/* TODO - timeout should be smaller on repeats of this loop */
	} while (ret == -1 && errno == EINTR);
	return ret;
}

bool safe_strtoull(const char *str, uint64_t *out) {
	assert(out != NULL);
	errno = 0;
	*out = 0;
	char *endptr;
	unsigned long long ull = strtoull(str, &endptr, 10);
	if ((errno == ERANGE) || (str == endptr)) {
		return false;
	}

	if (xisspace(*endptr) || (*endptr == '\0' && endptr != str)) {
		if ((long long) ull < 0) {
			/* only check for negative signs in the uncommon case when
			 * the unsigned number is so big that it's negative as a
			 * signed number. */
			if (strchr(str, '-') != NULL) {
				return false;
			}
		}
		*out = ull;
		return true;
	}
	return false;
}

bool safe_strtoll(const char *str, int64_t *out) {
	assert(out != NULL);
	errno = 0;
	*out = 0;
	char *endptr;
	long long ll = strtoll(str, &endptr, 10);
	if ((errno == ERANGE) || (str == endptr)) {
		return false;
	}

	if (xisspace(*endptr) || (*endptr == '\0' && endptr != str)) {
		*out = ll;
		return true;
	}
	return false;
}

bool safe_strtoul(const char *str, uint32_t *out) {
	char *endptr = NULL;
	unsigned long l = 0;
	assert(out);
	assert(str);
	*out = 0;
	errno = 0;

	l = strtoul(str, &endptr, 10);
	if ((errno == ERANGE) || (str == endptr)) {
		return false;
	}

	if (xisspace(*endptr) || (*endptr == '\0' && endptr != str)) {
		if ((long) l < 0) {
			/* only check for negative signs in the uncommon case when
			 * the unsigned number is so big that it's negative as a
			 * signed number. */
			if (strchr(str, '-') != NULL) {
				return false;
			}
		}
		*out = l;
		return true;
	}

	return false;
}

bool safe_strtol(const char *str, int32_t *out) {
	assert(out != NULL);
	errno = 0;
	*out = 0;
	char *endptr;
	long l = strtol(str, &endptr, 10);
	if ((errno == ERANGE) || (str == endptr)) {
		return false;
	}

	if (xisspace(*endptr) || (*endptr == '\0' && endptr != str)) {
		*out = l;
		return true;
	}
	return false;
}

void vperror(const char *fmt, ...) {
	int old_errno = errno;
	char buf[1024];
	/**
	 va_list ap;

	 va_start(ap, fmt);
	 if (vsnprintf(buf, sizeof(buf), fmt, ap) == -1) {
	 buf[sizeof(buf) - 1] = '\0';
	 }
	 va_end(ap);
	 **/errno = old_errno;

	perror(buf);
}

#ifndef HAVE_HTONLL
static uint64_t mc_swap64(uint64_t in) {
#ifdef ENDIAN_LITTLE
	/* Little endian, flip the bytes around until someone makes a faster/better
	 * way to do this. */
	int64_t rv = 0;
	int i = 0;
	for(i = 0; i<8; i++) {
		rv = (rv << 8) | (in & 0xff);
		in >>= 8;
	}
	return rv;
#else
	/* big-endian machines don't need byte swapping */
	return in;
#endif
}

uint64_t ntohll(uint64_t val) {
	return mc_swap64(val);
}

uint64_t htonll(uint64_t val) {
	return mc_swap64(val);
}
#endif

