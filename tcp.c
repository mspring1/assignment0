#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stddef.h>
#include <math.h>
#include "hash.h"
#include <io.h>
#include <fcntl.h>
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netinet/in.h>

FILE *ofp;
FILE *fp;

struct server_arguments {
	int port;
	char *salt;
	size_t salt_len;
};

struct client_arguments {
	char ip_address[16]; /* You can store this as a string, but I probably wouldn't */
	int port; /* is there already a structure you can store the address
	           * and port in instead of like this? */
	int hashnum;
	int smin;
	int smax;
	char *filename; /* you can store this as a string, but I probably wouldn't */
};




char **split(char *line, const char *sep) {
    char **ret = (char **)malloc(20 * sizeof(char *));

    char *start = line, *end;
    size_t l = strlen(sep);
    int count = 0;

    while (end = strstr(start, sep))
    {
        ptrdiff_t diff = end - start;
        ret[count] = (char *)malloc(diff + 1);
        strncpy(ret[count], start, diff);
        ret[count][diff] = '\0';
        count++;
        start = end + l;
    }

    if (*start) // in case last is not a separator
    {
        ret[count] = (char *)malloc(strlen(start) + 1);
        strcpy(ret[count], start);
        count++;
    }

    ret[count] = NULL;

    return ret;
}

void deleteinput(char *str, int len){
  if (len <= strlen(str)) {
    strcpy(&str[0],&str[len]);
  }
}



int client(struct client_arguments *args) {

	//creating a socket
  char data[args->smax];
  char serverResponse[args->smax];

	int clientSocket;
	clientSocket = socket(AF_INET, SOCKSTREAM, 0); //0 means TCP

	//define address of socket
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(args->port);
	serverAddress.sin_addr.s_addr = args->ip_address;


	//connect to server address casts address to new struct type
	int connection = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	if (connection == -1) {
		printf("connection error");
	}


  //send initialization
  char init[256];
  snprintf(init, sizeof(init), "1,%d", args->hashnum);
  send(clientSocket, init, sizeof(init));

  // receive acknowledgement
  char ack[256];
  recv(clientSocket, &ack, sizeof(ack), 0); //0 is optional flags parameter

  //send HashRequests
  char currHashRequest[256];


  while (args->hashnum) {

    //read random number of bytes
    int f = open(args->filename, O_RDONLY);
    int bytes = (rand() % (args->smax - args->smin + 1)) + args->smin;
    char *currdata;
    read (f, currdata, bytes);

    //send HashRequest
    snprintf(currHashRequest, sizeof(currHashRequest), "3,%d,%s", strlen(currdata), currdata);
    send(clientSocket, currHashRequest, sizeof(currHashRequest));

    args->hashnum = args->hashnum - 1;
  }

  //receive HashResponses
  char currHashResponse[256];
  while (recv(clientSocket, &currHashResponse, sizeof(currHashResponse), 0) != -1) {

    send(clientSocket, currHashResponse, sizeof(currHashResponse));

    char **type = split(currHashResponse, ",");
    deleteinput(currHashResponse, strlen(*type) + 1);
    char **i = split(currHashResponse, ",");
    deleteinput(currHashResponse, strlen(*i) + 1);
    char *hash = currHashResponse;

  	printf("%s: 0x%x", *i, atoi(hash));

  }

	//close socket
	close(clientSocket);

	return 0;

}



int server(struct server_arguments *args){

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
  serverAddress.sin_port = htons(args->port);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
  listen(serverSocket, 10);

  //if(listen(serverSocket, 10) == 0){
	//	printf("[+]Listening....\n");
	//}else{
	//	printf("[-]Error in binding.\n");
	//}

  int clientSocket;
  struct sockaddr_in clientAddress;
  socklen_t addr_size;

  while(1){
		clientSocket = accept(sockfd, (struct sockaddr*)&clientAddress, &addr_size);
		if(clientSocket < 0){
			exit(1);
		}
		//printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

    //threading
    pid_t childpid;

		if((childpid = fork()) == 0){
			close(serverSocket);

			while(1){

        //receive initialization
        char init[256];
        recv(clientSocket, init, 1024, 0);

        char **type = split(init, ",");
        deleteinput(init, strlen(*type) + 1);
        char *numhash = init;

        //send acknowledgement
        char ack[256];
        snprintf(ack, sizeof(ack), "2,%d", atoi(numhash)*38);
        send(clientSocket, ack, strlen(ack), 0);

        //recieve HashRequests
        int i = 0;
        char currHashRequest[256];

        while (i < atoi(numhash)) {
          recv(clientSocket, currHashRequest, 1024, 0);

          char **type = split(currHashRequest, ",");
          deleteinput(currHashRequest, strlen(*type) + 1);
          char **len = split(currHashRequest, ",");
          deleteinput(currHashRequest, strlen(*len) + 1);
          char *data = currHashRequest;

          //compute HashResponse

          const uint8_t *payload = (const unsigned char*)data;
          uint8_t *hash;
          const uint8_t *s = (const unsigned char *)atol(args->salt);
          struct checksum_ctx *context = checksum_create(s, args->salt_len);
          checksum_update(context, payload);
          checksum_finish(context, payload, atoi(*len), hash);
          checksum_destroy(context);


          //send HashResponse
          char currHashResponse[256];
          snprintf(currHashResponse, sizeof(currHashResponse), "4,%d,%d", i, *hash);
          send(clientSocket, currHashResponse, strlen(currHashResponse), 0);
          i++;
        }

				//bzero(serverMessage, sizeof(serverMessage));
			}
		}

	}

	close(clientSocket);

	return 0;
}







int main() {

  fp = stdin;//fopen ("input-4-1.txt", "r");
  ofp = stdout;//fopen("Skiju.txt", "w");
  char str[100];


  if (ofp == NULL) {
    exit(1);
  }

  while (fgets(str, 100, fp)) {
      // Separate operation name
      char **type = split(str, " ");
      deleteinput(str, strlen(*type) + 1);

      if (strcmp(*type, "server") == 0) {

        struct server_arguments *args = {};

        int done = 0;

        while (done < 2) {
          char **k = split(str, " ");
          char *key = *k;
          deleteinput(str, strlen(key) + 1);
          char **v = split(str, " ");
          char *value = *v;
          deleteinput(str, strlen(value) + 1);

          if (strcmp(key, "-p") == 0) {
            args->port = atoi(value);
            done++;
          }
          if (strcmp(key, "-s") == 0) {
            args->salt_len = strlen(value);
        		args->salt = (char*)malloc(args->salt_len + 1);
        		strcpy(args->salt, value);
            done++;
          }

        }
        server(args);

      }

      else if (strcmp(*type, "client") == 0) {

        int done = 0;
        struct client_arguments *args = {};
        while (done < 5) {

          char **k = split(str, " ");
          char *key = *k;
          deleteinput(str, strlen(key) + 1);

          char **v = split(str, " ");
          char *value = *v;
          deleteinput(str, strlen(value) + 1);

          if (strcmp(key, "-a") == 0) {
            strncpy(args->ip_address, value, 16);
            done++;
          }
          else if (strcmp(key, "-p") == 0) {
            args->port = atoi(value);
            done++;
          }
          else if (strcmp(key, "-n") == 0) {
            args->hashnum = atoi(value);
            done++;
          }
          else if (strcmp(key, "-f") == 0) {
            int len = strlen(value);
            args->filename = (char*)malloc(len + 1);
            strcpy(args->filename, value);
            done++;
          }
          else {
            char **kmn = split(key, "=");
            char *kmin = *kmn;
            deleteinput(str, strlen(kmin) + 1);
            char *vmin = key;

            char **kmx = split(value, "=");
            char *kmax = *kmx;
            deleteinput(str, strlen(kmax) + 1);
            char *vmax = value;

            if (strcmp(kmin, "--smin") == 0) {
              args->smin = atoi(vmin);
            }

            if (strcmp(kmax, "--smax") == 0) {
              args->smax = atoi(vmax);
            }

          }

        }
        client(args);

      }

    }

  return 0;
}
