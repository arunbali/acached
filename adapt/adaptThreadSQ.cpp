/*
 * adaptThreadSQ.cpp
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */
#include <iostream>
#include <list>
#include <pthread.h>
#include <assert.h>
#include "adaptlog.h"
#include "adaptThreadSQ.h"


namespace _adapt_ {
adaptThreadSQ::adaptThreadSQ(int type, unsigned short int threadIndex,
		void * (*ThreadStartFunctionPtr)(void *), adaptThreadedSharedQ *aTSQptr,
		void *GlobalThreadQ) : adaptThread(type, ThreadStartFunctionPtr) {

	DBUG(ALOG_DALL, "Threadsq:(%p)Construct::Type:%d", this, type);
	//this->ThreadStartFunctionPtr = ThreadStartFunctionPtr ;
	this->aTSQptr = aTSQptr;
	this->threadIndex = threadIndex;

	if (GlobalThreadQ) {
		// single queue, not freebusy, threads always in busy
		// add to global thread shared Q
		((adaptSharedQ *) GlobalThreadQ)->aSQput(this);
	}
	//this->aThreadSQStart();
	this->aThreadStart(this);
}

adaptThreadSQ::~adaptThreadSQ() {
	// TODO Auto-generated destructor stub
}

void adaptThreadSQ::aTsqPrint() {
	adaptThreadSQ *ptr2 = this;

	DBUG(ALOG_DALL,
			"ThreadSQ:(%p)Type:%d Id(%p)SQ(%p)Q(%p)", ptr2, ptr2->type, ptr2->threadId, ptr2->aTSQptr, ptr2->aTSQptr->Q)

}

} /* namespace _adapt_ */
