/*
 * adaptThreadedSharedPort.h
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */

#ifndef ADAPTTHREADEDSHAREDPORT_H_
#define ADAPTTHREADEDSHAREDPORT_H_

#include "adaptThreadedSharedQ.h"
namespace _adapt_ {

class adaptThreadedSharedPort {
public:
	adaptThreadedSharedPort(int porttype, int type, int threadtype,
			int numthreadsIN, void * (*ThreadStartFunctionPtrIN)(void *),
			int numthreadsBUSY, void * (*ThreadStartFunctionPtrBUSY)(void *),
			int numthreadsDONE, void * (*ThreadStartFunctionPtrDONE)(void *),
			void *GlobalThreadQ);
	virtual ~adaptThreadedSharedPort();

	int porttype;
	adaptThreadedSharedQ *aTSQin;
	adaptThreadedSharedQ *aTSQbusy;
	adaptThreadedSharedQ *aTSQdone;
};

} /* namespace _adapt_ */
#endif /* ADAPTTHREADEDSHAREDPORT_H_ */
