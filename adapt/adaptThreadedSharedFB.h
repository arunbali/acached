/*
 * adaptThreadedSharedFB.h
 *
 *  Created on: Apr 25, 2011
 *      Author: awara
 */

#ifndef ADAPTTHREADEDSHAREDFB_H_
#define ADAPTTHREADEDSHAREDFB_H_

#include "adaptThreadedSharedQ.h"

namespace _adapt_ {

class adaptThreadedSharedFB {
public:
	adaptThreadedSharedFB(int FBtype, int type, int threadtype,
			int numthreadsFREE, void * (*ThreadStartFunctionPtrFREE)(void *),
			int numthreadsBUSY, void * (*ThreadStartFunctionPtrBUSY)(void *),
			void *GlobalThreadQ);
	virtual ~adaptThreadedSharedFB();

	int FBtype;
	adaptThreadedSharedQ *aTSQfree;
	adaptThreadedSharedQ *aTSQbusy;
};

}

#endif /* ADAPTTHREADEDSHAREDFB_H_ */
