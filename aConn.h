/*
 * conn.h
 *
 *  Created on: Apr 24, 2011
 *      Author: awara
 */

#ifndef CONN_H_
#define CONN_H_
#include <cstddef>
#include <stddef.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <bitset>
#include <event.h>

#include "acacheSGA.h"
#include "ahashtable.h"
#include <adaptThreadComm.h>
#include "platform.h"

void PrintaConn(void *ptr);

#define	ACONN_IOVREQINDEX(aC)				aC->iovReq.iovIndex
#define	ACONN_IOVREPLYINDEX(aC)				aC->iovReply.iovIndex

#define	ACONN_IOVREQ_DESC(aC,index)			aC->iovReq.iovDesc[index]
#define	ACONN_IOVREQ_BASEOFFSET(aC,index)	aC->iovReq.iovec_base_offset[index]
#define	ACONN_IOVREQ_ALLOCSIZE(aC,index)	aC->iovReq.iovec_alloc_size[index]
#define	ACONN_IOVREQ_IOVECLEN(aC,index)		aC->iovReq.iovec[index].iov_len
#define	ACONN_IOVREQ_IOVECBASE(aC,index)	aC->iovReq.iovec[index].iov_base

#define	ACONN_IOVREPLY_DESC(aC,index)			aC->iovReply.iovDesc[index]
#define	ACONN_IOVREPLY_BASEOFFSET(aC,index)		aC->iovReply.iovec_base_offset[index]
#define	ACONN_IOVREPLY_ALLOCSIZE(aC,index)		aC->iovReply.iovec_alloc_size[index]
#define	ACONN_IOVREPLY_IOVECLEN(aC,index)		aC->iovReply.iovec[index].iov_len
#define	ACONN_IOVREPLY_IOVECBASE(aC,index)		aC->iovReply.iovec[index].iov_base

#define	ACONN_IOVREPLY_IOVECBASE2(aC,index)		aC->iovReply.iovec_base2[index]

#define	AC_IOVREPLY_DESC_WRITTEN		0
#define	AC_IOVREPLY_DESC_INIT			1
#define	AC_IOVREPLY_DESC_MALLOC			2
#define	AC_IOVREPLY_DESC_FREE			3
#define	AC_IOVREPLY_DESC_BASE_OFFSET	4

#define SET_AC_IOVREPLY_DESC(aC,index,val)		ACONN_IOVREPLY_DESC(aC,index).set(val)
#define RESET_AC_IOVREPLY_DESC(aC,index,val)	ACONN_IOVREPLY_DESC(aC,index).reset(val)
#define ISSET_AC_IOVREPLY_DESC(aC,index,val)	ACONN_IOVREPLY_DESC(aC,index).test(val)

#define	AC_IOVREQ_DESC_HDR			0
#define	AC_IOVREQ_DESC_HDR_PART		1
#define	AC_IOVREQ_DESC_BODY			2
#define	AC_IOVREQ_DESC_BODY_PART	3

#define SET_AC_IOVREQ_DESC(aC,index,val)		ACONN_IOVREQ_DESC(aC,index).set(val)
#define RESET_AC_IOVREQ_DESC(aC,index,val)		ACONN_IOVREQ_DESC(aC,index).reset(val)
#define ISSET_AC_IOVREQ_DESC(aC,index,val)		ACONN_IOVREQ_DESC(aC,index).test(val)

struct aEvent {
	struct event_base *base;
	struct event event;
	_adapt_::adaptThreadComm *aTcommptr;
	aConn *aC;

};

//IOV_MAX

typedef struct {
	struct iovec *iovec;
	char **iovec_base2;
	std::bitset<64> *iovDesc;

//	aConnIovDesc		*iovDesc;
	uint32_t *iovec_base_offset;
	uint32_t *iovec_alloc_size;
	volatile uint32_t iovIndex;
	uint32_t iovIndexHdr;
	int iovSize;
} aConnIov_t;

typedef struct {
	aCmdType ctype;
	char key[MAX_KEY_LEN + 1];
	uint32_t keylen;
	uint32_t flags;
	uint32_t exptime;
	uint32_t datalen;
} aConnCmd_t;

class aConn {
public:
	aConn(int ctype, int cSockFd);
	virtual ~aConn();
	void Init(int cType, int cSockFd);
	uint32_t Assign();
	uint32_t Revoke();
	uint32_t ConcurrentSet();
	uint32_t ConcurrentReset();
	uint32_t IsConcurrent();
	uint32_t dirtyFdSet();
	uint32_t dirtyFdReset();
	uint32_t IsdirtyFd();
	uint32_t concPendingSet();
	uint32_t concPendingReset();
	uint32_t IsconcPending();
	int ConnWatchPut();
	int ConnWatchRemove();
	void SetError(int aerrnum);
	void SetError();
	aConn *Reset();
	int WriteData(char *buf, int len);
	bool CheckIOVTotalWritten(int start, int count, int *iovEnd,uint32_t *totalptr, uint32_t writeret);
	int WriteIOVReply(int start, int count, bool final);
	uint32_t AddIOVReply(char *ptr, int len, bool do_malloc, uint32_t index);
	int QReply(KVdata *KVdataptr, char *key);
	int WriteAllOnce(int start, int iovCount);
	int WriteAllOnceIMM(int start, int iovCount);
	int ProcessDataRecv();
	uint32_t IsBufferComplete();
	int SetCommandType();
	int ProcessHeader();
	int Post2KVdata();
	int FreeIov(int index);
	int Post2KVdataGets(aMsg *aMcmd);
	int Post2KVdataGetsPre();
	int Post2KVdataSet();
	int Post2KVdataQuit();
	int Post2KVdataTSQ(void *aM);
	int Release(); // close fd , release connection
	int CheckPendingCmdQ();
	class _adapt_::adaptSharedQ *PendingCmdQ;
	volatile uint32_t Busy;
	int cType;
	int cSockFd;
	volatile uint32_t Concurrent;
	volatile uint32_t dirtyFd;
	volatile uint32_t refcount;
	volatile uint32_t concPending;
	int aerrnum;
	int aerrtype;
	_adapt_::adaptThreadComm *aTcommptr;
	pthread_spinlock_t spinlock;
	volatile uint32_t reqCount;
	volatile uint32_t replyCount;
	volatile uint32_t totalsize;
	int emptyQ;
	volatile uint32_t conWatchSent;
	struct aEvent *commRead_wptr; //lib ev loop watch ptr
	struct event_base *loop;
	int dataPendingRead;
	volatile uint32_t writingflag;
	volatile uint32_t windexlast;
	aConnState state;
	aConnIov_t iovReq;
	aConnIov_t iovReply;
	aConnCmd_t cmd;
	std::bitset<16> mybits;
};

#endif /* CONN_H_ */
