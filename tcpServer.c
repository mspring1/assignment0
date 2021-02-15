#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

//#include <string.h>
//#include <arpa/inet.h>


void processing(int clientSocket) {

  int check;
  char message[256];
  bzero(message,256);
  check = read(sock, message, 255);

  if (check < 0) {
     perror("ERROR reading from socket");
     exit(1);
  }

  printf("Here is the message: %s\n", message);
  check = write(sock,"I got your message", 18);

  if (check < 0) {
     perror("ERROR writing to socket");
     exit(1);
  }

}

int main(){

  char serverMessage[1024];

  int serverSocket;
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);

  //if(serverSocket < 0){
  //  printf("[-]Error in connection.\n");
  //  exit(1);
  //}
  //printf("[+]Server Socket is created.\n");

  struct sockaddr_in serverAddress;
  memset(&serverAddress, '\0', sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(PORT);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
  listen(serverSocket, 10);

  //if(listen(serverSocket, 10) == 0){
	//	printf("[+]Listening....\n");
	//}else{
	//	printf("[-]Error in binding.\n");
	//}

  int clientSocket;
  struct sockaddr_in newAddr;
  socklen_t addr_size;

  while(1){
		clientSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(clientSocket < 0){
			exit(1);
		}
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

    //threading
    pid_t childpid;

		if((childpid = fork()) == 0){
			close(serverSocket);

			while(1){
				recv(clientSocket, serverMessage, 1024, 0);
				printf("Client: %s\n", serverMessage);
				send(clientSocket, serverMessage, strlen(serverMessage), 0);
				//bzero(serverMessage, sizeof(serverMessage));
			}
		}

	}

	close(clientSocket);

	return 0;
}
