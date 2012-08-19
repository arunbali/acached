/*
 * hash.h
 *
 *  Created on: Apr 20, 2011
 *      Author: awara
 */

#ifndef HASH_H_
#define HASH_H_

#ifdef    __cplusplus
extern "C" {
#endif

uint32_t hash(const void *key, size_t length, const uint32_t initval);

#ifdef    __cplusplus
}
#endif

#endif /* HASH_H_ */
