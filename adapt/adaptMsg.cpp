/*
 * adaptMsg.cpp
 *
 *  Created on: Mar 25, 2011
 *      Author: awara
 */
#include <iostream>
#include <list>
#include <pthread.h>
#include <assert.h>
#include "adaptlog.h"
#include <string.h>
#include <cstdlib>

#include "adaptMsg.h"

namespace _adapt_ {

adaptMsg::adaptMsg(int type) {
	INFO(10, "MSG:(%p)Construct::Type:%d", this, type);
	this->type = type;
	this->pbuf = (char *) malloc(512);
	assert(this->pbuf);
	this->aMsgClear();

}

int adaptMsg::aMsgClear() {
	this->portParent = NULL;
	this->subtype = -1;
	this->parent = NULL;
	this->pbuf = (char *) memset(this->pbuf, 0, 512);
	return 0;
}

adaptMsg::~adaptMsg() {
	// TODO Auto-generated destructor stub
}

int adaptMsg::aMsgPrint() {
	DBUG(ALOG_DALL,
			"MSG:(%p)Type:%d Subtype:%d PortParent(%p)Parent(%p)", this, this->type, this->subtype, this->portParent, this->parent);
	return 0;
}

adaptMsg * adaptMsg::aMsgGet() {
	return (adaptMsg *) this;
}

void adaptMsg::aMsgSetportParent(void *ptr) {
	this->portParent = ptr;
}

void *adaptMsg::aMsgGetportParent() {
	return (this->portParent);
}

} /* namespace _adapt_ */
