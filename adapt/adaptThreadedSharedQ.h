/*
 * adaptThreadedSharedQ.h
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */

#ifndef ADAPTTHREADEDSHAREDQ_H_
#define ADAPTTHREADEDSHAREDQ_H_

#include "adaptSharedQ.h"
#include "adaptThread.h"

namespace _adapt_ {

class adaptThreadedSharedQ: public _adapt_::adaptSharedQ {
public:
	adaptThreadedSharedQ(int type, int threadtype, int numthreads,
			void * (*ThreadStartFunctionPtr)(void *), void *GlobalThreadQ);
	virtual ~adaptThreadedSharedQ();

	void *aTSQgetMsg();
	int aTSQputMsg(void *ptr);

	pthread_cond_t WaitForTask_condQ;
};

} /* namespace _adapt_ */
#endif /* ADAPTTHREADEDSHAREDQ_H_ */
