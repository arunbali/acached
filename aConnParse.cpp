#include "aConn.h"
#include <errno.h>

typedef struct token_s {
	char *value;
	size_t length;
} token_t;

#define COMMAND_TOKEN 0
#define SUBCOMMAND_TOKEN 1
#define KEY_TOKEN 1

#define MAX_TOKENS 8

/*
 * Tokenize the command string by replacing whitespace with '\0' and update
 * the token array tokens with pointer to start of each token and length.
 * Returns total number of tokens.  The last valid token is the terminal
 * token (value points to the first unprocessed character of the string and
 * length zero).
 *
 * Usage example:
 *
 *  while(tokenize_command(command, ncommand, tokens, max_tokens) > 0) {
 *      for(int ix = 0; tokens[ix].length != 0; ix++) {
 *          ...
 *      }
 *      ncommand = tokens[ix].value - command;
 *      command  = tokens[ix].value;
 *   }
 */
static size_t tokenize_command(char *command, token_t *tokens,
		const size_t max_tokens) {
	char *s, *e;
	size_t ntokens = 0;
	size_t len = strlen(command);
	unsigned int i = 0;

	assert(command != NULL && tokens != NULL && max_tokens > 1);

	s = e = command;
	for (i = 0; i < len; i++) {
		if (*e == ' ') {
			if (s != e) {
				tokens[ntokens].value = s;
				tokens[ntokens].length = e - s;
				ntokens++;
				*e = '\0';
				if (ntokens == max_tokens - 1) {
					e++;
					s = e; /* so we don't add an extra token */
					break;
				}
			}
			s = e + 1;
		}
		e++;
	}

	if (s != e) {
		tokens[ntokens].value = s;
		tokens[ntokens].length = e - s;
		ntokens++;
	}

	/*
	 * If we scanned the whole string, the terminal value pointer is null,
	 * otherwise it is the first unprocessed character.
	 */
	tokens[ntokens].value = *e == '\0' ? NULL : e;
	tokens[ntokens].length = 0;
	ntokens++;

	return ntokens;
}

int aConn::ProcessDataRecv() {
	int ret, len;
	int srcIndex;
	char *ptr, *ptr2;
	uint32_t bufferComplete;
	uint32_t dataPendingRead;
	char tmpbuf[8192];

	switch (this->state) {

	case cmd_wait:
		//assert(this->iovReq.iovIndex == 0);

		ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex)= ACONN_CMD_WAIT_INIT;
		ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex) = malloc(ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex));
		ptr = (char *)ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex);
		//len = ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex);

		ret=read(this->cSockFd,ptr,ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex));
		DBUG(ALOG_PARSE,"CONN(%p)read1:ret:%d len:%d", this,ret, ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex));
		if ( ret <= 0 ) {
			this->SetError();
			return ret;
		}

		//if ( ret > 0) {
		sprintf(tmpbuf,"Read:fd:%d",this->cSockFd);
		hexdump(ptr, ret, tmpbuf,1);

		ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex) += ret;
//		ptr = ptr + ret;
//		len -= ret ;

		ret = this->SetCommandType();
		DBUG((ALOG_TCONN|ALOG_TCONC), "CONN(%p)CMD type:%d fd:%d iovIndex:%d",this,ret,this->cSockFd,this->iovReq.iovIndex);
		if (ret < 0 ) return ret;

		bufferComplete = this->IsBufferComplete();

		if ( bufferComplete) {

			SET_AC_IOVREQ_DESC(this,this->iovReq.iovIndex,AC_IOVREQ_DESC_HDR);
			this->iovReq.iovIndexHdr = this->iovReq.iovIndex;

			if ( this->cmd.ctype == ACACHED_CMD_SET ) {
				this->state = body_wait;
				if (this->ProcessHeader() == false )
				return -1;
			} else {
				if ( this->cmd.ctype == ACACHED_CMD_VERSION)
				this->WriteData("VERSION 1.2alpha\r\n",18);
				this->state = cmd_done;
			}

			DBUG(ALOG_PARSE, "CONN(%p)buffer complete:%d", this,bufferComplete);
			if ( bufferComplete < ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex)) {
				this->dataPendingRead = 1;
				ACONN_IOVREQ_BASEOFFSET(this,this->iovReq.iovIndex) = bufferComplete;
			}
			return bufferComplete;

		} else {
			//TODO add check if buffer is not complete then command can be get multiple
			// otherwise send error on this read if read == ACONN_IOVREQ_IOVECLEN

			SET_AC_IOVREQ_DESC(this,this->iovReq.iovIndex,AC_IOVREQ_DESC_HDR_PART);

			this->state = hdr_wait;
			assert(this->state!= hdr_wait);
			assert(("Got partial header in cmd_wait", 0));

		}

		return (ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex) );

		break;

		case body_wait:
		if (this->cmd.datalen > 0) {
			dataPendingRead = 0;
			if ( ISSET_AC_IOVREQ_DESC(this,this->iovReq.iovIndex,AC_IOVREQ_DESC_BODY_PART) ) {

				ptr = (char *)ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex) + ACONN_IOVREQ_BASEOFFSET(this,this->iovReq.iovIndex);
				len = (this->cmd.datalen + 2) - ACONN_IOVREQ_BASEOFFSET(this,this->iovReq.iovIndex);

			} else {

				if (this->dataPendingRead) {
					dataPendingRead = ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex) - ACONN_IOVREQ_BASEOFFSET(this,this->iovReq.iovIndex);
					DBUG(ALOG_TCONN, "CONN(%p)datapending:%d iovecLen:%d baseoffset:%d ", this,dataPendingRead,
							ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex),ACONN_IOVREQ_BASEOFFSET(this,this->iovReq.iovIndex));
					srcIndex = this->iovReq.iovIndex;
				} else {
					dataPendingRead = 0;
				}

				this->iovReq.iovIndex++;
				ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex) = this->cmd.datalen + 2;
				ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex) = malloc(ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex));
				ptr = (char *)ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex);
				len = ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex);
			}
			DBUG(ALOG_PARSE, "CONN(%p)datapending:%d len %d ", this,dataPendingRead,len);
			if ( this->dataPendingRead ) {
				if ( dataPendingRead >= len ) {
					//fprintf(stderr, "base(%p)offset(%d)\n",ACONN_IOVREQ_IOVECLEN(this,srcIndex),ACONN_IOVREQ_BASEOFFSET(this,srcIndex),ptr2,len,ptr2);

					ptr2 = (ACONN_IOVREQ_BASEOFFSET(this,srcIndex)+ (char *)ACONN_IOVREQ_IOVECBASE(this,srcIndex));
					//fprintf(stderr, "ptr(%p)ptr2(%p)len(%d)ptr2str(%p)\n",ptr,ptr2,len,ptr2);
					memcpy(ptr,ptr2,len);
					ret = len;
					len = 0;

				} else {
					ptr2 = (ACONN_IOVREQ_BASEOFFSET(this,srcIndex)+ (char *)ACONN_IOVREQ_IOVECBASE(this,srcIndex));
					//fprintf(stderr, "ptr(%p)ptr2(%p)len(%d)ptr2str(%p)\n",ptr,ptr2,len,ptr2);
					memcpy(ptr,ptr2,dataPendingRead);
					len -= dataPendingRead;
					ret = dataPendingRead;
					ptr += dataPendingRead;
				}

				if ( dataPendingRead > ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex) ) {
					this->dataPendingRead = 1;
					ACONN_IOVREQ_BASEOFFSET(this,srcIndex) = ret;
				} else {
					ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex) += ret;
					ACONN_IOVREQ_BASEOFFSET(this,srcIndex)=0;
					this->dataPendingRead =0;
				}
			}

			if ( len ) {
				ret=read(this->cSockFd,ptr,len);
				DBUG(ALOG_PARSE,"CONN(%p)read2:ret:%d len:%d", this,ret, len);

				if ( ret <= 0 ) {

					this->SetError();
					return ret;
				}

				sprintf(tmpbuf,"Read2:fd:%d",this->cSockFd);
				hexdump(ptr, ret, tmpbuf,0);
				ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex) += ret;
			}
			DBUG(ALOG_PARSE,"CONN(%p)after read2:req len:%d alloc size:%d", this,ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex),
					ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex));

			if ( ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex) < ACONN_IOVREQ_ALLOCSIZE(this,this->iovReq.iovIndex) ) {
				ACONN_IOVREQ_BASEOFFSET(this,this->iovReq.iovIndex) += ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex);
				SET_AC_IOVREQ_DESC(this,this->iovReq.iovIndex,AC_IOVREQ_DESC_BODY_PART);

				this->state = body_wait;
			} else {
				SET_AC_IOVREQ_DESC(this,this->iovReq.iovIndex,AC_IOVREQ_DESC_BODY);

				this->state = cmd_done;

			}

			return ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex);
		} else {
			//this->WriteData("CLIENT_ERROR invalid data len \r\n",32);
			this->SetError(AERR_TOKENIZER_CMD_INVALID_DATALEN);
			this->state = cmd_done;
			return -1;
		}

		return 1;

		break;

		default :
		ALERT(ALOG_ALERT, "CONN(%p):invalid conn state:%d fd:%d", this,this->state, this->cSockFd);
		assert(false);
		break;
	}

	return -1;
}

// return len if buffer is Complete
// return 0 if buffer is partial

uint32_t aConn::IsBufferComplete() {
	int i = 0;
	char *ptr;
	uint32_t found = 0;

	ptr = (char *) ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex);
	while (i < (ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex)-1) ) {
		if ( *ptr == '\r') {
			ptr++;
			if(*ptr == '\n') {
				found = (ptr - (char *)ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex)) +1;
				*ptr=0x0;
				ptr--;
				*ptr= 0x0;
				break;
			}
		}
		ptr++;
	}

	return found;
}

// return true or false
int aConn::ProcessHeader() {
	char *bufptr = (char *) ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex);
	int len = ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex);
	char *ptr;
	const char *arr[5];
	int i,x;
	bool boolret1,boolret2,boolret3;
	token_t tokens[MAX_TOKENS];
	size_t ntokens;

	while ( ( (*(bufptr) == ' ') || (*(bufptr) == 0x0) ) && (len > 0) ) {
		bufptr++;len--;
	}

	if ( this->cmd.ctype == ACACHED_CMD_SET ) {

		ntokens = tokenize_command(bufptr, tokens, MAX_TOKENS);
		/****
		 //hexdump(bufptr, 270,"SET CMD",1);
		 ptr = strtok (bufptr," ");
		 i = 0;
		 //		printf("\nCMD:%s",ptr);
		 while (ptr != NULL)
		 {
		 ptr = strtok ((char *)NULL, " ");
		 if ( ptr ) {
		 arr[i] = ptr;
		 //printf("\nargsi:%d ptr:%s",i,ptr);
		 i++;
		 }
		 }
		 **/
		i = ntokens;
		if ( i > 2) {
			this->cmd.keylen= tokens[1].length;
			strncpy(this->cmd.key,tokens[1].value, this->cmd.keylen);
			DBUG(ALOG_PARSE,"CONN(%p):SET:keylen:%d key:(%s)", this, this->cmd.keylen,tokens[1].value);

			boolret1 = safe_strtoul(tokens[2].value, &this->cmd.flags);
			boolret2 = safe_strtoul(tokens[3].value, &this->cmd.exptime);
			boolret3 = safe_strtoul(tokens[4].value, &this->cmd.datalen);
			if ( (boolret1 && boolret2 && boolret3))
			return true;

		}
		//for( x = 0 ; x < i ; x++)
		//DBUG((ALOG_PARSE|ALOG_TERR),"CONN(%p):SET:ERROR:ProcessHeader: index:%d value(%s)", this,  x,this->cmd.keylen,arr[x]);

		this->SetError(AERR_TOKENIZER_CMD_INVALID_HDR);
		return false;

	}

	return true;

}

int aConn::SetCommandType() {
	char *bufptr = (char *) ACONN_IOVREQ_IOVECBASE(this,this->iovReq.iovIndex);
	int len = ACONN_IOVREQ_IOVECLEN(this,this->iovReq.iovIndex);

	//* check if buffer has anything, this will be truncated

			/*
			 if ( (*(this->start) == '\r') && ( *(this->start +1) == '\n')  ) {
			 this->start += 2 ;
			 this->len -= 2;
			 this->nchars += 2 ;
			 }
			 */

	while ( ( (*(bufptr) == ' ') || (*(bufptr) == 0x0) ) && (len > 0) ) {
		bufptr++;len--;
	}

	if ( len > 5 ) {

		if ( strncmp(bufptr, "set", 3) == 0 ) {
			this->cmd.ctype = ACACHED_CMD_SET;
			return ACACHED_CMD_SET;
		} else if ( strncmp(bufptr, "version", 7) == 0 ) {
			this->cmd.ctype = ACACHED_CMD_VERSION;
			return ACACHED_CMD_QUIT;
		} else if ( strncmp(bufptr, "quit", 4) == 0 ) {
			this->cmd.ctype = ACACHED_CMD_QUIT;
			return ACACHED_CMD_QUIT;
		} else if ( strncmp(bufptr, "get", 3) == 0 ) {
			this->cmd.ctype = ACACHED_CMD_GETS;
			return ACACHED_CMD_GETS;
		} else if ( strncmp(bufptr, "gets", 4) == 0 ) {
			this->cmd.ctype = ACACHED_CMD_GETS;
			return ACACHED_CMD_GETS;
		}
	}

	this->SetError(AERR_TOKENIZER_CMD_INVALID);
	hexdump(bufptr,4,"CONN:PARSE:ERROR:INVALID TOKEN",0);
	DBUG(ALOG_TERR, "CONN(%p):PARSE:Set Command Type:ERROR:AERR_TOKENIZER_CMD_INVALID:%d", this,AERR_TOKENIZER_CMD_INVALID );

	return -1;

}

