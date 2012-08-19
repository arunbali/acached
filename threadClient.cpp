/*
 * threadclient.cpp
 *
 *  Created on: Apr 1, 2011
 *      Author: awara
 */

#include "acached.h"

extern acacheSGA *SGAptr;
void *thrClient(void *Tptr) {
	void *ptr;
	_adapt_::adaptThreadSQ *aTsqptr;
	aMsg *aM;
	int ctype;

	char tmpbuf[1024];
	MsgPipe_t AddFd;
	int ret;
	int rcount = 0;
	pthread_t mytid = pthread_self();

	/*
	 if (sigignore(SIGPIPE) == -1) {
	 perror("failed to ignore SIGPIPE; sigaction");
	 exit(1);
	 }
	 */
	aTsqptr = (_adapt_::adaptThreadSQ *) Tptr;

	DBUG(ALOG_DALL,
			"CLIENT:KVdata(%p)threadId(%p)sharedQ(%p)Type:%d", pthread_self(), aTsqptr->threadId, aTsqptr->aTSQptr, aTsqptr->type);

	while (true) {

		ptr = aTsqptr->aTSQptr->aTSQgetMsg();

		aM = (aMsg *) ptr;
		INFO(ALOG_TMSG, "CLIENT:GetMsg(%p)Type:%d", ptr, aM->type);

		assert( (aM->type == ERROR) || (aM->type = SUCCESS));
		DBUG((ALOG_TMSG|ALOG_CLIENT),
				"CLIENT:get msg:%p type:%d", ptr, aM->type);

		if (aM->type == ERROR) {
			switch (aM->aC->aerrtype) {
			case AERR_TYPE_APP_GETS:
				//aM->aC->ConnWatchPut();
				break;

			case AERR_TYPE_APP:

				DBUG((ALOG_CLIENT|ALOG_INFO),
						"CLIENT:AERR:APP:Sending:%s Key:%s", aM->OUT.ErrBuf, aM->HdrIN.aMsgHdrIN.key);
				aM->aC->WriteData(aM->OUT.ErrBuf, strlen(aM->OUT.ErrBuf));
				//fdatasync(aM->aC->cSockFd);
				//aM->aC->ConnWatchPut();

				break;

			case AERR_TYPE_SYS_COMM:

				if (aM->aC->aerrnum != AERR_TOKENIZER_SOCKET_HUNGUP) {
					DBUG((ALOG_CLIENT|ALOG_INFO),
							"CLIENT:AERR:SYS_COMM:Sending:CLIENT_ERROR %d", aM->aC->aerrnum);

					// try and send error , ok if fails

					aM->aC->WriteData("CLIENT_ERROR Data expected\r\n", 28);
				}
				//fdatasync(aM->aC->cSockFd);
				//free aconnptr
				/*
				 if ( (rcount = aM->aC->Revoke()) < 1 ) {
				 DBUG(ALOG_TCONC, "CLIENT:Releasing aConn(%p)rcount:%d",  aM->aC, rcount);
				 close(aM->aC->cSockFd);
				 SGAptr->aConnRelease(aM->aC);
				 aM->aC = NULL;
				 } else { // >= 1
				 DBUG(ALOG_TCONC, "CLIENT:Not Releasing aConn(%p) rcount:%d setting DirtyFD", rcount ,aM->aC);

				 aM->aC->dirtyFdSet();
				 if ( aM->aC->IsConcurrent() != 1 )
				 DBUG(ALOG_TCONC, "(%p):!!!!:CLIENT:Expecting Concurrent set...", mytid);
				 }
				 */
				//close(aM->aC->cSockFd);
				// aM->aC->ConnWatchRemove();
				//SGAptr->aConnRelease(aM->aC);
				//DBUG(ALOG_CLIENT, "CLIENT:Releasing aConn(%p)rcount:%d",  aM->aC, aM->aC->Revoke());
				break;

			default:
				DBUG(ALOG_DALL,
						"(%d):!!!!::CLIENT:::INVALID type:%d", mytid, aM->aC->aerrtype);
				break;
			}
		} else if (aM->type == SUCCESS) {

			/**
			 if ( aM->ctype != ACACHED_CMD_GETS) {
			 aM->aC->ConnWatchPut();
			 } else {
			 if (  atomicReset(&(aM->aC->conWatchSent))   ) {
			 aM->aC->ConnWatchPut();
			 }

			 }
			 **/

		}

		// for now release sMsg
		SGAptr->aMsgRelease(aM);
		DBUG(ALOG_INTERNAL,
				"CLIENT:MSG:Released(%p)FreeQ size(%d)BusyQ size(%d)", aM, SGAptr->aFBSQGaMsg->FreeQ->Q->size(), SGAptr->aFBSQGaMsg->BusyQ->Q->size());
		// sched_yield();

	}
	// thread never returns
	//pthread_exit((void*) t);
	return NULL;
}

