#include "aConn.h"
#include <errno.h>

int aConn::WriteData(char *buf, int len) {
	int ret;
	int req;

	/*
	 if (this->IsdirtyFd()) {
	 this->aerrnum = AERR_WRITE_DIRTY_FD ;
	 this->aerrtype = AERR_TYPE_SYS_COMM;
	 return 0;
	 }
	 */
	ret = send(this->cSockFd, buf, len, MSG_NOSIGNAL);

	DBUG(ALOG_TCONN,
			"CONN(%p)Writedata:ret:%d ask:%d buf:%s", this, ret, len, buf);
	//this->dd(buf, ret, "writeData",0);
	if (ret <= 0) {
		if (ret < 0) {
			this->aerrnum = AERR_TOKENIZER_READ_ERR;
			this->aerrtype = AERR_TYPE_SYS_COMM;
		} else {
			this->aerrnum = AERR_TOKENIZER_SOCKET_HUNGUP;
			this->aerrtype = AERR_TYPE_SYS_COMM;
		}
		return -1;
	}

	return ret;
}

bool aConn::CheckIOVTotalWritten(int start, int count, int *iovEnd,
		uint32_t *totalptr, uint32_t writeret) {

	uint32_t tmptotal, total = 0;
	int i;
	int found = -1;
	int seek;
	char abc[256];

	tmptotal = 0;
	for (i = 0; i < count; i++) {
		total = total + ACONN_IOVREPLY_IOVECLEN(this,start+i);
		DBUG((ALOG_TCONN),"CONN(%p):DIFF:index:%d desc:%d ptr (%p) len %d count:%d",this,start+i,
				ISSET_AC_IOVREPLY_DESC(this,start+i,AC_IOVREPLY_DESC_WRITTEN),ACONN_IOVREPLY_IOVECBASE(this,start+i),ACONN_IOVREPLY_IOVECLEN(this,start+i),
				total);
		sprintf(abc,"Writing:start:%d count:%d writeret:%d index:%d len:%d",start,count,writeret, start+i, ACONN_IOVREPLY_IOVECLEN(this,start+i));
		hexdump((char *)ACONN_IOVREPLY_IOVECBASE(this,start+i),ACONN_IOVREPLY_IOVECLEN(this,start+i),abc,0);
		if ( total >= writeret && !tmptotal ) {
			found = i;
			tmptotal = total;
		}
	}
	*totalptr = total;
	if (tmptotal) {
		total = tmptotal;
		i = found;
	}

	if (total == writeret) {
		i++;
		*iovEnd = start + i;
		return true;

	} else if (total > writeret) {

		*iovEnd = start + i;

		total = total - ACONN_IOVREPLY_IOVECLEN(this,*iovEnd);
		DBUG(ALOG_TCONN,
				"CONN(%p):loop out i:%d ret:%d count:%d len:%d ", this, *iovEnd, writeret, total, ACONN_IOVREPLY_IOVECLEN(this,*iovEnd));

		seek = writeret - total;
		ACONN_IOVREPLY_IOVECBASE(this,*iovEnd)= (char *)(ACONN_IOVREPLY_IOVECBASE(this,*iovEnd)) + seek;
		ACONN_IOVREPLY_IOVECLEN(this,*iovEnd)= ACONN_IOVREPLY_IOVECLEN(this,*iovEnd) - seek;

	} else { // (total < writeret )
		INTERNAL(ALOG_INTERNAL, "CONN(%p):ALERT:Total:%d < write ret:%d", this,total,writeret );
		//assert(0);
	}

	return false;
}

int aConn::WriteIOVReply(int start, int count, bool final) {
	int32_t writeret;
	uint32_t total;
	int i, x, z;
	int seek, len;
	bool boolret;
	int iovEnd;
	struct msghdr mhdr;
	struct sockaddr sockaddr;

	memset(&mhdr, 0x0, sizeof(struct msghdr));
	memset(&sockaddr, 0x0, sizeof(struct sockaddr));

	sockaddr.sa_family = AF_UNSPEC;

	if (__sync_bool_compare_and_swap(&(this->writingflag), 0, 1)) {

		DBUG((ALOG_TCONN|ALOG_TCONC),"CONN(%p)WRITING FLAG SET start:%d iovcount:%d ", this, start, count);

		if (count > IOV_MAX)
			count = IOV_MAX;
		do {
			DBUG(ALOG_TCONC, "Writev1: start:%d count:%d", start, count);
			//writeret = writev(this->cSockFd, &(this->iovReply.iovec[start]),count);
			mhdr.msg_iov = &(this->iovReply.iovec[start]);
			mhdr.msg_iovlen = count;
			mhdr.msg_name = &sockaddr;
			mhdr.msg_namelen = sizeof(struct sockaddr);
			writeret = sendmsg(this->cSockFd, &mhdr, 0);
		} while (writeret == -1 && errno == EINTR);

		if (writeret <= 0) {
			if (__sync_bool_compare_and_swap(&(this->writingflag), 1,0) == false)
				ALERT(ALOG_TCONN, "CONN(%p):writing flag swap FAILED ", this);
			return writeret;
		}

		boolret = CheckIOVTotalWritten(start, count, &iovEnd, &total,(uint32_t) writeret);
		DBUG((ALOG_TCONN|ALOG_TCONC),
				"CONN(%p):check all boolret:%d total:%d actual:%d iovEnd:%d start:%d count:%d", this, boolret, total, writeret, iovEnd, start, count);

		if ((x = __sync_val_compare_and_swap(&(this->windexlast), start, iovEnd)) != start)
			ALERT((ALOG_TCONN|ALOG_TCONC), "CONN(%p):windex swap FAILED windex:%d newval:%d stored:%d", this, start, iovEnd, x);

		if (!__sync_bool_compare_and_swap(&(this->writingflag), 1, 0))
			ALERT(ALOG_TCONN, "CONN(%p):writing2 flag swap FAILED ", this);
		return writeret;

	} else {
		DBUG((ALOG_TCONN|ALOG_TCONC),"WRITING FLAG NOT AVAILABLE val:%d", __sync_bool_compare_and_swap(&(this->writingflag), 999, 999));
		sched_yield();

	}
	return -2;
}

uint32_t aConn::AddIOVReply(char *ptr, int len, bool do_malloc,
		uint32_t index) {
	void *tmpptr;

	if (do_malloc) {
		tmpptr = malloc(len + 1);
		memset(tmpptr, 0x0, len + 1);
		strncpy((char*) tmpptr, ptr, len);
		SET_AC_IOVREPLY_DESC(this, index, AC_IOVREPLY_DESC_MALLOC);
	} else {
		tmpptr = (void *) ptr;
	}
	ACONN_IOVREPLY_IOVECBASE(this,index)= tmpptr;
	ACONN_IOVREPLY_IOVECBASE2(this,index)= (char *)tmpptr;
	ACONN_IOVREPLY_IOVECLEN(this,index)= len;

	SET_AC_IOVREPLY_DESC(this, index, AC_IOVREPLY_DESC_WRITTEN);
	return len;

}

int aConn::QReply(KVdata *KVdataptr, char *key) {
	int newindex, index, len, ret, ret2;
	uint32_t size, totalsize;
	char bufptr[512];
	char *valptr;
	char *tmpptr;
	uint32_t x, y, z;
	uint32_t seek, i, count, readret;

	len = snprintf(bufptr, 512, "VALUE %s 0 %d 1\r\n", key, KVdataptr->vallen);
	valptr = (char *) &(KVdataptr->data) + KVdataptr->keylen;
	tmpptr = valptr + KVdataptr->vallen;
	memcpy(tmpptr, "\r\n", 2);

	index = atomicIncr2GetAdd(&(ACONN_IOVREPLYINDEX(this)));
	DBUG(ALOG_ALERT,"CONN(%p)Qreply:Key:%s len:%d index:%d", this, bufptr, len, index);

	//DBUG((ALOG_TCONN|ALOG_MEM), "CONN:(%p)Qreply index:%d  malloc(%p) key:%s", this,index,ptr,key);
	/**
	 if ( index > 1020 ) {
	 pthread_spin_lock(&(this->spinlock));
	 if ( !this->emptyQ ) {

	 ret = this->WriteAll(index);
	 ACONN_IOVREPLYINDEX(this)=0;
	 this->emptyQ = 1 ;
	 ALERT(ALOG_ALERT, "write all index:%d ret:%d expecting:%d", index, ret,this->totalsize);
	 this->totalsize = 0;

	 }
	 index = atomicIncr2GetAdd(&(ACONN_IOVREPLYINDEX(this)));
	 pthread_spin_unlock(&(this->spinlock));

	 }
	 **/

	size = this->AddIOVReply(bufptr, strlen(bufptr), true, index);
	index++;
	size = size
			+ this->AddIOVReply(valptr, KVdataptr->vallen + 2, false, index);

	DBUG((ALOG_TCONN|ALOG_TCONC),"CONN(%p)adding to totalsize  size:%d totalsize:%d ", this, size, __sync_val_compare_and_swap(&(this->totalsize), 0, 0));
	totalsize = __sync_add_and_fetch(&(this->totalsize), size);

	y = __sync_val_compare_and_swap(&(this->windexlast), 0, 0);

	newindex = index;
	for (i = y; i < (index); i++) {
		if (ISSET_AC_IOVREPLY_DESC(this,index,AC_IOVREPLY_DESC_WRITTEN)== false ) {
			newindex = i;
			ALERT(ALOG_TCONN, "CONN(%p)ALERT>>>not iov_written index:%d", this,i);
			break;
		}
	}
	if (newindex < index)
		index = newindex;

	//TODO: check if all buffers are written with valptr,etc previous to this index

	DBUG((ALOG_TCONN |ALOG_TCONC),
			"CONN(%p):Writing1:totalsize:%d index:%d windexlast:%d", this, totalsize, index, y);

	if ((totalsize > 126 * 1024 || (index - y) > 1024)) {
		ret = this->WriteIOVReply(y, index - y, false);
		DBUG((ALOG_TCONN|ALOG_TCONC),
				"CONN(%p)after IOVReplywrite ret is:%d start:%d count:%d errno:%d", this, ret, y, index-y, errno);

		if (ret > 0) {
			z = __sync_sub_and_fetch(&(this->totalsize), ret);
			DBUG((ALOG_TCONN|ALOG_TCONC),
					"CONN(%p)Reduced--> totalsize is:%d reducedby:%d", this, z, ret);
		}
	}

	return 0;
}

int aConn::WriteAllOnceIMM(int start, int iovCount) {

	struct msghdr mhdr;
	struct sockaddr sockaddr;
	int writeret;

	assert(iovCount ==1);
	memset(&mhdr, 0x0, sizeof(struct msghdr));
	memset(&sockaddr, 0x0, sizeof(struct sockaddr));
	do {

		//writeret = writev(this->cSockFd, &(this->iovReply.iovec[start]),iovCount);
		mhdr.msg_iov = &(this->iovReply.iovec[start]);
		mhdr.msg_iovlen = iovCount;
		mhdr.msg_name = &sockaddr;
		mhdr.msg_namelen = sizeof(struct sockaddr);

		writeret = sendmsg(this->cSockFd, &mhdr, 0);

	} while (writeret == -1 && errno == EINTR);

	return writeret;

}

int aConn::WriteAllOnce(int start, int iovCount) {
	int32_t ret2;
	int timeout = 10;
	int trycount = 0;
	int writeret;
	int TRY_COUNT_MAX = 5;
	int32_t totalcount = 0;
	bool boolret;
	uint32_t total;
	int iovEnd;
	int i, total1 = 0;
	struct msghdr mhdr;
	struct sockaddr sockaddr;

	memset(&mhdr, 0x0, sizeof(struct msghdr));
	memset(&sockaddr, 0x0, sizeof(struct sockaddr));

	do_write:

	do {
		DBUG(ALOG_TCONC,
				"CONN(%p)Writev2: start:%d count:%d", this, start, iovCount);
		for (i = start; i < (start + iovCount); i++) {
			total1 = total1 + ACONN_IOVREPLY_IOVECLEN(this,i);
			DBUG(ALOG_ALERT,"CONN(%p):writev21:index:%d desc:%d ptr(%p) len %d count:%d",this,i,
						ISSET_AC_IOVREPLY_DESC(this,i,AC_IOVREPLY_DESC_WRITTEN),ACONN_IOVREPLY_IOVECBASE(this,i),ACONN_IOVREPLY_IOVECLEN(this,i),
						total1);
		}

			//writeret = writev(this->cSockFd, &(this->iovReply.iovec[start]),iovCount);
		mhdr.msg_iov = &(this->iovReply.iovec[start]);
		mhdr.msg_iovlen = iovCount;
		mhdr.msg_name = &sockaddr;
		mhdr.msg_namelen = sizeof(struct sockaddr);

		writeret = sendmsg(this->cSockFd, &mhdr, 0);

	} while (writeret == -1 && errno == EINTR);

	if (writeret == 0) {
		return writeret;
	}

	DBUG(ALOG_TCONC,
			"Writev2: returns ret:%d errno:%d start:%d count:%d", writeret, errno, start, iovCount);

	trycount = 0;
	if (writeret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)
			&& (timeout > 0)) {
		do_select:
		ALERT(ALOG_TCONC, "Writev2:before select trycount:%d ", trycount);

		ret2 = wait_for_io_or_timeout(this->cSockFd, 0, 5);
		trycount++;
		if (ret2 == 0 and trycount < TRY_COUNT_MAX)
			goto do_select;
		if (ret2 < 0)
			return ret2;
		goto do_write;
	}

	if (writeret == -1)
		return writeret;

	boolret = CheckIOVTotalWritten(start, iovCount, &iovEnd, &total,
			(uint32_t) writeret);

	if (boolret == false) {
		totalcount = totalcount + writeret;
		start = iovEnd;
		iovCount = iovCount - (iovEnd - start);
		goto do_write;
	}

	return (totalcount + writeret);

}
