/*
 * threadKVdata.cpp
 *
 *  Created on: Apr 1, 2011
 *      Author: awara
 */
#include "aConn.h"
#include "platform.h"

extern acacheSGA *SGAptr;

void *thrKVdata(void *Tptr) {
	_adapt_::adaptThreadSQ *aTsqptr;
	void *ptr;
	aMsg *aM;
	int ret, ret2, i;
	int rcount = 0;
	aMsgType aMtype;
	int datasize;
	KVdata *KVdataptr;
	char *keyptr, *valptr;
	char tmpbuf2[8];
	int totalsize;
	int y;
	struct timespec ts;
	int size;
	int tot, count, newcount, start;

	if (sigignore(SIGPIPE) == -1) {
		perror("failed to ignore SIGPIPE; sigaction");
		exit(1);
	}

	aTsqptr = (_adapt_::adaptThreadSQ *) Tptr;
	Thread_SetAffinity(aTsqptr->threadIndex % 2);

	ALERT(
			ALOG_INFO, "KVDATA:Start:threadId(%p)sharedQ(%p)Type(%d)Q(%p)", aTsqptr->threadId, aTsqptr->aTSQptr, aTsqptr->type, aTsqptr->aTSQptr->Q);

	while (true) {

		aM = (aMsg *) aTsqptr->aTSQptr->aTSQgetMsg();
		DBUG((ALOG_TMSG|ALOG_KVDATA|ALOG_ALERT),
				"KVdata:GetMsg(%p)Type:%d", aM, aM->type);
		assert( aM->type == KVDATA);

		// DBUG(ALOG_DALL, "KVdata:fd:%d ctype:%d key:%s keylen:%d hashval:%u", aM->aC->cSockFd,aM->ctype,aM->key,aM->keylen,aM->hashval);

		// DBUG(ALOG_DALL, "KVdata:fd:%d ctype:%d key:%s ", aM->aC->cSockFd,aM->ctype,aM->key);

		switch (aM->ctype) {

		case ACACHED_CMD_SET:

			datasize = sizeof(KVdata) + aM->HdrIN.aMsgHdrIN.keylen + aM->IN.KVdataSet.vallen + 2;
			ptr = malloc(datasize);
			memset(ptr, 0x0, datasize);
			INFO(ALOG_MEM, "KVDATA:SET:MALLOC:%p", ptr);
			KVdataptr = (KVdata *) ptr;
			DBUG((ALOG_KVDATA|ALOG_ALERT),
					"KVDATA:SET:sizeofkvdata:%d keylen:%d datalen:%d +2 total:%d ptr(%p)data(%p)", sizeof(KVdata), aM->HdrIN.aMsgHdrIN.keylen, aM->IN.KVdataSet.vallen, datasize, ptr, &(KVdataptr->data));

			KVdataptr->keylen = aM->HdrIN.aMsgHdrIN.keylen;
			KVdataptr->vallen = aM->IN.KVdataSet.vallen;

			keyptr = (char *) &(KVdataptr->data);
			valptr = keyptr + KVdataptr->keylen;
			memcpy(keyptr, aM->HdrIN.aMsgHdrIN.key, KVdataptr->keylen);
			DBUG((ALOG_KVDATA|ALOG_MEM),
					"KVDATA:SET:FREE:Bodyptr:%p", aM->HdrIN.cmdptr);
			memcpy(valptr, aM->HdrIN.cmdptr, KVdataptr->vallen);
			// free cmdptr here
			free(aM->HdrIN.cmdptr);
			aM->HdrIN.cmdptr = NULL;

			//hexdump((char *)ptr,datasize,"KVDATA:SET:STORE",0);
			ret = SGAptr->ahashtableptr->set(aM->HdrIN.aMsgHdrIN.hashval,
					KVdataptr);
			//ALERT(ALOG_ALERT, "INDEX is %d\n",ACONN_IOVREPLYINDEX(aM->aC) );

			//pthread_spin_lock(&(aM->aC->spinlock));
			ret2 = atomicIncrGetAdd(&(ACONN_IOVREPLYINDEX(aM->aC)));
			strncpy(tmpbuf2, "STORED\r\n", 8);
			size = aM->aC->AddIOVReply(tmpbuf2, 8, true, ret2);
			ret2++;
			//y = __sync_val_compare_and_swap(&(aM->aC->windexlast), 0, 0) ;

			ret = aM->aC->WriteAllOnceIMM(0, 1);

			//pthread_spin_unlock(&(aM->aC->spinlock));

			if (ret != 8)
				ALERT(
						ALOG_ALERT, "KVDATA:SET:WriteData:writing STORED...fd:%d expecting return:8 got:%d errno:%d", aM->aC->cSockFd, ret, errno);

			if (!__sync_bool_compare_and_swap(&(aM->aC->Busy), 1, 0)) {
				ALERT(
						ALOG_ALERT, "KVDATA:SET:!!!:CONN(%p)NOT ABLE TO SWAP CONN BUSY val:%d", aM->aC, __sync_val_compare_and_swap(&(aM->aC->Busy), 99, 99));
			}

			aM->aC->windexlast = 0;
			aM->aC->totalsize = 0;
			aM->aC->iovReply.iovIndex = 0;
			aM->aC->replyCount = 0;
			aM->aC->writingflag = 0;

			if (aM->aC->PendingCmdQ->empty() == false) {
				aM->aC->CheckPendingCmdQ();
			}

			//sched_yield();

			rcount = aM->aC->Revoke();
			DBUG(ALOG_KVDATA,
					"KVDATA:CMD_SET:rcount:%d set add ret:%d ", rcount, ret);
			aMtype = SUCCESS;
			//post to client port
			aM->Init(aMtype, aM->aC, aM->ctype);
			//DBUG((ALOG_TMSG|ALOG_KVDATA),"KVdata:SET:Post Client(%p)type(%d)ctype(%d)",aM,aM->type, aM->ctype);
			//SGAptr->aTSQGClient->aTSQputMsg(aM) ; //post msg to Client SQ
			SGAptr->aMsgRelease(aM);
			DBUG(ALOG_INTERNAL,
					"CLIENT:MSG:Released(%p)FreeQ size(%d)BusyQ size(%d)", aM, SGAptr->aFBSQGaMsg->FreeQ->Q->size(), SGAptr->aFBSQGaMsg->BusyQ->Q->size());

			break;

		case ACACHED_CMD_GETS:

			ret = SGAptr->ahashtableptr->get(&(aM->HdrIN.aMsgHdrIN));
			if (ret < 0) {
				aM->aC->aerrtype = AERR_TYPE_APP_GETS;
				ALERT(
						(ALOG_TERR|ALOG_ALERT), "GET COMMAND KEY:%s len:%d NOT FOUND", aM->HdrIN.aMsgHdrIN.key, aM->HdrIN.aMsgHdrIN.keylen);
				snprintf(aM->OUT.ErrBuf, MAX_ERRBUF_LEN - 1,
						"CLIENT_ERROR NOT FOUND \r\n");
				rcount = aM->aC->Revoke();
				//aMtype = ERROR ;
				fprintf(stderr, "NOT FOUND!!!!\n");
				//atomicIncrAddGet(&(aM->aC->replyCount)) ;

			} else {
				KVdataptr = aM->HdrIN.aMsgHdrIN.KVdataptr;
				aM->aC->QReply(KVdataptr, aM->HdrIN.aMsgHdrIN.key);
				rcount = aM->aC->Revoke();
				//atomicIncrAddGet(&(aM->aC->replyCount)) ;

				DBUG(ALOG_TCONC, "KVDATA:conn revoke rcount:%d", rcount);
				DBUG(ALOG_KVDATA,
						"KVDATA:connection pending:%d reqcount:%d", aM->aC->IsconcPending(), aM->aC->reqCount);
			}
			atomicIncrAddGet(&(aM->aC->replyCount));

			if (!aM->aC->IsconcPending()) {

				// check if request == reply
				if ((ret = __sync_val_compare_and_swap(&(aM->aC->replyCount),
						aM->aC->reqCount, 0)) == aM->aC->reqCount) {
					DBUG(ALOG_KVDATA,
							"KVDATA:Replycount SWAP SUCESSFULL ret:%d", ret);
					aM->aC->reqCount = 0;

					ret2 = atomicIncrGetAdd(&(ACONN_IOVREPLYINDEX(aM->aC)));
					strncpy(tmpbuf2, "END\r\n", 5);
					size = aM->aC->AddIOVReply(tmpbuf2, 5, true, ret2);
					totalsize = __sync_add_and_fetch(&(aM->aC->totalsize),
							size);

					DBUG(ALOG_MEM, "KVDATA:aC(%p)index:%d", aM->aC, ret2);
					ret2++;
					// reset  here , as after recieving data, client will send next req
					ACONN_IOVREPLYINDEX(aM->aC) = 0;

					//ret = aM->aC->WriteAll(ret2);
					y = __sync_val_compare_and_swap(&(aM->aC->windexlast), 0,
							0);
					// z = __sync_val_compare_and_swap(&(aM->aC->writingflag), 110, 110) ;

					while (__sync_val_compare_and_swap(&(aM->aC->writingflag),
							111, 111)) {
						ALERT(
								ALOG_ALERT, "KVDATA:ALERT:WriteALL:Writing Flag is SET");
						sched_yield();
						ts.tv_sec = 0;
						ts.tv_nsec = 100000;
						nanosleep(&ts, NULL);
					}

					while (true) {
						for (i = y; i < ret2; i++) {
							if (ISSET_AC_IOVREPLY_DESC(aM->aC,i,AC_IOVREPLY_DESC_WRITTEN)== false ) {
								//newindex = i;
								ALERT((ALOG_TCONN|ALOG_TCONC), "KVDATA(%p)ALERT>>>not iov_written index:%d", aM->aC,i);
								break;
							}
						}
						if ( i == ret2) break;
						ALERT(ALOG_ALERT, "KVDATA:ALERT:Waiting for all desc writes");
						//sched_yield();
						ts.tv_sec = 1;
						ts.tv_nsec = 1000;
						nanosleep (&ts, NULL);

					}

					DBUG((ALOG_TCONN|ALOG_TCONC),
							"KVDATA:CONN(%p)WriteALLonce:before totalsize:%d windex:%d ", aM->aC, totalsize, y);
					aM->aC->writingflag = 0;

					//  int aConn::WriteIOVReply(int start, int count)

					count = ret2 - y;
					start = y;
					tot = 0;
					do {
						if (count > IOV_MAX) {
							newcount = IOV_MAX;
							count = count - IOV_MAX;
						} else {
							newcount = count;
						}
						ret = aM->aC->WriteAllOnce(start, newcount);
						tot = tot + ret;
						start = start + newcount;
						DBUG(ALOG_INFO,
								"KVDATA:WriteAllOnce:A:Expecting:%d Got:%d errno:%d", totalsize, tot, errno);

					} while (ret > 0 && newcount >= IOV_MAX);

					if (tot != totalsize) {
						ALERT(
								ALOG_INFO, "KVDATA:WriteAllOnce:TOTALSIZE FINAL:%d Expecting:%d Got:%d errno:%d", __sync_val_compare_and_swap(&(aM->aC->totalsize),0,0), totalsize, tot, errno);
					}
					//ret = aM->aC->WriteIOVReply(y, ret2-y,true);

					totalsize = __sync_val_compare_and_swap(
							&(aM->aC->totalsize), 0, 0);
					DBUG(ALOG_ALERT,
							"KVDATA:conn(%p)FINAL totalsize:%d", aM->aC, totalsize);

					aM->aC->windexlast = 0;
					aM->aC->totalsize = 0;
					aM->aC->iovReply.iovIndex = 0;
					aM->aC->replyCount = 0;
					aM->aC->writingflag = 0;

					for (i = 0; i < ret2; i++) {
						if (ISSET_AC_IOVREPLY_DESC(aM->aC,i,AC_IOVREPLY_DESC_MALLOC)) {
							DBUG(ALOG_MEM,"free:%p i:%d",aM->aC->iovReply.iovec[i].iov_base,i);
							//free(aM->aC->iovReply.iovec[i].iov_base);
							//free(ACONN_IOVREPLY_IOVECBASE2(aM->aC,i));
						}
						ACONN_IOVREPLY_IOVECBASE(aM->aC,i)= NULL;
						ACONN_IOVREPLY_IOVECBASE2(aM->aC,i)= NULL;
						ACONN_IOVREPLY_IOVECLEN(aM->aC,i)= 0;
						ACONN_IOVREPLY_BASEOFFSET(aM->aC,i)= 0;
						ACONN_IOVREPLY_ALLOCSIZE(aM->aC,i)= 0;
						//ACONN_IOVREQ_DESC(aM->aC,i).reset();
						ACONN_IOVREPLY_DESC(aM->aC,i).reset();

					}

					DBUG(ALOG_TCONC,
							"KVDATA:conn revoke bef END/r/n rcount:%d totalsize:%d ret:%d iovcount:%d "
							"reqCount:%d replyCount:%d", rcount, aM->aC->totalsize, ret, ret2, aM->aC->reqCount, aM->aC->replyCount);
					//ret = aM->aC->WriteData("END\r\n",5);
					//printf("return from write data:%d", ret);
					//	SGAptr->aSQGFreebusySQ->aSQprintall("Check freebusy rcount",PrintGFreebusySQ);

					atomicSet(&(aM->aC->conWatchSent));

					if (!__sync_bool_compare_and_swap(&(aM->aC->Busy), 1, 0)) {
						ALERT(
								ALOG_ALERT, "KVDATA:GETS:!!!:CONN(%p)NOT ABLE TO SWAP CONN BUSY val:%d ptr(%p)", aM->aC, __sync_val_compare_and_swap(&(aM->aC->Busy), 99, 99), &(aM->aC->Busy));
					}
					//aM->aC->Post2KVdataTSQ(NULL);
					if (aM->aC->PendingCmdQ->empty() == false) {
						aM->aC->CheckPendingCmdQ();
					}

				} else {
					DBUG(ALOG_KVDATA, "KVDATA:SWAP NOT SUCESSFULL ret:%d", ret);

				}

				//DBUG(ALOG_DALL, "KVDATA:CMD_GET:rcount:%d write returned:%d expecting:%d buf:%s", rcount,ret,);
			}

			aMtype = SUCCESS;
			//post to client port
			aM->Init(aMtype, aM->aC, aM->ctype);
			//INFO(ALOG_TMSG,"KVdata:GETS:Post Client(%p)type(%d)ctype(%d)",aM,aM->type, aM->ctype);
			//SGAptr->aTSQGClient->aTSQputMsg(aM) ; //post msg to Client SQ
			SGAptr->aMsgRelease(aM);
			DBUG(ALOG_INTERNAL,
					"CLIENT:MSG:Released(%p)FreeQ size(%d)BusyQ size(%d)", aM, SGAptr->aFBSQGaMsg->FreeQ->Q->size(), SGAptr->aFBSQGaMsg->BusyQ->Q->size());

			break;

		default:
			DBUG(ALOG_DALL, "ERROR::KVdata:::INVALID CMD:%d", aM->ctype);
			break;
		}

	}
	// thread never returns
	//pthread_exit((void*) t);
	return NULL;
}

