/*
 * adaptSharedQ.cpp
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */

#include <iostream>
#include <list>
#include <pthread.h>

#include "adaptlog.h"
#include "adaptSharedQ.h"
#include "adaptThreadSQ.h"
#include "adaptMsg.h"

namespace _adapt_ {

void adaptLock(pthread_mutex_t *mutex);
void adaptUnLock(pthread_mutex_t *mutex);

adaptSharedQ::adaptSharedQ(int type) :
		adaptQ(type) {
	DBUG(ALOG_DALL, "SharedQ:(%p)Construct::Type:%d", this, type);
	//  this->type = type;
	pthread_mutex_init(&(this->mutexQ), NULL);

}

adaptSharedQ::~adaptSharedQ() {
	// TODO Auto-generated destructor stub
}
int adaptSharedQ::aSQsetType(int type) {
	return (this->aQsetType(type));
}

int adaptSharedQ::aSQgetType() {
	return (this->aQgetType());
}

int adaptSharedQ::aSQput(void *ptr) {
//	::std::list<void *> *Qptr = (::std::list<void *> *)this->Q;

	DBUG(ALOG_DALL, "(%p)Q(%p)aSQput (%p)", pthread_self(), this->Q, ptr);
	adaptLock(&(this->mutexQ));
	this->aQput(ptr);
	adaptUnLock(&(this->mutexQ));

	return 0;
}

void *adaptSharedQ::aSQget() {
	void *ptr = NULL;

//	DBUG(ALOG_DALL, "(%p)Q(%p)getting asq",pthread_self(), this->Q ) ;

	adaptLock(&(this->mutexQ));
	ptr = this->aQget();
	adaptUnLock(&(this->mutexQ));
	return ptr;
}

int adaptSharedQ::aSQremove(void *ptr) {

	adaptLock(&(this->mutexQ));
	this->aQremove(ptr);
	adaptUnLock(&(this->mutexQ));
	return 0;
}

void adaptSharedQ::aSQprint(void *ptr, void (FunctionPtr)(void *)) {

	return (this->aQprint(ptr, FunctionPtr));
}

void adaptSharedQ::aSQprintall(char *str, void (FunctionPtr)(void *)) {

	void *ptr;
	::std::list<void *>::iterator i;
	::std::list<void *> *Qptr = (::std::list<void *> *) this->Q;

	adaptLock(&(this->mutexQ));

	INFO( (ALOG_DALL|ALOG_TSQ),
			"\nSHAREDQ:(%s)Type:%d size(%d)\n", str, this->type, Qptr->size());

	for (i = Qptr->begin(); i != Qptr->end(); ++i) {
		ptr = *i;
		this->aSQprint(ptr, FunctionPtr);
	}

	adaptUnLock(&(this->mutexQ));
	return;
}

void * adaptSharedQ::aSQFindPtr(int (FunctionPtr)(void *, int), int x) {
	void *ptr;

	adaptLock(&(this->mutexQ));
	ptr = this->aQFindPtr(FunctionPtr, x);
	adaptUnLock(&(this->mutexQ));
	//DBUG(ALOG_INTERNAL,"returning ptr %p", ptr);
	return ptr;
}

bool adaptSharedQ::empty() {
	bool ret;

	adaptLock(&(this->mutexQ));
	ret = this->Q->empty();
	adaptUnLock(&(this->mutexQ));

	return ret;
}

} /* namespace _adapt_ */
