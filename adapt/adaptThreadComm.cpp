/*
 * adaptThreadComm.cpp
 *
 *  Created on: Apr 1, 2011
 *      Author: awara
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <list>
#include <pthread.h>
#include <assert.h>
#include "adaptlog.h"
#include "adaptThreadComm.h"

namespace _adapt_ {

adaptThreadComm::adaptThreadComm(int type,
		void * (*ThreadStartFunctionPtr)(void *), void *GlobalThreadQ) :
		adaptThread(type, ThreadStartFunctionPtr) {
	DBUG(ALOG_DALL, "Threadsq:(%p)Construct::Type:%d", this, type);
//	this->ThreadStartFunctionPtr = ThreadStartFunctionPtr ;
	pipe2(this->fds, O_NONBLOCK);

	if (GlobalThreadQ) {
		// single queue, not freebusy, threads always in busy
		// add to global thread shared Q
		((adaptSharedQ *) GlobalThreadQ)->aSQput(this);

	}
	//this->aThreadCommStart();
	this->aThreadStart(this);
}

adaptThreadComm::~adaptThreadComm() {
	// TODO Auto-generated destructor stub
}

void adaptThreadComm::aTCommPrint() {
	adaptThreadComm *ptr2 = this;

	DBUG(ALOG_DALL,
			"ThreadSQ:(%p)Type:%d Id(%p)FDS:(%d)(%d)", ptr2, ptr2->type, ptr2->threadId, ptr2->fds[0], ptr2->fds[1])

}

} /* namespace _adapt_ */
