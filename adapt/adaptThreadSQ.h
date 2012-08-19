/*
 * adaptThreadSQ.h
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */

#ifndef ADAPTTHREADSQ_H_
#define ADAPTTHREADSQ_H_

#include "adaptThread.h"
#include "adaptThreadedSharedQ.h"

namespace _adapt_ {

class adaptThreadSQ: public _adapt_::adaptThread {
public:
	adaptThreadSQ(int type, unsigned short int threadIndex,
			void * (*ThreadStartFunctionPtr)(void *),
			adaptThreadedSharedQ *aTSQptr, void *GlobalThreadQ);
	virtual ~adaptThreadSQ();
	void aTsqPrint();

//	void * (*ThreadStartFunctionPtr)(void *);
	adaptThreadedSharedQ *aTSQptr;
	unsigned short int threadIndex;
};

} /* namespace _adapt_ */
#endif /* ADAPTTHREADSQ_H_ */
