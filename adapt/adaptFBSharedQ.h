/*
 * adaptFBSharedQ.h
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */

#ifndef ADAPTFBSHAREDQ_H_
#define ADAPTFBSHAREDQ_H_

#include "adaptSharedQ.h"

namespace _adapt_ {

class adaptFBSharedQ {
public:
	adaptFBSharedQ(int FBSQtype, int SQtype);
	virtual ~adaptFBSharedQ();
	void *aFBSQGetFree();
	void *aFBSQFindPtrBUSY(int (FunctionPtr)(void *, int), int x);
	int aFBSQRemoveBusy(void *ptr);
	int aFBSQPutBusy(void *ptr);
	int aFBSQPutFree(void *ptr);

	int FBSQtype;
	adaptSharedQ *FreeQ;
	adaptSharedQ *BusyQ;
};

} /* namespace _adapt_ */
#endif /* ADAPTFBSHAREDQ_H_ */
