/*
 * ahashtable.cpp
 *
 *  Created on: Apr 30, 2011
 *      Author: awara
 */

#include "acached.h"

ahashtable::ahashtable(unsigned int hashsize, int hash_lock_ratio) {

	this->hashsize = hashsize;
	this->hash_lock_ratio = hash_lock_ratio;

	this->hashtable = (KVdata **) calloc(hashsize, sizeof(KVdata *));
	this->hashsize_power =(int) (floor(log((float) this->hashsize) / log(2.0)));
	this->hashmask = this->hashsize - 1;

	this->hash_lock_ratio_power = (int) (floor(log((float) this->hash_lock_ratio) / log(2.0)));
	this->hash_lock_count = (unsigned long) (1<< (this->hashsize_power - this->hash_lock_ratio_power));
	this->hash_lock_mask = this->hash_lock_count - 1;

	this->hashtablelock = (pthread_rwlock_t *) calloc(this->hash_lock_count,sizeof(pthread_rwlock_t));

	DBUG(ALOG_DALL,
			"HASH:INIT:hashtable:%p locktable:%p hashsize:%d lockcount:%d", this->hashtable, this->hashtablelock, this->hashsize, this->hash_lock_count);
}

ahashtable::~ahashtable() {
}

int ahashtable::set(uint32_t hashval, KVdata *KVdataptr) {

	//get the bucket start pointer
	KVdata *head, *prevhead, *tmphead, *tmp1, *tmp2;
	pthread_rwlock_t *bucketLock;
	uint32_t bucket;
	char *ptr, *ptr2;

	bucket = hashval & this->hashmask;
	head = tmphead = this->hashtable[bucket];
	bucketLock = &(this->hashtablelock[bucket & this->hash_lock_mask]);

	DBUG(ALOG_THASH,
			"hash:add:hash:%u bucket:%d bucketlock:%d bucketlockaddress:(%p)Key(%s)len(%d)head(%p)", hashval, bucket, (bucket&this->hash_lock_mask), bucketLock, (char *)&(KVdataptr->data), KVdataptr->keylen, head);
	DBUG(ALOG_THASH,
			"hash:set:Keylen(%d)Key(%s)hashval(%u)bucket(%u)bucket-head(%p)", KVdataptr->keylen, (char *)&(KVdataptr->data), hashval, bucket, head);

	// lock bucketLock
	pthread_rwlock_wrlock(bucketLock);

	if (tmphead) {

		do {
			ptr = (char *) &(KVdataptr->data);
			ptr2 = (char *) &(tmphead->data);
			if (!strncmp(ptr, ptr2, KVdataptr->keylen)) {
				ALERT(ALOG_THASH, "SET:DUPLICATE ENTRY IN HASH KEY:%s", ptr);

				tmp1 = KVdataptr;
				tmp2 = tmphead;
				tmp1->prev = tmp2->prev;
				tmp1->next = tmp2->next;

				if (tmp1->prev)
					(tmp1->prev)->next = tmp1;

				if (tmp1->next)
					(tmp1->next)->prev = tmp1;

				if (tmphead == head) {
					this->hashtable[bucket] = tmp1;
				}
				//* free malloc
				DBUG((ALOG_THASH|ALOG_MEM),
						"hash:set:FREEING:(%p) setting(%p)", tmp2, tmp1);
				free(tmp2);

				pthread_rwlock_unlock(bucketLock);
				return 0;
			}

			prevhead = tmphead;
			tmphead = tmphead->next;
		} while (tmphead != NULL);

		tmphead = prevhead;
		tmphead->next = KVdataptr;
		KVdataptr->prev = tmphead;
		KVdataptr->next = NULL;

	} else {
		this->hashtable[bucket] = KVdataptr;
		KVdataptr->next = NULL;
		KVdataptr->prev = NULL;
		DBUG(ALOG_THASH, "hash:set:HEAD setting(%p)", KVdataptr);
	}

	// unlock bucketLock
	pthread_rwlock_unlock(bucketLock);

	return 0;

}

int ahashtable::get(aMsgHdrIN_t *aMsgHdrINptr) {

	//get the bucket start pointer
	KVdata *head, *prevhead, *tmphead, *tmp1, *tmp2;
	pthread_rwlock_t *bucketLock;
	uint32_t bucket;
	char *ptr, *ptr2;
	uint32_t hashval;

	hashval = aMsgHdrINptr->hashval;
	bucket = hashval & this->hashmask;
	head = tmphead = this->hashtable[bucket];
	bucketLock = &(this->hashtablelock[bucket & this->hash_lock_mask]);

	INFO(ALOG_THASH,
			"hash:add:hash:%u bucket:%d bucketlock:%d bucketlockaddress:(%p)key(%s)head(%p)", hashval, bucket, (bucket&this->hash_lock_mask), bucketLock, aMsgHdrINptr->key, head);

	INFO(ALOG_THASH,
			"hash:get:Keylen(%d)Key(%s)hashval(%u)bucket(%u)bucket-head(%p)", aMsgHdrINptr->keylen, aMsgHdrINptr->key, aMsgHdrINptr->hashval, bucket, head);

	// lock bucketLock
	pthread_rwlock_rdlock(bucketLock);

	if (tmphead) {

		do {
			ptr = aMsgHdrINptr->key;
			ptr2 = (char *) &(tmphead->data);
			if (!strncmp(ptr, ptr2, aMsgHdrINptr->keylen)) {
				INFO(ALOG_THASH, "FOUND ENTRY IN HASH KEY:%s", ptr);
				aMsgHdrINptr->KVdataptr = tmphead;
				pthread_rwlock_unlock(bucketLock);
				return 0;

			}

			prevhead = tmphead;
			tmphead = tmphead->next;
		} while (tmphead != NULL);

	}

	// unlock bucketLock
	pthread_rwlock_unlock(bucketLock);

	return -1;

}

