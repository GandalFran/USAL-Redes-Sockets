/*

** Fichero: msgUtils.h
** Autores:
** Francisco Pinto Santos  DNI 70918455W
** Hector Sanchez San Blas DNI 70901148Z
*/

#ifndef __MSGUTILS_H
#define __MSGUTILS_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//TODO mirar cuales son los tamanos de estos y el codigo que corresopnde a octet
#define MSG_FILE_NAME_SIZE 512
#define MSG_MODE_SIZE 512
#define MSG_DATA_SIZE 512
#define MSG_ERROR_SIZE 512

#define OCTET_MODE "Octet"

typedef char byte;

typedef enum { READ, WRITE } opMode;
typedef enum { UNKNOWN=0, FILE_NOT_FOUND=1, DISK_FULL=3, ILLEGAL_OPERATION=4, FILE_ALREADY_EXISTS=6 } errorMsgCodes;

typedef struct{
	 short header;
	 byte operationMode;
	 byte fileName[MSG_FILE_NAME_SIZE];
	 byte separator;
	 byte characterMode[MSG_MODE_SIZE];
	 byte tail;
}rwMsg;

typedef struct{
	short header;
	short blockNumber;
	byte data[MSG_DATA_SIZE];
}dataMsg;

typedef struct{
	short header;
	short blockNumber;
}ackMsg;

typedef struct{
	short header;
	short errorCode;
	byte errorMsg[MSG_ERROR_SIZE];
	byte tail;
}errMsg;

void fillReadMsg(rwMsg*msg, opMode mode, char * fileName);
void fillDataMsg(dataMsg*msg, short blockNumber, char * data);
void fillAckMsg(ackMsg*msg, short blockNumber);
void fillErrMsg(errMsg*msg, errorMsgCodes errorCode, char * errorMsg);

#endif