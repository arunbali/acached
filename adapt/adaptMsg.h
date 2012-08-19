/*
 * adaptMsg.h
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */

#ifndef ADAPTMSG_H_
#define ADAPTMSG_H_

namespace _adapt_ {

class adaptMsg {
public:
	adaptMsg(int type);
	virtual ~adaptMsg();
	int aMsgPrint();
	adaptMsg *aMsgGet();
	void aMsgSetportParent(void *ptr);
	void *aMsgGetportParent();
	int aMsgClear();

	char *pbuf;
	int type;
	int subtype;
private:

	void *portParent;
	adaptMsg *parent; // if parent != null , then child msg

};

} /* namespace _adapt_ */
#endif /* ADAPTMSG_H_ */
