/*
 * aMsg.cpp
 *
 *  Created on: Apr 26, 2011
 *      Author: awara
 */

#include <stdio.h>
#include <fcntl.h>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "aMsg.h"

extern "C" uint32_t hash(const void *key, size_t length,const uint32_t initval);

aMsg::aMsg() {
	// TODO Auto-generated constructor stub

}

aMsg::~aMsg() {
	// TODO Auto-generated destructor stub
}

void PrintaMsg(void *ptr) {
	aConn *aM = (aConn *) ptr;

	//printf("(%p)",aM);
	return;

}

void aMsg::Init(aMsgType type, aConn *ptr, aCmdType ctype) {
	aMsgType tmptype = type;
	aCmdType tmpctype = ctype;
	aConn *tmpaC = ptr;

	memset(this, 0x0, sizeof(aMsg));
	this->type = (aMsgType) tmptype;
	this->ctype = tmpctype;
	this->aC = tmpaC;
	return;

}

void aMsg::SETErrbuf(char *errstr) {
	snprintf(this->OUT.ErrBuf, MAX_ERRBUF_LEN, errstr);
	return;

}

void aMsg::MsgHdrIN_Init(char *key, int keylen, uint32_t hashval,void *KVdataptr) {

	this->HdrIN.aMsgHdrIN.keylen = keylen;
	strncpy(this->HdrIN.aMsgHdrIN.key, key, keylen);
	*(this->HdrIN.aMsgHdrIN.key + keylen) = 0x0;
	if (!hashval) {
		this->HdrIN.aMsgHdrIN.hashval = hash(this->HdrIN.aMsgHdrIN.key, keylen,0);
		//("\nhashval:(%u)\n",this->HdrIN.aMsgHdrIN.hashval );
	}

	this->HdrIN.aMsgHdrIN.KVdataptr = NULL;
	return;
}

