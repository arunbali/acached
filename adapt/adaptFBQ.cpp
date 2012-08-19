/*
 * adaptFBQ.cpp
 *
 *  Created on: Apr 25, 2011
 *      Author: awara
 */

#include "adaptQ.h"
#include "adaptFBQ.h"

namespace _adapt_ {

adaptFBQ::adaptFBQ(int FBQtype, int Qtype) {
	// TODO Auto-generated constructor stub
	this->FBQtype = FBQtype;
	this->FreeQ = new adaptQ(Qtype);
	this->BusyQ = new adaptQ(Qtype);
}

void *adaptFBQ::aFBQGetFree() {
	return (this->FreeQ->aQget());
}
int adaptFBQ::aFBQRemoveBusy(void *ptr) {
	return (this->BusyQ->aQremove(ptr));
}
int adaptFBQ::aFBQPutBusy(void *ptr) {
	return (this->BusyQ->aQput(ptr));
}
int adaptFBQ::aFBQPutFree(void *ptr) {
	return (this->FreeQ->aQput(ptr));
}
void * adaptFBQ::aFBQFindPtrBusy(int (FunctionPtr)(void *, int), int x) {

	return (this->BusyQ->aQFindPtr(FunctionPtr, x));
}

adaptFBQ::~adaptFBQ() {
	// TODO Auto-generated destructor stub
}

} /* namespace _adapt_ */
