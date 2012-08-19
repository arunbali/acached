/*
 * adaptThreadedSharedFB.cpp
 *
 *  Created on: Apr 25, 2011
 *      Author: awara
 */

#include "adaptThreadedSharedFB.h"

namespace _adapt_ {

adaptThreadedSharedFB::adaptThreadedSharedFB(int FBtype, int type,
		int threadtype, int numthreadsFREE,
		void * (*ThreadStartFunctionPtrFREE)(void *), int numthreadsBUSY,
		void * (*ThreadStartFunctionPtrBUSY)(void *), void *GlobalThreadQ) {

	this->FBtype = FBtype;
	this->aTSQfree = new adaptThreadedSharedQ(type, threadtype, numthreadsFREE,
			ThreadStartFunctionPtrFREE, GlobalThreadQ);
	this->aTSQbusy = new adaptThreadedSharedQ(type, threadtype, numthreadsBUSY,
			ThreadStartFunctionPtrBUSY, GlobalThreadQ);

}

adaptThreadedSharedFB::~adaptThreadedSharedFB() {
	// TODO Auto-generated destructor stub
}

} /* namespace _adapt_ */

