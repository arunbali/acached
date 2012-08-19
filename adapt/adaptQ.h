/*
 * adaptQ.h
 *
 *  Created on: Apr 25, 2011
 *      Author: awara
 */

#ifndef ADAPTQ_H_
#define ADAPTQ_H_

#include <list>

namespace _adapt_ {

class adaptQ {
public:
	adaptQ(int type);
	virtual ~adaptQ();

	int aQgetType();
	int aQsetType(int type);
	void *aQget();
	int aQput(void *ptr);
	int aQremove(void *ptr);

	void aQprint(void *str, void (FunctionPtr)(void *));
	void aQprintall(char *str, void (FunctionPtr)(void *));
	void *aQFindPtr(int (FunctionPtr)(void *, int), int x);

	std::list<void *> *Q;
	int type;
};

} /* namespace _adapt_ */

#endif /* ADAPTQ_H_ */
