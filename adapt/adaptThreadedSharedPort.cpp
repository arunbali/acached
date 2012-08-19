/*
 * adaptThreadedSharedPort.cpp
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */

#include "adaptThreadedSharedPort.h"

namespace _adapt_ {

adaptThreadedSharedPort::adaptThreadedSharedPort(int porttype, int type,
		int threadtype, int numthreadsIN,
		void * (*ThreadStartFunctionPtrIN)(void *), int numthreadsBUSY,
		void * (*ThreadStartFunctionPtrBUSY)(void *), int numthreadsDONE,
		void * (*ThreadStartFunctionPtrDONE)(void *), void *GlobalThreadQ) {

	this->porttype = porttype;

	if (numthreadsIN) {
		this->aTSQin = new adaptThreadedSharedQ(type, threadtype, numthreadsIN,
				ThreadStartFunctionPtrIN, GlobalThreadQ);
	} else {
		this->aTSQin = NULL;
	}

	if (numthreadsBUSY) {
		this->aTSQbusy = new adaptThreadedSharedQ(type, threadtype,
				numthreadsBUSY, ThreadStartFunctionPtrBUSY, GlobalThreadQ);
	} else {
		this->aTSQbusy = NULL;
	}

	if (numthreadsDONE) {
		this->aTSQdone = new adaptThreadedSharedQ(type, threadtype,
				numthreadsDONE, ThreadStartFunctionPtrDONE, GlobalThreadQ);
	} else {
		this->aTSQdone = NULL;
	}

}

adaptThreadedSharedPort::~adaptThreadedSharedPort() {
	// TODO Auto-generated destructor stub
}

} /* namespace _adapt_ */
