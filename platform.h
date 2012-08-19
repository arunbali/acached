/*
 * platform.h
 *
 *  Created on: May 10, 2011
 *      Author: arun
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <stdint.h>
#include <adaptlog.h>



int Thread_SetAffinity(unsigned short int CPUnum);
int Thread_PrintAffinity();
int	GetNumCores();
uint32_t atomicIsSet(volatile uint32_t *var);
uint32_t atomicSet(volatile uint32_t *var);
uint32_t atomicReset(volatile uint32_t *var);
uint32_t atomicIncrAddGet(volatile uint32_t  *var);
uint32_t atomicDecrSubGet(volatile uint32_t *var);
uint32_t atomicIncrGetAdd(volatile uint32_t  *var);
uint32_t atomicDecrGetSub(volatile uint32_t *var);
uint32_t atomicIncr2GetAdd(volatile uint32_t  *var);
uint32_t atomicDecr2GetSub(volatile uint32_t *var);


#endif /* PLATFORM_H_ */
