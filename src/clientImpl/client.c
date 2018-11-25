/*

** Fichero: cliente.c
** Autores:
** Francisco Pinto Santos  DNI 70918455W
** Hector Sanchez San Blas DNI 70901148Z
*/

#include <stdio.h>
#include <stdlib.h>
#include "tcpClient.h"
#include "udpClient.h"
#include "../utils/utils.h"
#include "../utils/logUtils.h"
#include "../utils/msgUtils.h"
#include "../utils/concurrencyUtils.h"

#define USAGE_ERROR_MSG "Error: usage ./client <server name or ip> <TCP or UDP> <r or w> <file>"

#define TCP_ARG "TCP"
#define UDP_ARG "UDP"
#define READ_ARG "r"
#define WRITE_ARG "w"

void tcpReadMode(void);
void tcpWriteMode(void);
void udpReadMode(void);
void udpWriteMode(void);

int main(int argc, char * argv[]){
	//Register handlers for signals

	//Check the args are correct
	if( argc != 5
		|| (!strcmp(argv[2],TCP_ARG) && !strcmp(argv[2],UDP_ARG))
		|| (!strcmp(argv[3],READ_ARG) && !strcmp(argv[3],WRITE_ARG))
	){
		fprintf(stderr,"%s",USAGE_ERROR_MSG);
	}

	if(!strcmp(argv[2],TCP_ARG)){
		if(!strcmp(argv[3],READ_ARG))
			tcpReadMode();
		else
			tcpWriteMode();
	}else{
		if(!strcmp(argv[3],READ_ARG))
			udpReadMode();
		else
			udpWriteMode();
	}

	return 0;
}