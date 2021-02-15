#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


//#include <string.h>
//#include <arpa/inet.h>

//create socket, connect to an ip address, recieve function to get data back


int main(int argc, char *argv[]) {

	client_parseopt(argc, argv);

	//creating a socket
	int clientSocket;
	clientSocket = socket(AF_INET, SOCKSTREAM, 0); //0 means TCP

	//define address of socket
	struct sockaddr_in serverAddress;

	//sets type of address
	serverAddress;.sin_family = AF_INET;

	//sets port
	serverAddress;.sin_port=htons(9002);

	//sets actual address
	serverAddress;.sin_addr.s_addr = INADDR_ANY;


	//connect to server address casts address to new struct type
	int connection = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	if (connection == -1) {
		printf("connection error");
	}

	// send data to server
	char data[1024];
	send((clientSocket, data, sizeof(data));

	// recieve data from server
	char serverResponse[1024];
	recv(clientSocket, &serverResponse, sizeof(serverResponse), 0); //0 is optional flags parameter
	printf("Data Recieved: %s", serverResponse);

	//close socket
	close(sock);

	return 0;

}
