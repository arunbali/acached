/*
 * adaptThread.cpp
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */
#include <iostream>
#include <list>
#include <pthread.h>
#include <assert.h>
#include "adaptlog.h"
#include <cstdlib>
#include "adaptThread.h"

#define	THREAD_TYPE_KVDATA			205

namespace _adapt_ {

adaptThread::adaptThread(int type, void *(*ThreadStartFunctionPtr)(void *)) {

	DBUG(ALOG_DALL, "Thread:(%p)Construct::Type:%d", this, type);

	this->type = type;
	this->ThreadStartFunctionPtr = ThreadStartFunctionPtr;
	this->freebusyGQ = this;
	this->Q1 = this;

}

adaptThread::~adaptThread() {
	// TODO Auto-generated destructor stub
}
//parent class passess threadptr
int adaptThread::aThreadStart(void *parentThreadptr) {
	int rc;
	pthread_attr_t attr;
	struct sched_param param;
	int rc1, rc2;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if (this->type == THREAD_TYPE_KVDATA) {
		rc1 = pthread_attr_getschedparam(&attr, &param);
		//param.sched_priority = sched_get_priority_max(SCHED_RR);
		//pthread_attr_setschedparam(&attr, &param);
		rc2 = pthread_attr_setschedpolicy(&attr, SCHED_RR);
		//printf("rc1:%d rc2:%d\n", rc1, rc2);

	}

	rc = pthread_create(&(this->threadId), &attr, this->ThreadStartFunctionPtr,
			(void *) parentThreadptr);
	if (rc) {
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}
	return rc;

}

} /* namespace _adapt_ */
