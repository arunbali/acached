/*
 * aMsg.h
 *
 *  Created on: Apr 26, 2011
 *      Author: awara
 */

#ifndef AMSG_H_
#define AMSG_H_

#include "acacheconst.h"

void PrintaMsg(void *ptr);

typedef struct {
	char key[MAX_KEY_LEN + 1];
	int keylen;
	uint32_t hashval;
	struct kvdata *KVdataptr;
} aMsgHdrIN_t;

typedef struct {
	int fd;
} aMsgDataArrived_IN;

typedef struct {
	uint32_t memsize;
} aMsgGetMem_IN;

typedef struct {
	void *memptr;
	uint32_t memsize;
} aMsgGetMem_OUT;

typedef struct {

} aMsgKVdata_IN_GET;

typedef struct {

	int flag;
	int exptime;
	int vallen;
} aMsgKVdata_IN_SET;

typedef struct {
	void *KVdataptr;
} aMsgKVdata_OUT_SET;

class aMsg {
public:
	aMsg();
	~aMsg();
	void Init(aMsgType type, class aConn *ptr, aCmdType ctype);
	void SETErrbuf(char *errstr);
	void MsgHdrIN_Init(char *key, int keylen, uint32_t hashval,
			void *KVdataptr);

	aMsgType type;
	aCmdType ctype; //command type
	class aConn *aC;
	int result;
	int errnum;

	struct {
		aMsgHdrIN_t aMsgHdrIN;
		void *cmdptr;
	} HdrIN;
	union _in_ {
		aMsgGetMem_IN GetMem;
		aMsgKVdata_IN_GET KVdataGet;
		aMsgKVdata_IN_SET KVdataSet;
		aMsgDataArrived_IN DataArrived;
	} IN;

	union _out_ {
		aMsgGetMem_OUT GetMem;
		aMsgKVdata_OUT_SET KVdataSet;
		char ErrBuf[MAX_ERRBUF_LEN];
	} OUT;

};

#endif /* AMSG_H_ */
