// Aleksander, Danilo e Suryan

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>


#define SERVER_PATH     "/tmp/server"
#define BUFFER_LENGTH    1024
#define FALSE              0
#define		CTRL_BACKLOG	5
#define		FIM		"\n fim da transmissao"


#define NUMCONS 1000


pthread_t cons[NUMCONS];


void *server(void*);

void main(){
   int i=0;
   int    sd=-1, sd2=-1;
   int    rc, length;
   char   buffer[BUFFER_LENGTH];
   struct sockaddr_un serveraddr;
   do{
      sd = socket(AF_UNIX, SOCK_STREAM, 0);

      if (sd < 0){
         perror("socket() failed");
         break;
      }

      memset(&serveraddr, 0, sizeof(serveraddr));
      serveraddr.sun_family = AF_UNIX;
      strcpy(serveraddr.sun_path, SERVER_PATH);

      rc = bind(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
      if (rc < 0){
         perror("bind() failed");
         break;
      }

      rc = listen(sd, 10);
      if (rc< 0){
         perror("listen() failed");
         break;
      }

      printf("Ready for client connect().\n");
      sd2 = accept(sd, NULL, NULL);

      if (sd2 < 0){
         perror("accept() failed");
         break;
      }

      length = BUFFER_LENGTH;
      rc = setsockopt(sd2, SOL_SOCKET, SO_RCVLOWAT,(char *)&length, sizeof(length));
      if (rc < 0){
         perror("setsockopt(SO_RCVLOWAT) failed");
         break;
      }

 for(i=0; i<NUMCONS; i++)
         pthread_create(&(cons[i]), NULL, server, &sd);
 
   
 
      for(i=0; i<NUMCONS; i++)
         pthread_join(cons[i], NULL);
 
   

      rc = recv(sd2, buffer, sizeof(buffer), 0);
       
        

      if (rc < 0){
         perror("\n recv() failed");
         break;
      } 


      if (rc == 0 || rc < sizeof(buffer)){
         printf("\n The client closed the connection before all of the data was sent \n");
         break;
      }

      rc = send(sd2, buffer, sizeof(buffer), 0);
      if (rc < 0){
         perror("\n send() failed");
         break;
      }

      

   } while (FALSE);



   if (sd != -1)
       close(sd);
   if (sd2 != -1)
      close(sd2);
   unlink(SERVER_PATH);
}

void *server(void* sfd) {
   struct sockaddr_in cli_addr;
   char buff[BUFFER_LENGTH] = {};
   int newsocketfd, n, fd, cli_len;
   int socketfd = *(int *)sfd;
   listen(socketfd, CTRL_BACKLOG);
   cli_len = sizeof(cli_addr);

   if ((newsocketfd = accept(socketfd, (struct sockaddr *) &cli_addr, &cli_len)) < 0) {
	printf("\nFuncao server: erro no accept\n");
	close(socketfd);
	exit(0);
   }

   if ((n = read(newsocketfd, buff, BUFFER_LENGTH)) < 0) {
	printf("\nFuncao server: erro no recebimento do nome do arquivo\n");
	close(socketfd);
	close(newsocketfd);
	exit(0);
   }
   buff[n] = '\0';
   printf("\n\tNome do arquivo: %s\t(%d bytes)", buff, n);

   if ((fd = open(buff, 0)) < 0) {
	sprintf(buff, "\nServidor nao consegue abrir arquivo solicitado\n");
	n = strlen(buff);
	if (write(newsocketfd, buff, n) != n) {
		printf("\nFuncao server: erro no envio da mensagem de erro\n");
		close(socketfd);
		close(newsocketfd);
		exit(0);
	}

   } else {
	while ((n = read(fd, buff, BUFFER_LENGTH)) > 0) {
		if (write(newsocketfd, buff, n) != n) {
		    printf("\nFuncao server: erro no envio dos dados do arq. pelo socket\n");
		    close(socketfd);
		    close(newsocketfd);
		    exit(0);
		}
	}

	if (n < 0) {
	    printf("\nFuncao server: erro na leitura do arquivo\n");
	    close(socketfd);
	    close(newsocketfd);
	    exit(0);
	}
   } 

   n = sizeof(FIM);
   if (write(newsocketfd, FIM, n) != n) {
	printf("\nFuncao server: erro no envio do fim de transmissao pelo socket\n");
	close(socketfd);
	close(newsocketfd);
	exit(0);
   }

   return(0);
}
