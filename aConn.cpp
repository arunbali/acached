/*
 * conn.cpp
 *
 *  Created on: Apr 24, 2011
 *      Author: awara
 */

#include "aConn.h"
#include <errno.h>

aConn::aConn(int cType, int cSockFd) {

	this->cType = cType;
	this->cSockFd = cSockFd;

	this->PendingCmdQ = new _adapt_::adaptSharedQ(1234);
	this->iovReq.iovSize = ACONN_IOV_REQ_SIZE_INIT;
	this->iovReply.iovSize = ACONN_IOV_REPLY_SIZE_INIT;

	pthread_spin_init(&(this->spinlock), 0);

	this->iovReq.iovec = (struct iovec *) malloc(
			this->iovReq.iovSize * sizeof(struct iovec));
	//this->iovReq.iovDesc = (aConnIovDesc  *)calloc(this->iovReq.iovSize, sizeof(aConnIovDesc));
	this->iovReq.iovDesc = (std::bitset<64> *) malloc(
			this->iovReq.iovSize * sizeof(std::bitset<64> *));

	this->iovReq.iovec_base_offset = (uint32_t *) calloc(this->iovReq.iovSize,
			sizeof(uint32_t));
	this->iovReq.iovec_alloc_size = (uint32_t *) calloc(this->iovReq.iovSize,
			sizeof(uint32_t));

	this->iovReply.iovec = (struct iovec *) calloc(this->iovReply.iovSize,
			sizeof(struct iovec));
	this->iovReply.iovec_base2 = (char **) calloc(this->iovReply.iovSize,
			sizeof(char *));

	//this->iovReply.my = (std::bitset<16>  *)calloc(this->iovReply.iovSize, sizeof(std::bitset<16>));

	this->iovReply.iovDesc = (std::bitset<64> *) calloc(this->iovReply.iovSize,
			sizeof(std::bitset<64> *));
	this->iovReply.iovec_base_offset = (uint32_t *) calloc(
			this->iovReply.iovSize, sizeof(uint32_t));
	this->iovReply.iovec_alloc_size = (uint32_t *) calloc(
			this->iovReply.iovSize, sizeof(uint32_t));

	this->reqCount = 0;
	this->replyCount = 0;
	ACONN_IOVREPLYINDEX(this) = 0;
	this->writingflag = 0;
	this->windexlast = 0;
	this->totalsize = 0;
	this->Busy = 0;
	this->dataPendingRead = 0;

	SET_AC_IOVREPLY_DESC(this,0,AC_IOVREPLY_DESC_INIT);

	this->Reset();
}

aConn::~aConn() {
}

void aConn::Init(int cType, int cSockFd) {
	this->cType = cType;
	this->cSockFd = cSockFd;
	this->Reset();

	return;
}
aConn *aConn::Reset() {

	this->state = cmd_wait;
	this->dirtyFd = 0;
	this->refcount = 0;
	this->Concurrent = 0;
	this->aerrnum = 0;
	this->aerrtype = 0;
	this->concPending = 0;
	ACONN_IOVREQINDEX(this) = 0;
	this->iovReq.iovSize = ACONN_IOV_REQ_SIZE_INIT;
	this->emptyQ = 0;
	this->conWatchSent = 0;

	memset(this->iovReq.iovec, 0x0,
			this->iovReq.iovSize * sizeof(struct iovec));
	memset(this->iovReq.iovDesc, 0x0,
			this->iovReq.iovSize * sizeof(aConnIovDesc));
	return (aConn *) NULL;
}

uint32_t aConn::Assign() {
	return atomicIncrAddGet(&(this->refcount));
}

uint32_t aConn::Revoke() {
	return atomicDecrSubGet(&(this->refcount));

}
uint32_t aConn::IsdirtyFd() {

	return atomicIsSet(&(this->dirtyFd));

}

uint32_t aConn::dirtyFdSet() {

	return atomicSet(&(this->dirtyFd));

}

uint32_t aConn::dirtyFdReset() {

	return atomicReset(&(this->dirtyFd));

}

uint32_t aConn::IsConcurrent() {

	return atomicIsSet(&(this->Concurrent));

}

uint32_t aConn::ConcurrentSet() {

	return atomicSet(&(this->Concurrent));

}

uint32_t aConn::ConcurrentReset() {

	return atomicReset(&(this->Concurrent));

}

uint32_t aConn::IsconcPending() {

	return atomicIsSet(&(this->concPending));

}

uint32_t aConn::concPendingSet() {

	return atomicSet(&(this->concPending));

}

uint32_t aConn::concPendingReset() {

	return atomicReset(&(this->concPending));

}

void PrintaConn(void *ptr) {
	aConn *aC = (aConn *) ptr;
	//printf("(%p)rcount(%d)concurrent(%d)dirtyFd(%d)\n",aC, aC->refcount,aC->Concurrent, aC->dirtyFd);
	return;

}

int aConn::FreeIov(int index) {
	free(ACONN_IOVREQ_IOVECBASE(this,index));
	// reset values
			ACONN_IOVREQ_IOVECBASE(this,index) = NULL;
			ACONN_IOVREQ_IOVECLEN(this,index) = 0;
			ACONN_IOVREQ_BASEOFFSET(this,index) = 0;
			ACONN_IOVREQ_ALLOCSIZE(this,index) = 0;
			//ACONN_IOVREQ_DESC(this,index) = init;

			return 0;
		}

void aConn::SetError() {

	this->aerrnum = errno;
	this->aerrtype = AERR_TYPE_SYS;

}

void aConn::SetError(int aerrnum) {

	this->aerrnum = aerrnum;
	this->aerrtype = AERR_TYPE_APP;

}

int aConn::Release() {
	MsgPipe_t AddFd;
	int ret, ret2;

	close(this->cSockFd);
	AddFd.val.fd = this->cSockFd;
	AddFd.initType = 'R';
	this->cSockFd = -1;
	DBUG(ALOG_TCONN, "ConnWatchPut:fds1:%d ", this->aTcommptr->fds[1]);
	ret = write(this->aTcommptr->fds[1], &AddFd, sizeof(MsgPipe_t));
	//release in threadcomm
	return 0;
}

int aConn::CheckPendingCmdQ() {
	MsgPipe_t AddFd;
	int ret, ret2;

	//AddFd.val.aC = this;
	AddFd.val.fd = this->cSockFd;
	AddFd.initType = 'P';
	AddFd.pad[2] = '\n';
	ret = write(this->aTcommptr->fds[1], &AddFd, sizeof(MsgPipe_t));
	DBUG(ALOG_TCONN,
			"CheckPendingCmdQ:fds1:%d ret:%d errno:%d ", this->aTcommptr->fds[1], ret, errno);

	//release in threadcomm
	return 0;
}

