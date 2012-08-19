#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <signal.h>

#include <event.h>
#include <event2/thread.h>

#define EVENT_DBG_ALL 0xffffffffu

#define ADAPTLOG_MAIN 1
#include <adaptlog.h>
#include "acached.h"
#include "acacheSGA.h"

using namespace std;

#define PORT_NO 11211
#define BUFFER_SIZE 1024
#define LISTEN_BACKLOG 5     // how many pending connections queue will hold
int total_clients = 0; // Total number of connected clients
void *get_in_addr(struct sockaddr *sa);
acacheSGA *SGAptr;

int main(int argc, char *argv[]) {

	int sockfd, new_fd;
	struct sockaddr_in addr;
	int addr_len = sizeof(addr);
	int reuse = 1;
	char buf[80];
	uintptr_t abc;
	int i, mask;
	long int m;
	MsgPipe_t AddFd;
	KVdata xx;
	int ret;
	fd_set fdset;    	// master file descriptor list
	int flag;
	int flags;
	adaptlog aLog("log");

	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];
	int curr_comm_index = 0;

	/*
	 * ignore SIGPIPE signals; we can use errno == EPIPE if we
	 * need that information
	 */
	if (sigignore(SIGPIPE) == -1) {
		perror("failed to ignore SIGPIPE; sigaction");
		exit(1);
	}
	abc: sockfd = server_socket("localhost", PORT_NO, 1);

	fprintf(stderr, " sockfd : %d \n", sockfd);
	//exit(1);

	pthread_mutex_init(&(logmutex), NULL);
	aLog.Init(ALOG_INFO);
	/*
	aLog.Init( ALOG_INFO | ALOG_INTERNAL | ALOG_TCONC | ALOG_TLCK | ALOG_TMSG | ALOG_TCONN);
	aLog.Init(ALOG_TCONN);

	aLog.Init(ALOG_INTERNAL | ALOG_INFO | ALOG_ALERT | ALOG_TERR | ALOG_PARSE);
	aLog.Init(ALOG_CLIENT);
	aLog.Init(ALOG_COMM);
	aLog.Init(ALOG_PROCMD);
	aLog.Init(ALOG_KVDATA | ALOG_TCONN);
	aLog.Init(ALOG_TERR);
	aLog.Init(ALOG_TCONN | ALOG_TMSG | ALOG_MEM);
	aLog.Init(ALOG_TCONC);
	*/
	//event_enable_debug_logging(EVENT_DBG_ALL);
	printf("\nLIBEVENT VERSION:%u(%s)\n", event_get_version_number(),
			event_get_version());
	//evthread_use_pthreads();
	// event_enable_debug_mode();

	acacheSGA G1(1);
	SGAptr = &G1;
	G1.Print();
	//printf(" ssize max is :%u \n ",SSIZE_MAX);
	while (1) {  // main accept() loop
		ret = wait_for_io_or_timeout(sockfd, 1, -1);
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			fprintf(stderr, " sockfd : %d \n", sockfd);
			close(sockfd);
		}

		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
		DBUG(ALOG_DALL, "server: got connection from %s", s);
		//setNonblocking(new_fd);
		printf("new fd is... %d\n", new_fd);

		if (-1 == (flags = fcntl(new_fd, F_GETFL, 0)))
			flags = 0;
		ret = fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);
		flags = fcntl(new_fd, F_GETFL, 0);

		AddFd.val.fd = new_fd;
		AddFd.initType = 'I';
		AddFd.pad[2] = '\n';

		write(SGAptr->aTcomm[curr_comm_index]->fds[1], &AddFd,
				sizeof(MsgPipe_t));

		//fdatasync(SGAptr->aTcomm[curr_comm_index]->fds[1]);
		//DBUG(ALOG_DALL, "main:index:%dsent:%s", curr_comm_index, buf);
		DBUG(ALOG_ALERT,
				"writing to fd:%d ret:%d", SGAptr->aTcomm[curr_comm_index]->fds[1], ret);

		new_fd = -1;
		if (curr_comm_index == (ACACHED_THREAD_COMM_MAX - 1)) {
			curr_comm_index = 0;
		} else {
			curr_comm_index++;
		}

	}		//end while

	exit(1);
	/*
	 pthread_join (thread[0], NULL);
	 pthread_join (thread[1], NULL);
	 pthread_join (thread[2], NULL);
	 pthread_join (thread[3], NULL);
	 */

}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}



int sendall(int s, char *buf, int *len) {
	int total = 0;        // how many bytes we've sent
	int bytesleft = *len; // how many we have left to send
	int n;

	while (total < *len) {
		n = send(s, buf + total, bytesleft, 0);
		if (n == -1) {
			break;
		}
		total += n;
		bytesleft -= n;
	}

	*len = total; // return number actually sent here

	return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

