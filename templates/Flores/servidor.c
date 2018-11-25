/*
 ** Fichero: servidor.c
 ** Autores:
 ** Julio García Valdunciel DNI 70892288D
 ** David Flores Barbero DNI 70907575R
 */


/*
 *          		S E R V I D O R
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

#define PUERTO 7575
#define TAM_BUFFER 1024
#define MAXHOST 128
#define ADDRNOTFOUND 0xffffffff	/* value returned for unknown host */



typedef struct tipoMensaje{
	char orden[8];
	char pagina[48];
	char host[24];
	char keepAlive[16];
	char codigoError[24];
} Mensaje;


void serverTCP(int s, struct sockaddr_in peeraddr_in);
void serverUDP(int s , Mensaje *msg, struct sockaddr_in clientaddr_in);
void errout(char *);		/* declare error out routine */
void recuperarMensaje(char *buffer, Mensaje *msg);
char * construirRespuesta(Mensaje *msg);

extern int errno;
int FIN = 0;

//Log peticiones
FILE *fileLog;
char *ipCliente;
char fechaHora[60];

/*----------------------------------------*/
void finalizar(){

	FIN = 1;
}
/*----------------------------------------*/

/*----------------------------------------*/
void errout(char *hostname)
{
	printf("Connection with %s aborted on error\n", hostname);
	exit(1);
}
/*----------------------------------------*/

/*----------------------------------------*/
int main(int argc, char * argv[]){

	int s_TCP, s_UDP;		/* connected socket descriptor */
    int ls_TCP, ls_UDP;				/* listen socket descriptor */
	char buffer[TAM_BUFFER];
	char hostname[MAXHOST];
    int tamMsg;				    /* contains the number of bytes read */

	struct sigaction sa = {.sa_handler = SIG_IGN};

	struct sockaddr_in myaddr_in;	/* for local socket address */
    struct sockaddr_in clientaddr_in;	/* for peer socket address */
	int addrlen;
	time_t timevar;

	fd_set readmask;
    int numfds,s_mayor;

	struct sigaction vec;


	Mensaje msg;

		/* Create the listen socket. */
	ls_TCP = socket (AF_INET, SOCK_STREAM, 0);
	if (ls_TCP == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket TCP\n", argv[0]);
		exit(1);
	}
	/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
   	memset ((char *)&clientaddr_in, 0, sizeof(struct sockaddr_in));

    addrlen = sizeof(struct sockaddr_in);

		/* Set up address structure for the listen socket. */
	myaddr_in.sin_family = AF_INET;

	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	myaddr_in.sin_port = htons(PUERTO);

	/* Bind the listen address to the socket. */
	if (bind(ls_TCP, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) 		{
		perror(argv[0]);
		fprintf(stderr, "%s: unable to bind address TCP\n", argv[0]);
		exit(1);
	}

	/* Initiate the listen on the socket so remote users
	 * can connect.  The listen backlog is set to 5, which
	 * is the largest currently supported.
	 */
	if (listen(ls_TCP, 5) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to listen on socket\n", argv[0]);
		exit(1);
	}

	/* Create the socket UDP. */
	ls_UDP = socket (AF_INET, SOCK_DGRAM, 0);
	if (ls_UDP == -1) {
		perror(argv[0]);
		printf("%s: unable to create socket UDP\n", argv[0]);
		exit(1);
	}

	/* Bind the server's address to the socket. */
	if (bind(ls_UDP, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		printf("%s: unable to bind address UDP\n", argv[0]);
		exit(1);
	}
	 /* Se desvincula del terminal */
	setpgrp();

	switch (fork()) {
		case -1:		/* Unable to fork, for some reason. */
			perror(argv[0]);
			fprintf(stderr, "%s: unable to fork daemon\n", argv[0]);
			exit(1);
	case 0:     /* The child process (daemon) comes here. */

		close(stdin);
		close(stderr);

		 /* Registra la señal SIGCHLD para no tener que esperar por los hijos */
		if ( sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror(" sigaction(SIGCHLD)");
            fprintf(stderr,"%s: unable to register the SIGCHLD signal\n", argv[0]);
            exit(1);
		}
	   /* Registrar SIGTERM para la finalizacion ordenada del programa servidor */
        vec.sa_handler = (void *) finalizar;
        vec.sa_flags = 0;
        if ( sigaction(SIGTERM, &vec, (struct sigaction *) 0) == -1) {
            perror(" sigaction(SIGTERM)");
            fprintf(stderr,"%s: unable to register the SIGTERM signal\n", argv[0]);
            exit(1);
        }




		while (!FIN) {

			/* Meter en el conjunto de sockets los sockets UDP y TCP */
		    FD_ZERO(&readmask);
		    FD_SET(ls_TCP, &readmask);
		    FD_SET(ls_UDP, &readmask);

			/*
		    Seleccionar el descriptor del socket que ha cambiado. Deja una marca en
		    el conjunto de sockets (readmask)
		    */
			if (ls_TCP > ls_UDP){
				s_mayor=ls_TCP;
			} else s_mayor=ls_UDP;

			/* Se queda esperando a que un socket cambie */
			if ( (numfds = select(s_mayor+1, &readmask, (fd_set *)0, (fd_set *)0, NULL)) < 0){
				 if (errno == EINTR) {
                    FIN=1;
		            close (ls_TCP, ls_UDP);
                    perror("\nFinalizando el servidor. SeÃal recibida en elect\n ");
                }
			} else {
					/* Si el socket que ha cambiado es el listen TCP */
				if (FD_ISSET(ls_TCP, &readmask)) {
					/* Espera a una conexion, guarda su direccion en clientaddr_in y un nuevo descriptor de socket para la conexion */
					s_TCP = accept(ls_TCP, (struct sockaddr *) &clientaddr_in, &addrlen);
					/* Si falla */
					if (s_TCP == -1) exit(1);

					switch (fork()) {
						case -1:	/* Can't fork, just exit. */
							exit(1);
						case 0:		/* Child process comes here. */
			            	close(ls_TCP); /* Close the listen socket inherited from the daemon. */

							serverTCP(s_TCP, clientaddr_in);

							exit(0);
						default:
							/* el proceso daemon cierra el socket porque ya lo ha heredado el hijo */
							close(s_TCP);
	    			}
				} /* De TCP*/
			/* Comprobamos si el socket seleccionado es el socket UDP */
				if (FD_ISSET(ls_UDP, &readmask)) {

					tamMsg = recvfrom(ls_UDP, buffer, TAM_BUFFER - 1, 0, (struct sockaddr *)&clientaddr_in, &addrlen);
					if (tamMsg == -1) exit(1);
				    buffer[tamMsg]='\0';

fprintf(stderr,"Servidor recibe:\n%s\n",buffer);

					memset(msg.orden, 0, sizeof(msg.orden));
					memset(msg.pagina, 0, sizeof(msg.pagina));
					memset(msg.host, 0, sizeof(msg.host));
					memset(msg.keepAlive, 0, sizeof(msg.keepAlive));

					recuperarMensaje(buffer,&msg);
					
					//Inicio del log:
					if(NULL == (fileLog = fopen("peticiones.log","a+"))){
						perror("No se pudo crear el log");
						exit(1);
					}

					if(getnameinfo((struct sockaddr *)&clientaddr_in ,sizeof(clientaddr_in), hostname,MAXHOST,NULL,0,0)){
							/* Si la informacion del host no esta disponible */
							/* inet_ntop para interoperatividad con IPv6 */
				   		if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
				   	    	perror(" inet_ntop \n");

					}

					memset(fechaHora,0,sizeof(fechaHora));

					time (&timevar);
					strncpy(fechaHora, (char *)ctime(&timevar), strlen((char *)ctime(&timevar))-1);

					ipCliente = inet_ntoa(clientaddr_in.sin_addr);

					fprintf(fileLog, "[%s] %s (%s) %s p:%d\n",fechaHora , ipCliente, hostname, "UDP", clientaddr_in.sin_port);

					if(strcmp(msg.keepAlive,"keep-alive") == 0){

						switch (fork()) {
							case -1:	/* Can't fork, just exit. */
								exit(1);
							case 0:		/* Child process comes here. */
					        	close(ls_UDP); /* Close the listen socket inherited from the daemon. */

									/* Set up address structure for the listen socket. */
								myaddr_in.sin_family = AF_INET;

								myaddr_in.sin_addr.s_addr = INADDR_ANY;
								myaddr_in.sin_port = htons(0);

								s_UDP = socket (AF_INET, SOCK_DGRAM, 0);
								if (s_UDP == -1) {
									perror(argv[0]);
									printf("%s: unable to create socket UDP\n", argv[0]);
									exit(1);
								}

								/* Bind the server's address to the socket. */
								if (bind(s_UDP, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
									perror(argv[0]);
									printf("%s: unable to bind address UDP\n", argv[0]);
									exit(1);
								}


								while(strcmp(msg.keepAlive,"keep-alive") == 0){

									serverUDP (s_UDP, &msg, clientaddr_in);

									tamMsg = recvfrom(s_UDP, buffer, TAM_BUFFER - 1, 0, (struct sockaddr *)&clientaddr_in, &addrlen);
									if (tamMsg == -1) exit(1);
									buffer[tamMsg]='\0';

fprintf(stderr,"Servidor recibe:\n%s\n",buffer);

									memset(msg.orden, 0, sizeof(msg.orden));
									memset(msg.pagina, 0, sizeof(msg.pagina));
									memset(msg.host, 0, sizeof(msg.host));
									memset(msg.keepAlive, 0, sizeof(msg.keepAlive));

									/* Esta función es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta. */
									recuperarMensaje(buffer,&msg);

								}
								serverUDP (s_UDP, &msg, clientaddr_in);
								fclose(fileLog);

								exit(0);
							default:
								/* el proceso daemon cierra el socket porque ya lo ha heredado el hijo */
								close(s_TCP);
						}
					}
					
					

				} /* De UDP*/
        	}

		}   /* Fin del bucle infinito de atención a clientes */
        /* Cerramos los sockets UDP y TCP */

        close(ls_TCP);
        close(s_UDP);

        printf("\nFin de programa servidor!\n");

	default:		/* Parent process comes here. */
		exit(0);
	}
}
/*----------------------------------------*/

/*
 *				S E R V E R  T C P
 *
 */

/*----------------------------------------*/
void serverTCP(int s, struct sockaddr_in clientaddr_in){
/*----------------------------------------*/

	int reqcnt = 0;		/* keeps count of number of requests */
	char buf[TAM_BUFFER];		/* This example uses TAM_BUFFER byte messages. */
	char hostname[MAXHOST];		/* remote host's name string */


	int tamMsg, status;
	time_t timevar;
	struct linger linger; /* allow a lingering, graceful close; */
    				            /* used when setting SO_LINGER */

	Mensaje msg;

	//creacion del log
	FILE *fileLog;
	char *ipCliente;
	char fechaHora[60];

	char *mensajeRespuesta;

	status = getnameinfo((struct sockaddr *)&clientaddr_in ,sizeof(clientaddr_in),
                           hostname,MAXHOST,NULL,0,0);

	if(status){
			/* Si la informacion del host no esta disponible */
			/* inet_ntop para interoperatividad con IPv6 */
   		if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
   	    	perror(" inet_ntop \n");

	}


	//Inicio del log:
	if(NULL == (fileLog = fopen("peticiones.log","a+"))){
		perror("No se pudo crear el log");
		exit(1);
	}

	 /* Log a startup message. */
    time (&timevar);
	printf("Startup from %s port %u at %s", hostname, ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));

	//TODO para keep alive habra que hacerlo de otra forma
	linger.l_onoff  =1;
	linger.l_linger =1;

	/* asigna la operacion al socket para el cierre cuando termine la comunicacion */
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) == -1) {
		errout(hostname);
	}
	/* Proceso de lectura */

	memset(fechaHora,0,sizeof(fechaHora));

    time (&timevar);
	strncpy(fechaHora, (char *)ctime(&timevar), strlen((char *)ctime(&timevar)) -1);

	ipCliente = inet_ntoa(clientaddr_in.sin_addr);

	fprintf(fileLog, "[%s] %s (%s) %s p:%d\n",fechaHora , ipCliente, hostname, "TCP", clientaddr_in.sin_port  );
	
//****************************** Lectura de mensajes**********************/
	while (tamMsg = recv(s, buf, TAM_BUFFER -1, 0)){
		if (tamMsg == -1) errout(hostname);
		

	reqcnt++;
	buf[tamMsg] = '\0';

	memset(msg.orden, 0, sizeof(msg.orden));
	memset(msg.pagina, 0, sizeof(msg.pagina));
	memset(msg.host, 0, sizeof(msg.host));
	memset(msg.keepAlive, 0, sizeof(msg.keepAlive));

fprintf(stderr,"M:%d Servidor recibe n:%lu:\n%s",reqcnt, strlen(buf), buf);

	//TODO tratamiento del mensaje
	
	recuperarMensaje(buf,&msg);

	mensajeRespuesta = construirRespuesta(&msg);

fprintf(stderr,"Respuesta:\n%s\n",mensajeRespuesta);

	//TODO envio al cliente
	if (send(s, mensajeRespuesta, strlen(mensajeRespuesta), 0) == -1) errout(hostname);

	free(mensajeRespuesta);

    time (&timevar);
	strncpy(fechaHora, (char *)ctime(&timevar), strlen((char *)ctime(&timevar))-1);
	fprintf(fileLog, "[%s] %s Peticion: %s %s %s\n",fechaHora , ipCliente, msg.orden, msg.pagina, msg.codigoError);

	}

	/* Termina la comunicacion */
	fclose(fileLog);
	close(s);
	/* Log a finishing message. */
	time (&timevar);
	printf("Completed %s port %u, %d requests, at %s\n", hostname, ntohs(clientaddr_in.sin_port), reqcnt, (char *) ctime(&timevar));
}
/*----------------------------------------*/





/*----------------------------------------*/
void serverUDP(int s, Mensaje *msg, struct sockaddr_in clientaddr_in){
/*----------------------------------------*/

	time_t timevar;
	char *mensajeRespuesta;
	int tamMsg;
	

	mensajeRespuesta = construirRespuesta(msg);

fprintf(stderr,"Servidor responde:\n%s\n",mensajeRespuesta);

	tamMsg = sendto (s,mensajeRespuesta ,strlen(mensajeRespuesta) , 0, (struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));

	free(mensajeRespuesta);

    time (&timevar);
	strncpy(fechaHora, (char *)ctime(&timevar), strlen((char *)ctime(&timevar))-1);
	fprintf(fileLog, "[%s] %s Peticion: %s %s %s\n",fechaHora , ipCliente, msg->orden, msg->pagina, msg->codigoError);

	//comprobacion del envio
	if ( tamMsg == -1) {
	     perror("serverUDP");
	     printf("%s: sendto error\n", "serverUDP");
	     return;
	}

}

void recuperarMensaje(char *buffer, Mensaje *msg){

	//Tratamiento mensaje
	char *linea;
	char *palabra;
	char *bufferLineas;
	char *bufferPalabras;
	int numeroPalabra = 1;
	int numeroLinea = 1;

	linea = strtok_r(buffer,"\r\n",&bufferLineas);
	numeroLinea = 1;
	while(linea != NULL){
		switch(numeroLinea){
			case 1:					//GET /index.html HTTP/1.1
				palabra = strtok_r(linea," ",&bufferPalabras);
				numeroPalabra = 1;
				while(palabra != NULL){
					switch(numeroPalabra){
						case 1:		//GET
							strcpy(msg->orden,palabra);
							break;
						case 2:		// /index.html
							strcpy(msg->pagina,palabra);
							break;
						case 3:		//Http/1.1
							break;
					}
					palabra = strtok_r(NULL," ",&bufferPalabras);
					numeroPalabra++;
				}
				break;

			case 2:					//Host: localhost | olivo.fis.usal.es
				palabra = strtok_r(linea," ",&bufferPalabras);
				numeroPalabra = 1;
				while(palabra != NULL){
					switch(numeroPalabra){
						case 1:		// host:
							break;
						case 2:		//localhost
							strcpy(msg->host,palabra);

							break;

					}
					palabra = strtok_r(NULL," ",&bufferPalabras);
					numeroPalabra++;
				}
				break;

			case 3:					//Connection: keep-alive | close
					//Solo entraria si hay tercera linea
				palabra = strtok_r(linea," ",&bufferPalabras);
				palabra = strtok_r(NULL," ",&bufferPalabras);

				strcpy(msg->keepAlive,palabra);

				break;			
		}

		linea = strtok_r(NULL,"\r\n",&bufferLineas);
		numeroLinea++;
	}
	if(strcmp(msg->keepAlive,"keep-alive") != 0){
		strcpy(msg->keepAlive,"close");
	}
}

char *construirRespuesta(Mensaje *msg){

	//Lectura de los html
	FILE *fileHtml;
	char ruta[40];
	char contenidoPagina[1920];
	char lineaPagina[100];

	//mensaje de respuesta al cliente
	char *mensajeRespuesta;

	memset(contenidoPagina, 0, sizeof(contenidoPagina));

	mensajeRespuesta = (char *)malloc(1024*sizeof(char));

	if(0 != strcmp(msg->orden,"GET")){
		strcpy(msg->codigoError,"501 Not implemented");
	} else {

		strcpy(ruta,"www");
		strcat(ruta, msg->pagina);

		if(NULL == (fileHtml = (fopen(ruta,"r")))){
			strcpy(msg->codigoError,"404 Not found");

		} else {

			fgets(lineaPagina, sizeof(lineaPagina), fileHtml);
			while(!feof(fileHtml)){

				strcat(contenidoPagina,lineaPagina);
				fgets(lineaPagina, sizeof(lineaPagina), fileHtml);

			}
			contenidoPagina[strlen(contenidoPagina)-1] = '\0';
			strcpy(msg->codigoError,"200 OK");
			fclose(fileHtml);

		}

	}

	//Construccion mensaje de respuesta
	strcpy(mensajeRespuesta,"HTTP/1.1 ");
	strcat(mensajeRespuesta,msg->codigoError);
	strcat(mensajeRespuesta, "\r\n");
	strcat(mensajeRespuesta, "Server: servidor 7575\r\n");

	strcat(mensajeRespuesta, "Connection: ");
	strcat(mensajeRespuesta, msg->keepAlive);
	strcat(mensajeRespuesta, "\r\n");

	strcat(mensajeRespuesta, "\r\n");

	if(strcmp(msg->codigoError,"200 OK") == 0){
		strcat(mensajeRespuesta, contenidoPagina);
	} else {
		strcpy(contenidoPagina,"<html><body><h1>");
		strcat(contenidoPagina, msg->codigoError);
		strcat(contenidoPagina, "</h1></body></html>");
		strcat(mensajeRespuesta, contenidoPagina);
	}
	return mensajeRespuesta;
}
