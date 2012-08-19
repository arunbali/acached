/*
 * adaptThreadComm.h
 *
 *  Created on: Apr 1, 2011
 *      Author: awara
 */

#ifndef ADAPTTHREADCOMM_H_
#define ADAPTTHREADCOMM_H_

#include "adaptThread.h"
#include "adaptThreadedSharedQ.h"

namespace _adapt_ {

class adaptThreadComm: public _adapt_::adaptThread {
public:
	adaptThreadComm(int type, void * (*ThreadStartFunctionPtr)(void *),void *GlobalThreadQ);
	virtual ~adaptThreadComm();
	//int aThreadCommStart();
	void aTCommPrint();

	int fds[2];

};

} /* namespace _adapt_ */
#endif /* ADAPTTHREADCOMM_H_ */
