/*
 * adaptlog.cpp
 *
 *  Created on: May 11, 2011
 *      Author: arun
 */

#include "adaptlog.h"

adaptlog::adaptlog(char *logFilename) {

	if ((logFP = fopen(logFilename, "a+")) == NULL) {
		fprintf(stderr, "Setting logFP to stderr...failed(%s)\n", logFilename);
		logFP = stderr;
	}

	alogVal = 0x00000000;
	alogMask = 0xFFFFFFFF;

}

adaptlog::~adaptlog() {
	// TODO Auto-generated destructor stub
}

unsigned long int adaptlog::Init(unsigned long int mask) {
	alogVal = alogVal | mask;
	return alogVal;

}
