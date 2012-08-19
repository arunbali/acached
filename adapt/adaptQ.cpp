/*
 * adaptQ.cpp
 *
 *  Created on: Apr 25, 2011
 *      Author: awara
 */

#include <iostream>
#include <list>
#include <pthread.h>

#include "adaptQ.h"
#include "adaptThreadSQ.h"
#include "adaptMsg.h"
#include "adaptlog.h"

namespace _adapt_ {

adaptQ::adaptQ(int type) {
	DBUG(ALOG_DALL, "Q:(%p)Construct::Type:%d", this, type);
	this->type = type;
	this->Q = new ::std::list<void *>;
}

adaptQ::~adaptQ() {
	// TODO Auto-generated destructor stub
}
int adaptQ::aQsetType(int type) {
	return (this->type = type);
}

int adaptQ::aQgetType() {
	return (this->type);
}

int adaptQ::aQput(void *ptr) {
	::std::list<void *> *Qptr = (::std::list<void *> *) this->Q;

	DBUG(ALOG_DALL, "[%lud]aQput Q (%p) ptr(%p)", pthread_self(), this->Q, ptr);
	Qptr->push_back(ptr);
	return 0;
}

void *adaptQ::aQget() {
	void *ptr = NULL;
	::std::list<void *> *Qptr = (::std::list<void *> *) this->Q;

	if (!Qptr->empty()) {
		DBUG(ALOG_DALL, "[%p]aQGET Q(%p) not empty", pthread_self(), Qptr);
		ptr = Qptr->front();
		Qptr->pop_front();
	}

	return ptr;
}

int adaptQ::aQremove(void *ptr) {
	::std::list<void *> *Qptr = (::std::list<void *> *) this->Q;

	DBUG(ALOG_TQ, "bef removeQ(%p)(%d) ", Qptr, Qptr->size());
	Qptr->remove(ptr);
	DBUG( ALOG_TQ, "after remove(%d)\n ", Qptr->size());

	return 0;
}

void adaptQ::aQprint(void *ptr, void (FunctionPtr)(void *)) {
	FunctionPtr(ptr);
	return;
}

void * adaptQ::aQFindPtr(int (FunctionPtr)(void *, int), int x) {

	void * ptr;
	::std::list<void *>::iterator i;
	::std::list<void *> *Qptr = (::std::list<void *> *) this->Q;

	//DBUG(ALOG_DALL, "Q:(%s)Type:%d FRONT(%p)",str, this->type,Qptr->front());

	for (i = Qptr->begin(); i != Qptr->end(); ++i) {
		ptr = *i;
		if (FunctionPtr(ptr, x)) {
			return ptr;
		}
	}

	return NULL;
}

} /* namespace _adapt_ */

