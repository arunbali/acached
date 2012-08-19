/*
 * adaptSharedQ.h
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */

#ifndef ADAPTSHAREDQ_H_
#define ADAPTSHAREDQ_H_

#include <pthread.h>
#include "adaptQ.h"

namespace _adapt_ {

class adaptSharedQ: public _adapt_::adaptQ {
public:
	adaptSharedQ(int type);
	virtual ~adaptSharedQ();

	int aSQgetType();
	int aSQsetType(int type);
	void *aSQget();
	int aSQput(void *ptr);
	int aSQremove(void *ptr);
	void aSQprint(void *str, void (FunctionPtr)(void *));
	void aSQprintall(char *str, void (FunctionPtr)(void *));
	void *aSQFindPtr(int (FunctionPtr)(void *, int), int x);
	bool empty();

	pthread_mutex_t mutexQ;

};

} /* namespace _adapt_ */
#endif /* ADAPTSHAREDQ_H_ */
