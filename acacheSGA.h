/*
 * acacheSGA.h
 *
 *  Created on: Mar 31, 2011
 *      Author: awara
 */

#ifndef ACACHESGA_H_
#define ACACHESGA_H_

#include "acached.h"

void PrintGthreads(void *ptr);
void PrintGFreebusySQ(void *ptr);
int Post2KVdata(class aMsg *aM);

class acacheSGA {
public:
	acacheSGA(int type);
	virtual ~acacheSGA();

	void *aSGAAllocate(class _adapt_::adaptFBSharedQ *aFBSQGptr);
	void *aSGARelease(class _adapt_::adaptFBSharedQ *aFBSQGptr, void *ptr);
	class aConn *aConnAllocate();
	class aConn *aConnRelease(class aConn *aC);
	class aMsg *aMsgAllocate();
	class aMsg *aMsgRelease(class aMsg *aMsgPtr);
	void Print();

	int type;
	int numCores;
	int hashsize;
	int hashsize_power;
	int hashmask;
	int hash_lock_ratio;
	int hash_lock_ratio_power;
	int hash_lock_count;
	int hash_lock_mask;

	class ahashtable *ahashtableptr;
	_adapt_::adaptSharedQ *aSQGSharedQ;
	_adapt_::adaptSharedQ *aSQGFreebusySQ;
	_adapt_::adaptSharedQ *aSQGThreadedSQ;
	_adapt_::adaptSharedQ *aSQGthreads;
	_adapt_::adaptSharedQ *aSQGprocessCommandBusy;


	_adapt_::adaptFBSharedQ *aFBSQGaMsg;
	_adapt_::adaptFBSharedQ *aFBSQGaConn;
	_adapt_::adaptFBSharedQ *aFBSQGaLock;

	//_adapt_::adaptFBSharedQ		*aFBSQGaClient;

	_adapt_::adaptThreadedSharedQ *aTSQGprocessCommand;
	_adapt_::adaptThreadedSharedQ *aTSQGgetHash;
	_adapt_::adaptThreadedSharedQ *aTSQGgetMem;
	_adapt_::adaptThreadedSharedQ *aTSQGClient;

	_adapt_::adaptThreadedSharedQ **aTSQGKVdata;

	_adapt_::adaptThreadComm *aTcomm[ACACHED_THREAD_COMM_MAX + 1];

	struct settings {
		size_t maxbytes;
		int maxconns;
		int port;
		int udpport;
		char *inter;
		int verbose;
//	        rel_time_t oldest_live; /* ignore existing items older than this */
		int evict_to_free;
		char *socketpath; /* path to unix socket if using local socket */
		int access; /* access mask (a la chmod) for unix domain socket */
		double factor; /* chunk size growth factor */
		int chunk_size;
		int num_threads; /* number of worker (without dispatcher) libevent threads to run */
		int num_threads_per_udp; /* number of worker threads serving each udp socket */
		char prefix_delimiter; /* character that marks a key prefix (for stats) */
		int detail_enabled; /* nonzero if we're collecting detailed stats */
		int reqs_per_event; /* Maximum number of io to process on each
		 io-event. */
		bool use_cas;
//	        enum protocol binding_protocol;
		int backlog;
		int item_size_max; /* Maximum item size, and upper end for slabs */
		bool sasl; /* SASL on/off */
		bool maxconns_fast; /* Whether or not to early close connections */
		bool slab_reassign; /* Whether or not slab reassignment is allowed */
		bool slab_automove; /* Whether or not to automatically move slabs */
		int hashpower_init; /* Starting hash poweaSQGthreadsr level */
	};
};

#endif /* ACACHESGA_H_ */
