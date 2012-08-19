/*
 * adaptThread.h
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */

#ifndef ADAPTTHREAD_H_
#define ADAPTTHREAD_H_

#include <pthread.h>

namespace _adapt_ {

class adaptThread {
public:
	adaptThread(int type, void *(*ThreadStartFunctionPtr)(void *));
	virtual ~adaptThread();
	int aThreadStart(void *parentThreadptr);

	void *(*ThreadStartFunctionPtr)(void *);

	adaptThread *freebusyGQ;
	adaptThread *Q1;
	pthread_t threadId;
	int type;
};

} /* namespace _adapt_ */
#endif /* ADAPTTHREAD_H_ */
