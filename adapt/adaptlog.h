/*
 * adaptlog.h
 *
 *  Created on: May 11, 2011
 *      Author: arun
 */
#ifndef ADAPTLOG_H_
#define ADAPTLOG_H_

#include <stdio.h>
#include <pthread.h>

#ifndef ADAPTLOG_MAIN

extern unsigned long int alogVal;
extern unsigned long int alogMask;
extern FILE *logFP;
extern pthread_mutex_t logmutex;

#endif

#ifdef ADAPTLOG_MAIN

unsigned long int alogVal;
unsigned long int alogMask;
FILE *logFP;
pthread_mutex_t logmutex;

#endif

#define	ALOG_INFO		(unsigned long int)(1 << 0 )
#define	ALOG_DALL		(unsigned long int)(1 << 1 )
#define	ALOG_MSG		(unsigned long int)(1 << 2 )
#define	ALOG_TMSG		(unsigned long int)(1 << 3 )
#define	ALOG_TCONN		(unsigned long int)(1 << 4 )
#define	ALOG_THASH		(unsigned long int)(1 << 5 )
#define	ALOG_TTOK		(unsigned long int)(1 << 6 )
#define	ALOG_TQ			(unsigned long int)(1 << 7 )
#define	ALOG_TSQ		(unsigned long int)(1 << 8 )
#define	ALOG_TTHR		(unsigned long int)(1 << 9 )
#define	ALOG_INTERNAL		(unsigned long int)(1 << 10 )
#define	ALOG_TCONC		(unsigned long int)(1 << 11 )
#define	ALOG_TLCK		(unsigned long int)(1 << 12 )
#define	ALOG_TERR		(unsigned long int)(1 << 13 )
#define	ALOG_COMM		(unsigned long int)(1 << 14 )
#define	ALOG_PROCMD		(unsigned long int)(1 << 15 )
#define	ALOG_KVDATA		(unsigned long int)(1 << 16 )
#define	ALOG_CLIENT		(unsigned long int)(1 << 17 )
#define	ALOG_ALERT		(unsigned long int)(1 << 18 )
#define	ALOG_MEM		(unsigned long int)(1 << 19 )
#define	ALOG_PARSE		(unsigned long int)(1 << 20 )

class adaptlog {
public:
	adaptlog(char *logFilename);
	virtual ~adaptlog();
	unsigned long int Init(unsigned long int mask);

};

#ifndef ALOG_H_
#define ALOG_H_

#define LOGOG_LEVEL_NONE	0
#define LOGOG_LEVEL_INTERNAL	8
#define LOGOG_LEVEL_ALERT	16
#define LOGOG_LEVEL_CRITICAL	24
#define LOGOG_LEVEL_ERROR	32
#define LOGOG_LEVEL_WARN	40
#define LOGOG_LEVEL_WARN1	48
#define LOGOG_LEVEL_WARN2	56
#define LOGOG_LEVEL_WARN3	64
#define LOGOG_LEVEL_INFO	72
#define LOGOG_LEVEL_DEBUG	80
#define LOGOG_LEVEL_ALL		88
//! [Level Constants]

#define LOGOG_LEVEL_TYPE		int

#ifndef LOGOG_LEVEL
#define LOGOG_LEVEL LOGOG_LEVEL_ALERT
#endif

#define LOGOG_GROUP 1
#define	LOGOG_CATEGORY 2

#define LOGOG_LEVEL_GROUP_CATEGORY_MESSAGE( flg, str,formatstring, ... ) \
{ \
	if ( (flg & alogVal) ) { \
		pthread_mutex_lock(&logmutex); \
		fprintf(logFP,"%s:%p:",str,pthread_self()); \
		fprintf(logFP,formatstring, ##__VA_ARGS__ ); \
		fprintf(logFP,"\n"); \
		fflush(logFP); \
		pthread_mutex_unlock(&logmutex); \
	} \
}

/** Calls LOGOG_LEVEL_GROUP_CATEGORY_MESSAGE with the current LOGOG_GROUP and
 * LOGOG_CATEGORY setting.
 */
#define LOGOG_LEVEL_MESSAGE( flg,  str,formatstring, ... ) \
	LOGOG_LEVEL_GROUP_CATEGORY_MESSAGE( flg, str,formatstring, ##__VA_ARGS__ )

/** Calls LOGOG_LEVEL_MESSAGE with the current LOGOG_LEVEL setting. */
#define LOGOG_MESSAGE( flg, str, formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( flg, str, formatstring, ##__VA_ARGS__ )

#if LOGOG_LEVEL >= LOGOG_LEVEL_DEBUG
/** Logs a message at the DEBUG reporting level. */
#define LOGOG_DEBUG( flg,... ) \
	LOGOG_LEVEL_MESSAGE( flg, "DEBUG", ##__VA_ARGS__ )
#else
#define LOGOG_DEBUG( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL >= LOGOG_LEVEL_INFO
/** Logs a message at the INFO reporting level. */
#define LOGOG_INFO( flg, ... ) \
	LOGOG_LEVEL_MESSAGE(flg,"INFO", ##__VA_ARGS__ )
#else
#define LOGOG_INFO( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL	>= LOGOG_LEVEL_ALERT
/** Logs a message at the ALERT reporting level. */
#define LOGOG_ALERT( flg, ... ) \
	LOGOG_LEVEL_MESSAGE( flg, "ALERT", ##__VA_ARGS__ )
#else
#define LOGOG_ALERT( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL	>= LOGOG_LEVEL_INTERNAL
/** Logs a message at the ALERT reporting level. */
#define LOGOG_INTERNAL( flg, ... ) \
	LOGOG_LEVEL_MESSAGE( flg, "INTERNAL", ##__VA_ARGS__ )
#else
#define LOGOG_INTERNAL( formatstring, ... ) {};
#endif

#ifndef LOGOG_USE_PREFIX
/* If you get compilation errors in this section, then define the flag LOGOG_USE_PREFIX during compilation, and these
 * shorthand logging macros won't exist -- you'll need to use the LOGOG_* equivalents above.
 */
/* We can't use DEBUG in Win32 unfortunately, so we use DBUG for shorthand here. */
//! [Shorthand]
/** \sa LOGOG_DEBUG */
#define DBUG(flg,...) LOGOG_DEBUG( flg, ##__VA_ARGS__ )
/** \sa LOGOG_INFO */
#define INFO(flg,...) LOGOG_INFO( flg, ##__VA_ARGS__)
/** \sa LOGOG_WARN3 */
#define WARN3(...) LOGOG_WARN3( __VA_ARGS__ )
/** \sa LOGOG_WARN2 */
#define WARN2(...) LOGOG_WARN2( __VA_ARGS__ )
/** \sa LOGOG_WARN1 */
#define WARN1(...) LOGOG_WARN1( __VA_ARGS__ )
/** \sa LOGOG_WARN */
#define WARN(...) LOGOG_WARN( __VA_ARGS__ )
/** \sa LOGOG_ERROR */
#define ERR(...) LOGOG_ERROR( __VA_ARGS__ )
/** \sa LOGOG_ALERT */
#define ALERT(...) LOGOG_ALERT( __VA_ARGS__ )
/** \sa LOGOG_CRITICAL */
#define CRITICAL(...) LOGOG_CRITICAL( __VA_ARGS__ )
/** \sa LOGOG_EMERGENCY */
#define INTERNAL(...) LOGOG_INTERNAL( __VA_ARGS__ )
//! [Shorthand]
#endif

#endif /* ALOG_H_ */

#endif /* ADAPTLOG_H_ */
