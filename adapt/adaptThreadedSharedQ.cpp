/*
 * adaptThreadedSharedQ.cpp
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
#include "adaptThreadedSharedQ.h"

namespace _adapt_ {

void adaptLock(pthread_mutex_t *mutex);
void adaptUnLock(pthread_mutex_t *mutex);

adaptThreadedSharedQ::adaptThreadedSharedQ(int type, int threadtype,
		int numthreads, void * (*ThreadStartFunctionPtr)(void *),
		void *GlobalThreadQ) :adaptSharedQ(type) {

	int i = 0;
	adaptThreadSQ *Tsqptr;


	pthread_cond_init(&(this->WaitForTask_condQ), NULL);

	for (i = 0; i < numthreads; i++) {
		Tsqptr = new adaptThreadSQ(threadtype, i, ThreadStartFunctionPtr, this,GlobalThreadQ);
	}
}

adaptThreadedSharedQ::~adaptThreadedSharedQ() {
	// TODO Auto-generated destructor stub

}
// get msg and free from Q
void *adaptThreadedSharedQ::aTSQgetMsg() {
	void *ptr = NULL;
	::std::list<void *> *Qptr = (::std::list<void *> *) this->Q;

	DBUG(ALOG_TCONN,
			"(%p)Entering TSQgetmsg Q(%p) empty:%d front(%p)", pthread_self(), Qptr, Qptr->empty(), Qptr->front());
	do {

		ptr = NULL;

		adaptLock(&(this->mutexQ));

		if (Qptr->empty() == 1) {
			DBUG(ALOG_TCONN,"(%p)waiting for Msg front is(%p)", pthread_self());
			pthread_cond_wait(&(this->WaitForTask_condQ), &(this->mutexQ));
			adaptUnLock(&(this->mutexQ));
			DBUG(ALOG_DALL,"(%p)Q(%p)After cond wait front is(%p)", pthread_self(), Qptr, Qptr->front());
			//ptr = this->aSQget();
		} else {
			ptr = Qptr->front();
			DBUG(ALOG_TCONN, "(%p)got MSG(%p)", pthread_self(), ptr);
			Qptr->pop_front();
			adaptUnLock(&(this->mutexQ));

		}
		//DBUG(ALOG_DALL, "(%p) Check if got MSG(%p)" , pthread_self(),ptr);

	} while (ptr == NULL);
	return ptr;

}

// put at end
int adaptThreadedSharedQ::aTSQputMsg(void *ptr) {

	//DBUG(ALOG_DALL, "(%p):Q(%p)Put Msg(%p)" , pthread_self(),this->Q, ptr) ;
	//this->aSQput(ptr) ;
	//DBUG(ALOG_DALL, "(%p)Q(%p)TSQ putting asq (%p)",pthread_self(), this->Q, ptr) ;

	adaptLock(&(this->mutexQ));
	this->aQput(ptr);
	pthread_cond_signal(&(this->WaitForTask_condQ));
	adaptUnLock(&(this->mutexQ));

	return 0;
}

void adaptLock(pthread_mutex_t *mutex) {

//	DBUG(ALOG_DALL, "(%p)before lock (%p)" , pthread_self(),mutex);
	pthread_mutex_lock(mutex);
//	DBUG(ALOG_DALL, "(%p)after lock (%p)" , pthread_self(),mutex);

}

void adaptUnLock(pthread_mutex_t *mutex) {
//	DBUG(ALOG_DALL, "(%p)before unlock (%p)" , pthread_self(),mutex);
	pthread_mutex_unlock(mutex);
//	DBUG(ALOG_DALL, "(%p)after unlock (%p)" , pthread_self(),mutex);

}

} /* namespace _adapt_ */
