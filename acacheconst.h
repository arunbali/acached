/*
 * acacheconst.h
 *
 *  Created on: Apr 27, 2011
 *      Author: awara
 */

#ifndef ACACHECONST_H_
#define ACACHECONST_H_

#define ENDIAN_LITTLE   			1
#define	INIT_COUNT_GLOBAL_MSG		8000
#define	INIT_COUNT_GLOBAL_CONN		500
#define ACONN_CMD_WAIT_INIT			1024*32 // inital cmd buffer to read header
//#define ACONN_IOV_SIZE_INIT			IOV_MAX*4 // IOV_MAX is usually 1024
#define ACONN_IOV_REQ_SIZE_INIT		32
#define ACONN_IOV_REPLY_SIZE_INIT		IOV_MAX*12

/*
 #define	INIT_COUNT_THREAD_PROCESS_CMD	4
 #define	INIT_COUNT_THREAD_GET_HASH		3
 #define	INIT_COUNT_THREAD_GET_MEM		3
 #define	INIT_COUNT_THREAD_KVDATA		4
 #define	INIT_COUNT_THREAD_CLIENT		3
 */
#define	INIT_COUNT_THREAD_PROCESS_CMD	1
#define	INIT_COUNT_THREAD_GET_HASH		1
#define	INIT_COUNT_THREAD_GET_MEM		1
#define	INIT_COUNT_THREAD_KVDATA		2
#define	INIT_COUNT_THREAD_CLIENT		2
#define	ACACHED_THREAD_COMM_MAX			2 // comm threads  for select

#define	MAX_RBUFFER_SIZE		1024*4

#define	MAX_KEY_LEN				255
#define MAX_ERRBUF_LEN			80

#define		Q_TYPE_Q_START		100
#define		Q_TYPE_SQ_START		200
#define		Q_TYPE_FBSQ_START	300
#define		Q_TYPE_TSQ_START	400

#define	Q_TYPE_PROCESS_CMD			Q_TYPE_TSQ_START + 1
#define	Q_TYPE_GET_HASH				Q_TYPE_TSQ_START + 2
#define	Q_TYPE_GET_MEM				Q_TYPE_TSQ_START + 3
#define	Q_TYPE_KVDATA				Q_TYPE_TSQ_START + 4
#define	Q_TYPE_CLIENT				Q_TYPE_TSQ_START + 5

#define	Q_TYPE_PROCESS_CMD_BUSY		Q_TYPE_SQ_START + 1
#define Q_TYPE_GLOBAL_THREADS		Q_TYPE_SQ_START + 2
#define Q_TYPE_GLOBAL_SQ			Q_TYPE_SQ_START + 3
#define Q_TYPE_GLOBAL_TSQ			Q_TYPE_SQ_START + 4
#define Q_TYPE_GLOBAL_FBQ			Q_TYPE_SQ_START + 5

#define	Q_TYPE_GLOBAL_MSGS			Q_TYPE_FBSQ_START + 1
#define	Q_TYPE_GLOBAL_CONN			Q_TYPE_FBSQ_START + 2
#define	Q_TYPE_GLOBAL_LOCK			Q_TYPE_FBSQ_START + 3

#define	FBSQ_TYPE_START				1000

#define FBSQ_TYPE_GLOBAL_MSGS		FBSQ_TYPE_START + 1
#define FBSQ_TYPE_GLOBAL_CONN		FBSQ_TYPE_START + 2
#define FBSQ_TYPE_GLOBAL_LOCK		FBSQ_TYPE_START + 3

#define	THREAD_TYPE_PROCESS_CMD		201
#define	THREAD_TYPE_GET_HASH		203
#define	THREAD_TYPE_GET_MEM			204
#define	THREAD_TYPE_KVDATA			205
#define	THREAD_TYPE_CLIENT			206
#define	THREAD_TYPE_COMM			220

#define	CONN_TYPE_NONE				401
#define	CONN_TYPE_TCP				402

enum aMsgType {

	PROCESS_COMMAND = 1001, KVDATA, ERROR, SUCCESS, NOP
};

enum aConnState {

	cmd_wait = 2001, hdr_wait, body_wait, cmd_done
};

enum aConnIovDesc {

	hdr = 2001, hdr_partial, body, body_partial, init, iov_written
};

enum aCmdType {

	ACACHED_CMD_GET = 1001,
	ACACHED_CMD_GETS,
	ACACHED_CMD_SET,
	ACACHED_CMD_ADD,
	ACACHED_CMD_REPLACE,
	ACACHED_CMD_APPEND,
	ACACHED_CMD_PREPEND,
	ACACHED_CMD_QUIT,
	ACACHED_CMD_VERSION
};

/** Maximum length of a key. */
#define KEY_MAX_LENGTH 250

#define		AERR_TYPE_APP						101
#define		AERR_TYPE_SYS						102
#define		AERR_TYPE_SYS_COMM					103
#define		AERR_TYPE_APP_GETS					104

#define		AERR_START	5000

enum AERRNUM_t {

	AERR_TOKENIZER_READ_ERR = AERR_START,
	AERR_TOKENIZER_SOCKET_HUNGUP,
	AERR_TOKENIZER_PARSE,
	AERR_TOKENIZER_CMD_INVALID,
	AERR_TOKENIZER_EAGAIN,
	AERR_READ_DIRTY_FD,
	AERR_WRITE_DIRTY_FD,
	AERR_TOKENIZER_CMD_INVALID_HDR,
	AERR_TOKENIZER_CMD_INVALID_DATALEN,
	AERR_TOKENIZER_TOKEN_INVALID
};

#ifndef AERRNUM_STR__
extern char *AERRNUM_STR;
#endif
#ifdef AERRNUM_STR__
char *AERRNUM_STR[] = {
	"Token Read Error",
	"Socket Hung up",
	"Parse Error",
	"Invalid Command",
	"TOKENIZER EAGAIN",
	"Dirty File Descriptor(Read)",
	"Dirty File Descriptor(Write)",
	"Invalid Command Header",
	"Invalid data length in header"
	"Invalid Token"
};

#endif

#define	GetErrNumStr(aerrnum) AERRNUM_STR[aerrnum - AERR_START] ;

#define hashsize(n) ((unsigned long int)1<<(n))
#define hashmask(n) (hashsize(n)-1)

#define	ACACHED_HASH_SIZE 	64*1024
//lock ratio power defines the lock spread
// val of 6 with hashsize 64K will have  1k locks
// val of 7 with hashsize 64k will have  512 locks
// 8 .. 256 locks

#define	ACACHED_HASH_LOCK_RATIO		128

#endif /* ACACHECONST_H_ */
