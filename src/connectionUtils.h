/*

** Fichero: connectionUtils.h
** Autores:
** Francisco Pinto Santos  DNI 70918455W
** Hector Sanchez San Blas DNI 70901148Z
*/

#ifndef __CONNECTIONUTILS_H
#define __CONNECTIONUTILS_H

//TODO mirar cuales son los tamanos de estos y el codigo que corresopnde a octet
#define MSG_FILE_NAME_SIZE 512
#define MSG_MODE_SIZE 512
#define MSG_DATA_SIZE 512
#define MSG_ERROR_SIZE 512

#define OCTET_MODE "Octet"

typedef enum { READ, WRITE } opMode;
typedef enum { UNKNOWN=0, FILE_NOT_FOUND=1, DISK_FULL=3, ILLEGAL_OPERATION=4, FILE_ALREADY_EXISTS=6 } errorMsgCodes;

typedef struct{
	 short header;
	 char operationMode;
	 char fileName[MSG_FILE_NAME_SIZE];
	 byte separator;
	 char characterMode[MSG_MODE_SIZE];
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
	char errorMsg[MSG_ERROR_SIZE];
	byte tail;
}errMsg;


rwMsg getReadMsg(opMode mode, char * fileName);
dataMsg getDataMsg(int blockNumber, char * data);
errMsg getErrMsg(errorMsgCodes errorCode, char * errorMsg);


void openTcpSocket();
void openUdpSocket();

void closeTcpSocket();
void closeTcpSocket();

#endif
