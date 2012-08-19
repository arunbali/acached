/*
 * threadComm.cpp
 *
 *  Created on: Apr 1, 2011
 *      Author: awara
 */

#include "acached.h"
#include <event.h>
extern acacheSGA *SGAptr;

int setNonblocking(int fd);

int checkfd(void *ptr, int fd) {
	aConn *aC = (aConn *) ptr;

	DBUG(ALOG_COMM,
			"COMM:checkfd:ptr(%p)sockfd(%d)fd(%d)", aC, aC->cSockFd, fd);
	if (aC->cSockFd == fd)
		return 1;
	else
		return 0;

}

void ProcessNewFd(char *buf, int nbytes, void *arg, int fd);
void commRead_cb(int fd, short ev_flags, void *arg);

int closeConnection(int fd, struct aEvent *aEventptr) {
	aConn *aC;

	event_del(&aEventptr->event);
	close(fd);

	if (aEventptr->aC) {
		aC = aEventptr->aC;
		aC->Reset();
		aC->commRead_wptr = NULL;
		aC->loop = NULL;
		DBUG((ALOG_TCONN|ALOG_COMM|ALOG_ALERT),
				"COMM:Releasing aConn(%p)rcount:%d fd:%d conn-fd:%d", aC, aC->Revoke(), fd, aC->cSockFd);
		SGAptr->aConnRelease(aC);
	}

	DBUG(ALOG_ALERT,
			"FREEING EVENT(%p) event(%p)", aEventptr, &(aEventptr->event));
	free(aEventptr);

	return 0;
}

void commReadMain_cb(int fd, short ev_flags, void *arg) {
	char buf[1024];
	int ret;
	int len;
	int nbytes;
	aConn *aC;
	struct aEvent *aEventptr = (struct aEvent *) arg;

	//DBUG(ALOG_COMM,"COMM:Got read event onfd:%d events:%d revents:%d atcommptr:%p",  w1->io.fd, w1->io.events, revents, w1->aTcommptr);

	DBUG((ALOG_DALL|ALOG_ALERT), "COMM:FDS:Got something on:%d", fd);

	if ((nbytes = read(fd, buf, 1008)) <= 0) {
		if (nbytes == 0) {
			DBUG(ALOG_DALL, "(%d)COMM:FDS:(0):Socket %d HUNG-UP", fd);
		} else {
			perror("recv2");
		}
		closeConnection(fd, aEventptr);
		DBUG(ALOG_ALERT, "!!!!:COMM:FDS:com error");
	} else {
		ProcessNewFd(buf, nbytes, arg, fd);
	}

}

void commRead_cb(int fd, short ev_flags, void *arg) {
	char buf[1024];
	int ret;
	int len;
	int nbytes;
	aConn *aC;
	struct aEvent *aEventptr = (struct aEvent *) arg;

	//DBUG(ALOG_COMM,"COMM:Got read event onfd:%d events:%d revents:%d atcommptr:%p",  w1->io.fd, w1->io.events, revents, w1->aTcommptr);

	/***

	 if ( !w1->aC) {
	 ev_io_stop (loop, &(w1->io));
	 return;
	 }
	 ***/
	DBUG((ALOG_TCONN|ALOG_ALERT),
			"COMM:GotDATA:connptr(%p) aTcommptr(%p) state:%d fd:%d", aEventptr->aC, aEventptr->aTcommptr, aEventptr->aC->state, aEventptr->aC->cSockFd);

	assert(aEventptr->aC->aTcommptr == aEventptr->aTcommptr);

	aC = aEventptr->aC;
	/****
	 if ( aC->cSockFd == -1 ){
	 DBUG(ALOG_ALERT, "COMM:Got FD -1 ");

	 while ( (ret = read(fd,buf,880) ) >= 0 ){
	 //if (ret == 0) ev_io_stop (loop, &(w1->io));

	 DBUG(ALOG_ALERT, "COMM:Got -1 ret:%d",ret);
	 }
	 aEventptr->aC = NULL;
	 closeConnection(fd, aEventptr);
	 return;
	 }
	 ****/
	ret = -1;
	if (aC->state == cmd_wait) {
		//assert(aC->reqCount == 0 );
		//assert(aC->replyCount == 0 );
		if (!(aC->reqCount == 0 || aC->replyCount == 0)) {
			ret = read(fd, buf, 880);
			if (ret == 0) {
				closeConnection(fd, aEventptr);
				return;

			}
			DBUG(ALOG_ALERT,
					"ALERT:COMM:cmd_wait:req:%d reply:%d ret:%d errno:%d", aC->reqCount, aC->replyCount, ret, errno);
			hexdump(buf, ret, "ALERTCOMM", 0);
		}
	} else {
		ret = 1;
	}

	do {
		ret = aC->ProcessDataRecv();
		DBUG(ALOG_COMM,
				"COMM:ProcessDataRecv:ret:%d iovReq.iovIndex:%d ctype:%d state:%d datapendingread:%d", ret, aC->iovReq.iovIndex, aC->cmd.ctype, aC->state, aC->dataPendingRead);
		if (ret > 0 && aC->state == cmd_done) {
			if (aC->cmd.ctype == ACACHED_CMD_SET
					|| aC->cmd.ctype == ACACHED_CMD_GETS
					|| aC->cmd.ctype == ACACHED_CMD_QUIT) {
				aC->state = cmd_wait;
				aC->Post2KVdata();
				// sched_yield();

			}
			if (aC->cmd.ctype == ACACHED_CMD_QUIT)
				break;
			aC->Reset();
		}
	} while (ret > 0 || aC->dataPendingRead);

	if (ret <= 0) {
		if (ret == 0) {
			ALERT(ALOG_ALERT, "close from 2 fd:%d\n", fd);
			closeConnection(fd, aEventptr);
			return;
		}
		if (ret < 0 && aC->aerrnum != EAGAIN)
			ALERT(
					ALOG_ALERT, "COMM:ProcessDataRecv:ERROR:aerrtype:%d aerrnum:%d", aC->aerrtype, aC->aerrnum);
		// return < 0
		if (aC->aerrtype == AERR_TYPE_SYS) {
			if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
				closeConnection(fd, aEventptr);
				return;
			}
		}

		if (aC->aerrtype == AERR_TYPE_APP) {

			len = snprintf(buf, 880, "CLIENT_ERROR %s:%d\r\n", "errno",
					aC->aerrnum);
			aC->WriteData(buf, len);
			aC->Reset();

			return;

		}

	}

}

void ProcessNewFd(char *buf, int nbytes, void *arg, int fd)

{
	char *ptr;
	int x, newfd;
	char initType;
	MsgPipe_t *addFdptr;
	struct aEvent *commReadptr;
	aConn *aC;
	int ret1, ret2, ret3;
	struct aEvent *aEventptr = (struct aEvent *) arg;

	//INTERNAL((ALOG_COMM|ALOG_INTERNAL), "COMM:FDS:RECIEVED:%d expecting:%d",nbytes, sizeof(MsgPipe_t));
	if (nbytes != sizeof(MsgPipe_t)) {
		INTERNAL(
				(ALOG_COMM|ALOG_INTERNAL), "COMM:FDS:RECIEVED:%d expecting:%d", nbytes, sizeof(MsgPipe_t));
		//hexdump(buf,nbytes,"FDS RECV",1);
	}
	x = 0;
	ptr = buf;
	while (x < nbytes) {
		//hexdump(ptr,sizeof(MsgPipe_t),"MSGPIPE",1);

		addFdptr = (MsgPipe_t *) ptr;
		initType = addFdptr->initType;

		if (initType == 'I') {

			newfd = addFdptr->val.fd;
			DBUG(ALOG_TCONN,
					"COMM:Bytes:%d x:%d FD:%d:%c", nbytes, x, newfd, initType);

			setNonblocking(newfd);

			aC = SGAptr->aConnAllocate();
			aC->Init(CONN_TYPE_TCP, newfd);
			aC->aTcommptr = aEventptr->aTcommptr;

			commReadptr = (struct aEvent *) malloc(sizeof(struct aEvent));
			memset(commReadptr, 0x0, sizeof(struct aEvent));
			commReadptr->aTcommptr = aEventptr->aTcommptr;
			commReadptr->aC = aC;
			commReadptr->base = aEventptr->base;
			aC->commRead_wptr = commReadptr;
			aC->loop = aEventptr->base;

			//event_set(&commReadptr->event, newfd, EV_READ|EV_PERSIST, commRead_cb, commReadptr);
			//ret2=event_base_set(aEventptr->base, &commReadptr->event);
			ret1 = event_assign(&commReadptr->event, aEventptr->base, newfd,
					EV_READ | EV_PERSIST, commRead_cb, commReadptr);

			ret3 = event_add(&commReadptr->event, 0);

			DBUG((ALOG_TCONN|ALOG_COMM|ALOG_ALERT),
					"COMM:CONN(%p)NEW fd:%d aTcommptr(%p)base(%p)event(%p)ret:%d:%d:%d", aC, newfd, aC->aTcommptr, aEventptr->base, &commReadptr->event, ret1, ret2, ret3);

		}

		if (initType == 'P') {

			aC = (aConn *) SGAptr->aFBSQGaConn->aFBSQFindPtrBUSY(checkfd,
					addFdptr->val.fd);
			DBUG(ALOG_TCONN,
					"COMM:Bytes:%d x:%d aC:%p:%c", nbytes, x, aC, initType);

			if (aC) {
				DBUG(ALOG_ALERT,
						"COMM:conn(%p)calling Post2KVdataTSQ with NULL", aC);
				aC->Post2KVdataTSQ(NULL);
			}

		}

		/***
		 else if ( initType == 'R') {

		 aC = (aConn *)SGAptr->aFBSQGaConn->aFBSQFindPtrBUSY(checkfd,newfd);
		 DBUG(ALOG_TCONN, "COMM:after checkfd:ptr(%p)", ptr);

		 if (aC != (aConn *)NULL) {
		 if ( aC->commRead_wptr) {
		 ev_io_stop (aC->loop, &(aC->commRead_wptr->io));
		 free(aC->commRead_wptr);
		 }
		 aC->commRead_wptr = NULL;
		 aC->loop = NULL;
		 DBUG((ALOG_TCONN|ALOG_COMM|ALOG_ALERT), "COMM:Releasing aConn(%p)rcount:%d",  aC, aC->Revoke());
		 SGAptr->aConnRelease(aC);
		 } else {
		 ALERT(ALOG_ALERT, "COMM:FD:%d Conn already closed", newfd);
		 }

		 }
		 **/

		ptr += sizeof(MsgPipe_t);
		x += sizeof(MsgPipe_t);
	}

}

void *thrComm(void *Tptr) {
	_adapt_::adaptThreadComm *aTcommptr;
	struct aEvent *aEventMainptr;
	struct event_base *base;
	struct event event;
	short ev_flags;

	// use the default event loop unless you have special needs

	//Thread_SetAffinity(0);

	if (sigignore(SIGPIPE) == -1) {
		perror("failed to ignore SIGPIPE; sigaction");
		exit(1);
	}

	aTcommptr = (_adapt_::adaptThreadComm *) Tptr;

	aEventMainptr = (struct aEvent *) calloc(1, sizeof(struct aEvent));
	if (aEventMainptr == NULL) {
		fprintf(stderr, "Can't allocate aEventn");
		exit(1);
	}

	ALERT(
			ALOG_INFO, "COMM:Start:threadId(%p)Type(%d)Eventptr(%p)", aTcommptr->threadId, aTcommptr->type, aEventMainptr);

	aEventMainptr->aTcommptr = aTcommptr;
	aEventMainptr->aC = NULL;

	//aEventMainptr->base = event_base_new();
	aEventMainptr->base = event_init();

	if (!aEventMainptr->base) {
		fprintf(stderr, "Can't allocate event base\n");
		exit(1);
	}

	ALERT(
			ALOG_INFO, "COMM:Start:threadId(%p)Type(%d)Eventptr(%p)", aTcommptr->threadId, aTcommptr->type, aEventMainptr, &event);

	//event_base_set(aEventMainptr->base, &event);
	setNonblocking(aTcommptr->fds[0]);
	setNonblocking(aTcommptr->fds[1]);

	event_assign(&event, aEventMainptr->base, aTcommptr->fds[0],
			EV_READ | EV_PERSIST, commReadMain_cb, aEventMainptr);

	if (event_add(&event, 0) == -1) {
		fprintf(stderr, "Can't monitor libevent notify pipe\n");
		exit(1);
	}

	//event_set(&event, aTcommptr->fds[0],EV_READ | EV_PERSIST, commReadMain_cb, aEventMainptr);
	//event_base_set(aEventMainptr->base, &event);

	event_base_dispatch(aEventMainptr->base);
	ALERT(
			ALOG_ALERT, "COMM:EXIT:threadId(%p)Type(%d)", aTcommptr->threadId, aTcommptr->type);

	return 0;
}

