/*
 * acacheSGA.cpp
 *
 *  Created on: Mar 31, 2011
 *      Author: awara
 */

#include "acached.h"
extern acacheSGA *SGAptr;
extern void *thrIN(void *Tptr);
extern void *thrComm(void *Tptr);
extern void *thrProcessCommand(void *Tptr);
extern void *thrKVdata(void *Tptr);
extern void *thrClient(void *Tptr);

void PrintGFreebusySQ(void *ptr) {
	_adapt_::adaptFBSharedQ *aFBSQptr = (_adapt_::adaptFBSharedQ *) ptr;

	switch (aFBSQptr->FBSQtype) {

	case FBSQ_TYPE_GLOBAL_MSGS:
		aFBSQptr->BusyQ->aSQprintall("MSGS-BUSYQ", PrintaMsg);
		aFBSQptr->FreeQ->aSQprintall("MSGS-FREEQ", PrintaMsg);
		break;

	case FBSQ_TYPE_GLOBAL_CONN:
		aFBSQptr->BusyQ->aSQprintall("CONN-BUSYQ", PrintaConn);
		aFBSQptr->FreeQ->aSQprintall("CONN-FREEQ", PrintaConn);
		break;

		//default :
	}

}

void PrintGthreads(void *ptr) {
	_adapt_::adaptThreadSQ *aThreadSQptr = (_adapt_::adaptThreadSQ *) ptr;

	INFO(ALOG_TTHR,
			"Ptr(%p)Type(%d)Id(%p)index(%u)aTSQptr(%p)\n", aThreadSQptr, aThreadSQptr->type, aThreadSQptr->threadId, aThreadSQptr->threadIndex, aThreadSQptr->aTSQptr);
	return;
}

acacheSGA::acacheSGA(int type) {
	void *ptr;
	int i = 0;
	int numTSQGKVdata;
	//_adapt_::adaptThreadedSharedPort	*P1, *P2 ;

	DBUG(ALOG_DALL,
			"(%p)acacheSGA:(%p)Construct::Type:%d", pthread_self(), this, type);
	this->type = type;

	this->hashsize = ACACHED_HASH_SIZE;
	this->hashsize_power =
			(int) (floor(log((float) this->hashsize) / log(2.0)));
	this->hashmask = this->hashsize - 1;

	this->hash_lock_ratio = ACACHED_HASH_LOCK_RATIO;
	this->hash_lock_ratio_power = (int) (floor(log((float) this->hash_lock_ratio) / log(2.0)));
	this->hash_lock_count = (unsigned long) (1 << (this->hashsize_power - this->hash_lock_ratio_power));
	this->hash_lock_mask = this->hash_lock_count - 1;

	this->numCores = GetNumCores();

	this->aSQGSharedQ = new _adapt_::adaptSharedQ(Q_TYPE_GLOBAL_SQ);
	this->aSQGFreebusySQ = new _adapt_::adaptSharedQ(Q_TYPE_GLOBAL_FBQ);
	this->aSQGThreadedSQ = new _adapt_::adaptSharedQ(Q_TYPE_GLOBAL_TSQ);

	this->aSQGthreads = new _adapt_::adaptSharedQ(Q_TYPE_GLOBAL_THREADS);
	this->aSQGSharedQ->aSQput(this->aSQGthreads);

	this->aSQGprocessCommandBusy = new _adapt_::adaptSharedQ(Q_TYPE_PROCESS_CMD_BUSY);
	this->aSQGSharedQ->aSQput(this->aSQGprocessCommandBusy);

	this->aFBSQGaMsg = new _adapt_::adaptFBSharedQ(FBSQ_TYPE_GLOBAL_MSGS,Q_TYPE_GLOBAL_MSGS);
	this->aSQGFreebusySQ->aSQput(this->aFBSQGaMsg);

	this->aFBSQGaConn = new _adapt_::adaptFBSharedQ(FBSQ_TYPE_GLOBAL_CONN,Q_TYPE_GLOBAL_CONN);
	this->aSQGFreebusySQ->aSQput(this->aFBSQGaConn);

	this->aFBSQGaLock = new _adapt_::adaptFBSharedQ(FBSQ_TYPE_GLOBAL_LOCK,Q_TYPE_GLOBAL_LOCK);
	this->aSQGFreebusySQ->aSQput(this->aFBSQGaLock);

	// allocate hash KVdata bucket Queues
	this->ahashtableptr = new ahashtable(this->hashsize,
			this->hash_lock_ratio_power);

	for (i = 0; i < INIT_COUNT_GLOBAL_MSG; i++) {
		ptr = new aMsg();
		this->aFBSQGaMsg->aFBSQPutFree(ptr);
	}
	INFO(ALOG_TMSG,
			"INIT:MSG:allocating:%d FreeQ(%p)size(%d)BusyQ(%p)size(%d)", INIT_COUNT_GLOBAL_MSG, this->aFBSQGaMsg->FreeQ->Q, this->aFBSQGaMsg->FreeQ->Q->size(), this->aFBSQGaMsg->BusyQ->Q, this->aFBSQGaMsg->BusyQ->Q->size());

	for (i = 0; i < INIT_COUNT_GLOBAL_CONN; i++) {
		ptr = new aConn(CONN_TYPE_NONE, -1);
		this->aFBSQGaConn->aFBSQPutFree(ptr);
	}
	INFO(ALOG_TCONN,
			"INIT:ACONN:allocating:%d FreeQ(%p)size(%d)BusyQ(%p)size(%d)", INIT_COUNT_GLOBAL_CONN, this->aFBSQGaConn->FreeQ->Q, this->aFBSQGaConn->FreeQ->Q->size(), this->aFBSQGaConn->BusyQ->Q, this->aFBSQGaConn->BusyQ->Q->size());

	/*
	 this->aTSQGprocessCommand = new _adapt_::adaptThreadedSharedQ(
	 Q_TYPE_PROCESS_CMD, THREAD_TYPE_PROCESS_CMD,INIT_COUNT_THREAD_PROCESS_CMD,
	 thrProcessCommand,this->aSQGthreads
	 );
	 this->aSQGThreadedSQ->aSQput(this->aTSQGprocessCommand);


	 this->aTSQGgetHash = new _adapt_::adaptThreadedSharedQ(
	 Q_TYPE_GET_HASH, THREAD_TYPE_GET_HASH,INIT_COUNT_THREAD_GET_HASH,
	 thrIN,this->aSQGthreads
	 );
	 this->aSQGThreadedSQ->aSQput(this->aTSQGgetHash);



	 this->aTSQGgetMem = new _adapt_::adaptThreadedSharedQ(
	 Q_TYPE_GET_MEM, THREAD_TYPE_GET_MEM,INIT_COUNT_THREAD_GET_MEM,
	 thrIN,this->aSQGthreads
	 );
	 this->aSQGThreadedSQ->aSQput(this->aTSQGgetMem);
	 */

	numTSQGKVdata = this->numCores;
//	numTSQGKVdata=1;

	this->aTSQGKVdata = (_adapt_::adaptThreadedSharedQ **) (malloc(
			sizeof(KVdata *) * numTSQGKVdata));

	for (i = 0; i < (INIT_COUNT_THREAD_KVDATA / numTSQGKVdata + 1); i++) {
		this->aTSQGKVdata[i] = new _adapt_::adaptThreadedSharedQ(
				Q_TYPE_KVDATA, THREAD_TYPE_KVDATA,INIT_COUNT_THREAD_KVDATA / numTSQGKVdata, thrKVdata,
				this->aSQGthreads);
		this->aSQGThreadedSQ->aSQput((this->aTSQGKVdata[i]));
		fprintf(stderr, "\nkvdata--> index %d ", i);
	}

	/**
	 this->aTSQGClient = new _adapt_::adaptThreadedSharedQ(
	 Q_TYPE_CLIENT, THREAD_TYPE_CLIENT,INIT_COUNT_THREAD_CLIENT,
	 thrClient,this->aSQGthreads
	 );
	 this->aSQGThreadedSQ->aSQput(this->aTSQGClient);
	 **/

	for (i = 0; i < ACACHED_THREAD_COMM_MAX; i++)
		this->aTcomm[i] = new _adapt_::adaptThreadComm(THREAD_TYPE_COMM,
				thrComm, this->aSQGthreads);

	ALERT(ALOG_INFO, "\nInitialization Done %s !!!!!\n", "hello");
	//this->aSQGthreads->aSQprintall("ALL THREADS", PrintGthreads);
	this->aSQGFreebusySQ->aSQprintall("Check freebusy", PrintGFreebusySQ);
}

acacheSGA::~acacheSGA() {
	// TODO Auto-generated destructor stub
}

void *acacheSGA::aSGAAllocate(class _adapt_::adaptFBSharedQ *aFBSQGptr) {
	void *ptr;

	DBUG((ALOG_DALL|ALOG_TSQ),
			"allocating freeQ(%p)size(%d)BusyQ(%p)size(%d)\n", (void *)aFBSQGptr->FreeQ->Q, aFBSQGptr->FreeQ->Q->size(), (void *)aFBSQGptr->BusyQ->Q, aFBSQGptr->BusyQ->Q->size());
	ptr = aFBSQGptr->aFBSQGetFree();
	aFBSQGptr->aFBSQPutBusy(ptr);
	return ptr;
}

void *acacheSGA::aSGARelease(class _adapt_::adaptFBSharedQ *aFBSQGptr,void *ptr) {

	DBUG((ALOG_DALL| ALOG_TSQ),
			"Releasing freeQ(%p)size(%d)BusyQ(%p)size(%d)\n", aFBSQGptr->FreeQ->Q, aFBSQGptr->FreeQ->Q->size(), aFBSQGptr->BusyQ->Q, aFBSQGptr->BusyQ->Q->size());

	aFBSQGptr->aFBSQRemoveBusy(ptr);
	aFBSQGptr->aFBSQPutFree(ptr);

	return NULL;
}

aConn *acacheSGA::aConnAllocate() {

	return (aConn *) (this->aSGAAllocate(this->aFBSQGaConn));
}

aConn *acacheSGA::aConnRelease(class aConn *aC) {

	return (aConn *) (this->aSGARelease(this->aFBSQGaConn, aC));

}

class aMsg *acacheSGA::aMsgAllocate() {
	void *ptr;
	int i;
abc:
	ptr = this->aSGAAllocate(this->aFBSQGaMsg);
	if (ptr == NULL) {
		printf("\n MSG ALLOCATE RETURNS NULLLL\n");
		for (i = 0; i < INIT_COUNT_GLOBAL_MSG; i++) {
			ptr = new aMsg();
			this->aFBSQGaMsg->aFBSQPutFree(ptr);
		}
		goto abc;
	}
	return (class aMsg *) ptr;
}

class aMsg *acacheSGA::aMsgRelease(class aMsg *aM) {

	return (class aMsg *) (this->aSGARelease(this->aFBSQGaMsg, aM));

}

int Post2KVdata(class aMsg * aM) {
	int KVdatathrindex;
	int bucket;

	bucket = aM->HdrIN.aMsgHdrIN.hashval & SGAptr->hashmask;
	if (bucket <= SGAptr->hashsize / SGAptr->numCores) {
		KVdatathrindex = 0;

	} else {
		KVdatathrindex = 1;

	}

	DBUG(ALOG_DALL, "KV index:%d", KVdatathrindex);
	SGAptr->aTSQGKVdata[KVdatathrindex]->aTSQputMsg(aM);

	return 0;
}

void acacheSGA::Print() {

	ALERT(ALOG_INFO, "SGA:Cores:%d Type:%d", this->numCores, this->type);
	ALERT(ALOG_INFO, "Hash:size:%d power:%d mask:%d Lock:ratio:%d power:%d count:%d mask:%d",
				this->hashsize, this->hashsize_power, this->hashmask, this->hash_lock_ratio, this->hash_lock_ratio_power, this->hash_lock_count, this->hash_lock_mask);
	ALERT(ALOG_INFO, "Allocated:Msg:%d Connection:%d Thread:Kvdata:%d TSQ:%d COMM:%d", INIT_COUNT_GLOBAL_MSG,
				INIT_COUNT_GLOBAL_CONN, INIT_COUNT_THREAD_KVDATA, INIT_COUNT_THREAD_CLIENT, ACACHED_THREAD_COMM_MAX);

}

