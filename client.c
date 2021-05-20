// Aleksander, Danilo e Suryan

#include <stdio.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#define SERVER_PATH     "/tmp/server"
#define BUFFER_LENGTH    1024
#define FALSE              0
#define FNAME            256
#define		FIM		"\n fim da transmissao"

void client(int, char [FNAME]);

void main(int argc, char *argv[]){
   int    sd=-1, rc, bytesReceived;
   char   buffer[BUFFER_LENGTH];
   char   filename[FNAME];
   struct sockaddr_un serveraddr;

  if (argc != 2) {
	printf("\nUsage: clientcp  filename\n\n");
	exit(0);
    }

   strcpy(filename, argv[1]);
   do{
      sd = socket(AF_UNIX, SOCK_STREAM, 0);
      if (sd < 0){
         perror("socket() failed");
         break;
      }
      memset(&serveraddr, 0, sizeof(serveraddr));
      serveraddr.sun_family = AF_UNIX;
    
         strcpy(serveraddr.sun_path, SERVER_PATH);

      rc = connect(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
      if (rc < 0){
         perror("connect() failed");
         break;
      }
     
      
      memset(buffer, 'a', sizeof(buffer));
       client(sd, filename);
      rc = send(sd, buffer, sizeof(buffer), 0);
      if (rc < 0){
         perror("send() failed");
         break;
      }
      bytesReceived = 0;
      

      while (bytesReceived < BUFFER_LENGTH){
         rc = recv(sd, & buffer[bytesReceived], BUFFER_LENGTH - bytesReceived, 0);
         if (rc < 0){
            perror("recv() failed");
            break;
         }
         else if (rc == 0){
            printf("The server closed the connection\n");
            break;
         }

         bytesReceived += rc;

         

      }
      

   }while (FALSE);
   if (sd != -1)
   close(sd);
}



void client(int socketfd, char filename[FNAME]) {
    char buff[BUFFER_LENGTH] = {};
    int  n;

    // Envia o nome do arquivo ao servidor
    n = strlen(filename);
    if (write(socketfd, filename, n) != n) {
	printf("\nFuncao client: erro no envio do nome do arquivo ao servidor\n");
	close(socketfd);
	exit(0);
    }

    // Le os dados vindos do servidor e os escreve para a saida padrao
    do {
	if ((n = read(socketfd, buff, BUFFER_LENGTH)) < 0) {
		printf("\nFuncao client: erro no recebimento do conteudo do arquivo\n");
		close(socketfd);
		exit(0);
	}
	if (strcmp(buff, FIM) != 0)		// Se nao eh fim de transmissao
		if (write(1, buff, n) != n) {	// Escreve os dados para a saida padrao (fd 1 = stdout)
		    printf("\nFuncao client: erro na escrita para o video\n");
		    close(socketfd);
		    exit(0);
		}
    } while (strcmp(buff, FIM) != 0);
    return;
}	// Fim da funcao client

