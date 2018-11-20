/*

** Fichero: connectionUtils.c
** Autores:
** Francisco Pinto Santos  DNI 70918455W
** Hector Sanchez San Blas DNI 70901148Z
*/

#include "connectionUtils.h"

rwMsg getReadMsg(opMode mode, char * fileName){
	rwMsg msg;

	memset(&msg,0,sizeof(rwMsg));
	msg.operationMode = (mode == READ)? 1:2;
	strcpy(msg.fileName,fileName);
	strcpy(msg.characterMode,OCTET_MODE);

	return msg;
}

dataMsg getDataMsg(int blockNumber, char * data){
	dataMsg msg;

	memset(&msg,0,sizeof(dataMsg));
	msg.blockNumber = (short) bockNumber;
	strcpy(msg.data,data);

	return msg;
}

errMsg getErrMsg(errorMsgCodes errorCode, char * errorMsg){
	errMsg msg;

	memset(&msg,0,sizeof(errMsg));
	msg.errorCode = (short)errorCode;
	strcpy(msg.errorMsg,errorMsg)

	return msg;
}

void openTcpSocket(){

}
void openUdpSocket(){

}

void closeTcpSocket(){

}
void closeTcpSocket(){

}

