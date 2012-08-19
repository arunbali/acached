/*
 * ahash.h
 *
 *  Created on: Apr 30, 2011
 *      Author: awara
 */

#ifndef AHASH_H_
#define AHASH_H_
#include "aMsg.h"

typedef struct kvdata {

	struct kvdata *next;
	struct kvdata *prev;
	uint32_t keylen;
	uint32_t vallen;
	union {
		int x;
		int y;
	} data;

} KVdata;

class ahashtable {
public:
	ahashtable(unsigned int hashsize, int hash_lock_ratio);
	virtual ~ahashtable();
	int set(uint32_t hashval, KVdata *KVdataptr);
	int get(aMsgHdrIN_t *aMsgHdrINptr);

	uint32_t hashsize;
	uint32_t hashsize_power;
	uint32_t hashmask;
	uint32_t hash_lock_ratio;
	uint32_t hash_lock_ratio_power;
	uint32_t hash_lock_count;
	uint32_t hash_lock_mask;
	KVdata ** hashtable;
	pthread_rwlock_t *hashtablelock;

};

#endif /* AHASH_H_ */
