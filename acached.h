/*
 * acached.h
 *
 *  Created on: Apr 2, 2011
 *      Author: awara
 */

#ifndef ACACHED_H_
#define ACACHED_H_

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <list>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <signal.h>

#include "acacheconst.h"
#include <adaptlog.h>
#include <adaptMsg.h>
#include <adaptQ.h>
#include <adaptFBQ.h>
#include <adaptSharedQ.h>
#include <adaptFBSharedQ.h>
#include <adaptFBPSharedQ.h>
#include <adaptThread.h>
#include <adaptThreadSQ.h>
#include <adaptThreadComm.h>
#include <adaptThreadedSharedQ.h>
#include <adaptThreadedSharedPort.h>
#include "aConn.h"
#include "aMsg.h"
#include "ahashtable.h"
#include "acacheSGA.h"
#include "platform.h"

typedef struct {
	char initType;
	union {
		aConn *aC;
		int fd;
	} val;
	char pad[3];
} MsgPipe_t;

void hexdump(char *data, int size, char *caption, int flag);
int server_socket(const char *interface, int port, int transport);
int new_socket(struct addrinfo *ai);
int wait_for_io_or_timeout(int sock, int for_read, int timeout);
int setNonblocking(int fd);
bool safe_strtoull(const char *str, uint64_t *out);
bool safe_strtoll(const char *str, int64_t *out);
bool safe_strtoul(const char *str, uint32_t *out);
bool safe_strtol(const char *str, int32_t *out);
void vperror(const char *fmt, ...);
static uint64_t mc_swap64(uint64_t in);
uint64_t ntohll(uint64_t val);
uint64_t htonll(uint64_t val);

#endif /* ACACHED_H_ */
