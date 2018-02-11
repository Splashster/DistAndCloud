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
#include <string>
#include "fileserver.h"

#define PROTOCOL_MESSAGE_SIZE 100
#define FILE_CONTENT_SIZE 1024
#define NUM_SOCKETS_ALLOWED 10



using namespace std;

//Deallocates allocated memory
void dealocate(char* mem){
	delete[] mem;
}


//Gets size of a specified file.
int getFileSize(string fileName){
	int size = 0;
	ifstream theFile (fileName,ios::binary | ios::ate);
	
	theFile.seekg(0, ios::end);
	size = theFile.tellg();
	theFile.close();
	return size;
}

//Connects client to server. Returns 0 if sucessful and 1 if not.
int connect_to_server(char* who, int port, ConnectionInfo* con){
	struct sockaddr_in server_addr;
	struct hostent* hent;
	int result = 0;
	
	if((hent=gethostbyname(who)) == NULL) {
		cerr << "Hostname or ConnectionInfo null";
		result = 1;
		return result;
	}
	
	if((con->cli_socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cerr << "Socket error." << endl;
		result = 1;
		return result;
	}
	
	memset((void *) &server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr = *((struct in_addr *)hent->h_addr);
	server_addr.sin_port = htons(port);
	
	if(connect(con->cli_socket_num, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		result = 1;
		return result;
	}
	return result;
}


