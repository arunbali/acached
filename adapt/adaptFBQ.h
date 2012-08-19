/*
 * adaptFBQ.h
 *
 *  Created on: Apr 25, 2011
 *      Author: awara
 */

#ifndef ADAPTFBQ_H_
#define ADAPTFBQ_H_

namespace _adapt_ {

class adaptFBQ {
public:
	adaptFBQ(int FBQtype, int Qtype);
	virtual ~adaptFBQ();
	void *aFBQGetFree();
	int aFBQRemoveBusy(void *ptr);
	int aFBQPutBusy(void *ptr);
	int aFBQPutFree(void *ptr);
	void *aFBQFindPtrBusy(int (FunctionPtr)(void *, int), int x);

	int FBQtype;
	adaptQ *FreeQ;
	adaptQ *BusyQ;
};

} /* namespace _adapt_ */

#endif /* ADAPTFBQ_H_ */
