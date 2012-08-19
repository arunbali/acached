/*
 * adaptFBSharedQ.cpp
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */

#include "adaptFBSharedQ.h"
#include "adaptlog.h"

namespace _adapt_ {

adaptFBSharedQ::adaptFBSharedQ(int FBSQtype, int SQtype) {
	// TODO Auto-generated constructor stub
	this->FBSQtype = FBSQtype;
	this->FreeQ = new adaptSharedQ(SQtype);
	this->BusyQ = new adaptSharedQ(SQtype);
}

void *adaptFBSharedQ::aFBSQGetFree() {
	return (this->FreeQ->aSQget());
}
int adaptFBSharedQ::aFBSQRemoveBusy(void *ptr) {
	return (this->BusyQ->aSQremove(ptr));
}
int adaptFBSharedQ::aFBSQPutBusy(void *ptr) {
	return (this->BusyQ->aSQput(ptr));
}
int adaptFBSharedQ::aFBSQPutFree(void *ptr) {
	return (this->FreeQ->aSQput(ptr));
}

void * adaptFBSharedQ::aFBSQFindPtrBUSY(int (FunctionPtr)(void *, int), int x) {
	void *ptr = NULL;

	ptr = this->BusyQ->aSQFindPtr(FunctionPtr, x);
	//DBUG(ALOG_INTERNAL, "returning 222 %p ", ptr);
	return ptr;
}

adaptFBSharedQ::~adaptFBSharedQ() {
	// TODO Auto-generated destructor stub
}

} /* namespace _adapt_ */

