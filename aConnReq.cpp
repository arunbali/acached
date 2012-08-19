#include "aConn.h"
#include <errno.h>
extern acacheSGA *SGAptr;
int closeConnection(int fd, struct aEvent *aEventptr);

int aConn::Post2KVdata() {

	switch (this->cmd.ctype) {

	case ACACHED_CMD_GETS:
		return (this->Post2KVdataGetsPre());

	case ACACHED_CMD_SET:
		return (this->Post2KVdataSet());

	case ACACHED_CMD_QUIT:
		return (this->Post2KVdataQuit());

	default:
		return -1;
	}

}

int aConn::Post2KVdataSet() {
	aMsg *aM;
	int hdr_index;

	aM = SGAptr->aMsgAllocate();
	DBUG((ALOG_TMSG|ALOG_ALERT),
			"CONN(%p):P2KVSet:MSG:Allocated(%p)key:%s", this, aM, this->cmd.key);

	aM->Init(KVDATA, this, this->cmd.ctype);
	aM->MsgHdrIN_Init(this->cmd.key, this->cmd.keylen, 0, NULL);

	aM->IN.KVdataSet.flag = this->cmd.flags;
	aM->IN.KVdataSet.exptime = this->cmd.exptime;
	aM->IN.KVdataSet.vallen = this->cmd.datalen;
	aM->HdrIN.cmdptr = ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex);

	// release the header here, body free by KVdata
	hdr_index = this->iovReq.iovIndexHdr;
	if (hdr_index > 0)
		ALERT(ALOG_TCONN, "CONN(%p):ALERT:Post2KVdataSet:index:%d desc:%d", this, hdr_index, ISSET_AC_IOVREQ_DESC(this,hdr_index,AC_IOVREQ_DESC_HDR));

	while (hdr_index >= 0) {
		DBUG((ALOG_TCONN|ALOG_ALERT),
				"CONN(%p)index:%d desc all:%u", this, hdr_index, this->iovReq.iovDesc[hdr_index].to_ulong());
		assert(
				(ISSET_AC_IOVREQ_DESC(this,hdr_index,AC_IOVREQ_DESC_HDR)|| ISSET_AC_IOVREQ_DESC(this,hdr_index,AC_IOVREQ_DESC_HDR_PART)));
		this->FreeIov(hdr_index);
		hdr_index--;
	}
	//this->iovReq.iovIndex++;

	this->Post2KVdataTSQ(aM);

	return 0;
}

int aConn::Post2KVdataGetsPre() {
	aMsg *aM;
	int hdr_index, ret;
	char *ptr;

	assert(this->iovReq.iovIndex ==0);

	aM = SGAptr->aMsgAllocate();
	DBUG((ALOG_TMSG|ALOG_ALERT),
			"CONN(%p):P2KVGetsPre:MSG:Allocated(%p)", this, aM, this->cmd.key);

	aM->Init(KVDATA, this, this->cmd.ctype);
	aM->HdrIN.cmdptr = ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex);
	//  fprintf(stderr, "cmdptr:%s\n",aM->HdrIN.cmdptr );
	this->Post2KVdataTSQ(aM);

	return 0;
}

int aConn::Post2KVdataGets(aMsg *aMcmd) {
	aMsg *aM;
	int hdr_index, ret;
	char *ptr;
	int KVdatathrindex;
	int bucket;
	char *bufptr = (char *) aMcmd->HdrIN.cmdptr;

	//ALERT(ALOG_TCONN, "COMM:conn(%p)P2KVGets:ptr:(%s)buffer:%s",this, ptr,bufptr);

	this->concPendingSet();

	ptr = strtok(bufptr, " ");
	ptr = strtok(NULL, " ");

	while (ptr != NULL) {
		aM = SGAptr->aMsgAllocate();
		ALERT((ALOG_TMSG|ALOG_TCONN), "CONN(%p):P2KVGets:MSG:Allocated(%p)key:%s", this, aM, ptr);

		aM->Init(KVDATA, this, this->cmd.ctype);
		aM->MsgHdrIN_Init(ptr, strlen(ptr), 0, NULL);
		this->reqCount++;
		ptr = strtok(NULL, " ");
		if (ptr == NULL && this->IsconcPending()) {
			this->concPendingReset();
		}
		//this->Post2KVdataTSQ(aM);
		bucket = aM->HdrIN.aMsgHdrIN.hashval & SGAptr->hashmask;
		if (bucket <= SGAptr->hashsize / SGAptr->numCores) {
			KVdatathrindex = 0;
		} else {
			KVdatathrindex = 1;
		}

		DBUG(ALOG_DALL, "KV index:%d", KVdatathrindex);
		//printf("posting to kvdata val:%d\n",__sync_val_compare_and_swap(&(this->Busy), 99,99) );
		SGAptr->aTSQGKVdata[KVdatathrindex]->aTSQputMsg(aM);

	}

	free(aMcmd->HdrIN.cmdptr);
	SGAptr->aMsgRelease(aMcmd);
	DBUG(ALOG_INTERNAL,
			"COMM:MSG:GETS:cmdptr:Released(%p)FreeQ size(%d)BusyQ size(%d)", aMcmd, SGAptr->aFBSQGaMsg->FreeQ->Q->size(), SGAptr->aFBSQGaMsg->BusyQ->Q->size());

	return 0;
}

int aConn::Post2KVdataQuit() {
	aMsg *aM;

	//close(this->cSockFd);
	//return 0;
	aM = SGAptr->aMsgAllocate();
	DBUG((ALOG_TMSG|ALOG_ALERT),
			"CONN(%p):P2KVQuit:MSG:Allocated(%p)", this, aM);

	aM->Init(KVDATA, this, this->cmd.ctype);
	//aM->MsgHdrIN_Init(this->cmd.key,this->cmd.keylen, 0,NULL);

	this->Post2KVdataTSQ(aM);

	return 0;
}

int aConn::Post2KVdataTSQ(void *ptr) {
	int KVdatathrindex;
	int bucket;
	aMsg *aM = (aMsg *) ptr;
	aMsg *aM1;

	//DBUG(ALOG_TCONN, "CONN(%p)BUSY val:%d BEFORE  QUEING MSG(%p)",this,__sync_val_compare_and_swap(&(this->Busy), 9999,9999),aM);

	if (__sync_bool_compare_and_swap(&(this->Busy), 0, 1)) {

		DBUG(ALOG_TCONN,
				"CONN(%p)Post2KVDATA:BUSY SET TO 1 val:%d ptr(%p) ", this, __sync_val_compare_and_swap(&(this->Busy), 99,99), &(this->Busy));
		// check if pending messages on aC
		aM1 = (aMsg *) this->PendingCmdQ->aSQget();
		if (aM1) {
			DBUG(ALOG_TCONN, "CONN(%p)got PendingCmdQ msg(%p)", this, aM1);
			if (ptr) {
				this->PendingCmdQ->aSQput(aM);
			}
			aM = aM1;
		}

		if (aM) {
			if (aM->aC->cmd.ctype == ACACHED_CMD_GETS) {
				this->Post2KVdataGets(aM);
			} else if (aM->aC->cmd.ctype == ACACHED_CMD_QUIT) {
				closeConnection(aM->aC->cSockFd, aM->aC->commRead_wptr);
				if (!__sync_bool_compare_and_swap(&(this->Busy), 1, 0)) {
					ALERT(
							ALOG_ALERT, "CONN(%p)CMD:QUIT NOT ABLE TO SWAP CONN BUSY val:%d", this, __sync_val_compare_and_swap(&(this->Busy), 99, 99));
				}
				SGAptr->aMsgRelease(aM);
			} else {

				// cmd will be SET
				bucket = aM->HdrIN.aMsgHdrIN.hashval & SGAptr->hashmask;
				if (bucket <= SGAptr->hashsize / SGAptr->numCores) {
					KVdatathrindex = 0;
				} else {
					KVdatathrindex = 1;
				}

				DBUG(ALOG_ALERT, "KV index:%d", KVdatathrindex);
				//printf("posting to kvdata val:%d\n",__sync_val_compare_and_swap(&(this->Busy), 99,99) );
				SGAptr->aTSQGKVdata[KVdatathrindex]->aTSQputMsg(aM);
			}

		} else {
			if (!__sync_bool_compare_and_swap(&(this->Busy), 1, 0)) {
				ALERT(
						ALOG_ALERT, "CONN(%p)NOT ABLE TO SWAP CONN BUSY val:%d", this, __sync_val_compare_and_swap(&(this->Busy), 99, 99));
			}

		}

	} else {
		DBUG(ALOG_TCONN,
				"CONN(%p)BUSY val:%d QUEING MSG(%p)CMD:%d", this, __sync_val_compare_and_swap(&(this->Busy), 99,99), aM, aM->aC->cmd.ctype);
		//queue to connection list
		this->PendingCmdQ->aSQput(aM);
		DBUG(ALOG_TCONN,
				"CONN(%p)BUSY val:%d AFTER QUEING MSG(%p)", this, __sync_val_compare_and_swap(&(this->Busy), 99,99), aM);

	}

	return 0;
}

