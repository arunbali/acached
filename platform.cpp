/*
 * platform.cpp
 *
 *  Created on: May 10, 2011
 *      Author: arun
 */

#include "platform.h"

int Thread_SetAffinity(unsigned short int CPUnum) {
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(1, &mask);

	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
		perror("pthread_setaffinity_np");
		return -1;
	}
	printf("\nsetting affinity to %u", CPUnum);
	return CPUnum;

}

int Thread_PrintAffinity() {
	cpu_set_t cpuset;
	int s, j;

	CPU_ZERO(&cpuset);
	/* Check the actual affinity mask assigned to the thread */

	s = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0)
		return -1;

	INFO(ALOG_INFO, "Set returned by pthread_getaffinity_np() contained:");
	for (j = 0; j < CPU_SETSIZE; j++)
		if (CPU_ISSET(j, &cpuset))
			INFO(ALOG_INFO, "    CPU %d", j);

	return 0;
}

int GetNumCores() {

	return (get_nprocs());
}

/**
 * The “val” version returns the contents of *ptr before the operation
 */

uint32_t atomicIsSet(volatile uint32_t *var) {

	return __sync_val_compare_and_swap(var, 1, 1);

}

/**
 * var == 0 then return 0 value becomes 1
 * var == 1 then return 1 value remain 1
 */

uint32_t atomicSet(volatile uint32_t *var) {

	return __sync_val_compare_and_swap(var, 0, 1);

}

/**
 * if var == 0 then return=0 and value remains 0
 * if var == 1 then return=1 and value changes to 0
 */
uint32_t atomicReset(volatile uint32_t *var) {

	return __sync_val_compare_and_swap(var, 1, 0);

}

uint32_t atomicIncrAddGet(volatile uint32_t *var) {
	return __sync_add_and_fetch(var, 1);
}

uint32_t atomicDecrSubGet(volatile uint32_t *var) {
	return __sync_sub_and_fetch(var, 1);

}

uint32_t atomicIncr2GetAdd(volatile uint32_t *var) {
	return __sync_fetch_and_add(var, 2);
}

uint32_t atomicDecr2GetSub(volatile uint32_t *var) {
	return __sync_fetch_and_sub(var, 2);

}

uint32_t atomicIncrGetAdd(volatile uint32_t *var) {
	return __sync_fetch_and_add(var, 1);
}

uint32_t atomicDecrGetSub(volatile uint32_t *var) {
	return __sync_fetch_and_sub(var, 1);

}
