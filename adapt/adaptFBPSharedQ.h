/*
 * adaptFBPSharedQ.h
 *
 *  Created on: Apr 25, 2011
 *      Author: awara
 */

#ifndef ADAPTFBPSHAREDQ_H_
#define ADAPTFBPSHAREDQ_H_

#include "adaptFBSharedQ.h"

namespace _adapt_ {

class adaptFBPSharedQ: public _adapt_::adaptFBSharedQ {
public:
	adaptFBPSharedQ(int FBSQtype, int SQtype) :
			adaptFBSharedQ(FBSQtype, SQtype) {
	}
	;
	virtual ~adaptFBPSharedQ() {
	}
	;
	//void *aFBSQGetFree();
	int aFBSPQRemoveBusy(void *ptr) {
		return 0;
	}
	;
	int aFBSPQPutBusy(void *ptr) {
		return 0;
	}
	;

};

} /* namespace _adapt_ */

#endif /* ADAPTFBPSHAREDQ_H_ */
