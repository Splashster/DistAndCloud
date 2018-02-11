#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <thread> 
#include <fstream>
#include "fileserver.h"
#include <cstdint>

#define PROTOCOL_MESSAGE_SIZE 100
#define MAX_BUFFER_SIZE 1024
#define NUM_SOCKETS_ALLOWED 10



using namespace std;

//Deallocates allocated memory
void dealocate(char* mem){
	delete[] mem;
}

int readSock(int clisock){
	char buff[MAX_BUFFER_SIZE];
	int msgSize;
	FILE *fp = fdopen(clisock, "r");
	fprintf(fp, "%s");	

	}


void* connectHandler(void* args){
	int clisock = (intptr_t) args;
	while(readSock(clisock));
	close(clisock);
	pthread_exit(NULL);
}




//Handles connection between client and server. Also performs all the work of receiving (STOR) and transferring (RTRV) files from the client.
void handleConnection(int clisock){
	while(1){
		pthread_attr_t attribs;
		pthread_t thread;
		pthread_attr_init(&attribs);
		pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
		pthread_create(&thread, &attribs, connectHandler, (void*)clisock);
	}	
}

	

//Setups and starts the server. Returns 0 if sucessful and 1 if not.
int run_server(int port){
	struct sockaddr_in server_addr, cli_addr;
	unsigned int clilen;
	int result = 0;
	int client_num = 0;
	int newsock_con,serv_socket_num;
	
	if((serv_socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		result = 1;
		return result;
	}
	
	memset((void *) &server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	
	if(bind(serv_socket_num, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		cerr << "Bind error.";
		result = 1;
		return result;
	}
	
	listen(serv_socket_num, NUM_SOCKETS_ALLOWED);
	while(1){
		clilen = sizeof(cli_addr);
		newsock_con = accept(serv_socket_num, (struct sockaddr *) &cli_addr, &clilen);
		if(newsock_con < 0) {
			cerr << "Accept error." << endl;
			result = 1;
			return result;
		}
		cerr << "Listening to client " << to_string(client_num) << " on port number " << port << endl;
      	client_num++;
		handleConnection(newsock_con);
	}
	return result;
}
