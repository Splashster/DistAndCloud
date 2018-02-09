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

//Reads and returns contents of a file
string readFile(string fileName){
	int fileSize = getFileSize(fileName);
	ifstream getContent(fileName, ios::in);
	string theContents = "";
	string temp = "";

	while(getline(getContent,temp)){
		if(!getContent.eof()){
			theContents += temp + '\n';
		}
		else{
			theContents += temp;
		}
	}
	getContent.close();
	return theContents;
}

//Writes given input into a file.
void writeFile(string fileName, char* fileContents){
	ofstream file(fileName, ios::out); 
	char* token, *token_location,*saveptr; 
	int fileSize = 0;
	token = strtok_r (fileContents,":",&saveptr); 
	token = strtok_r (NULL,":",&saveptr);
	fileSize = atoi(token);
	if(fileSize > 0){
			token_location = token = new char[fileSize];
			memset(token, '\0', fileSize);
			token = strtok_r (NULL,"\0",&saveptr); //'\0' is used as a delimeter to ensure that the whole file content can be extracted from the CONT protocol message
			string contents(token);
			file << contents;
			dealocate(token_location);
	}
	file.close();	
}

//Generates protocol message that will be sent.
void gernerateProtocolMessage(char* msg, string protocolMessage, int size){
	memset(msg, '\0', size);
	memcpy(msg,protocolMessage.c_str(),protocolMessage.length());
}

//Sends message to client. Prints out message on server side if send was unsucessful not or if one of the parameters is NULL.
void serverSendMessage(int server_socket, char* msg){
	int msgSize = 0;

	if(msg == NULL){
		cerr << "Message null" << endl;
	}
	if((msgSize = send(server_socket, msg, strlen(msg)+1, 0)) < 0){
		cerr << "Send error." << endl;
	}
}

//Receives message from client. Returns message received if sucessful or NULL if not.
char* serverReceiveMessage(int server_socket, int size){
	int msgSize = 0;
	int totalSize = 0;
	char* temp = new char[size];
	memset(temp, '\0', size);
	string wrapper = "";
	do{
		if((msgSize = recv(server_socket, temp, size, 0)) < 0){
			cerr << "Receive error." << endl;
			dealocate(temp);
			exit;
		}
		if(temp[msgSize-1] != '\0'){
			temp[msgSize] = '\0';
			wrapper += temp;
			totalSize += msgSize;
		}
		else if(temp[msgSize-1] == '\0' && wrapper != ""){
				temp[msgSize] = '\0';
				wrapper += temp;
				totalSize += msgSize;
				char* msg = new char[totalSize];
				memset(msg, '\0', totalSize);
				memcpy(msg, wrapper.c_str(), totalSize);
				dealocate(temp);
				return msg;
		}
		else{
			return temp;
		}
	}while(temp[msgSize-1] != '\0');
}

//Handles connection between client and server. Also performs all the work of receiving (STOR) and transferring (RTRV) files from the client.
void handleConnection(int clisock){
	while(1){
		int fileSize = 0;
		string fileNme = "";
		char* protMsg = new char[PROTOCOL_MESSAGE_SIZE];
		
		string protocolMessage(serverReceiveMessage(clisock,PROTOCOL_MESSAGE_SIZE));
		if(protocolMessage.empty())
		{
				close(clisock);
				dealocate(protMsg);
				break;
		}
		else if(protocolMessage.substr(0,4) == "QUIT"){
				close(clisock);
				dealocate(protMsg);
				break;
		}
		else
		{
			fileNme = protocolMessage.substr(5);
			if(protocolMessage.substr(0,4) == "STOR"){
				
				if(ifstream(fileNme)){
					gernerateProtocolMessage(protMsg,"ERR:991 Cannot store file " + fileNme + " because it already exists on remote machine!",PROTOCOL_MESSAGE_SIZE);
					serverSendMessage(clisock, protMsg);
				}
				else{
						gernerateProtocolMessage(protMsg,"CTS:" + fileNme,PROTOCOL_MESSAGE_SIZE);
						serverSendMessage(clisock, protMsg);
						char* temp = serverReceiveMessage(clisock,FILE_CONTENT_SIZE);
						char* contents = new char[strlen(temp)+1];
						memset(contents, '\0', strlen(temp)+1);
						memcpy(contents,temp, strlen(temp));
						writeFile(fileNme,contents);
						dealocate(contents);
					}
				
			}
			else if(protocolMessage.substr(0,4) == "RTRV"){
					if(!ifstream(fileNme)){
							gernerateProtocolMessage(protMsg,"ERR:913 Cannot pull requested file " + fileNme + " from remote machine because it does not exist!",PROTOCOL_MESSAGE_SIZE);
							serverSendMessage(clisock,protMsg);
					}
					else{
							int protocolMessageSize = 0;
							gernerateProtocolMessage(protMsg,"RTS:" + fileNme,PROTOCOL_MESSAGE_SIZE);
							serverSendMessage(clisock,protMsg);
							
							string contentProtocol = "",theContents = "";
							protocolMessage = "CONT:";
							fileSize = getFileSize(fileNme);
							theContents = readFile(fileNme);

							contentProtocol = protocolMessage + to_string(fileSize) + ":" + theContents;
							protocolMessageSize = fileSize + protocolMessage.length()+1 + to_string(fileSize).length()+1;
							char* fileContents = new char[protocolMessageSize];
							gernerateProtocolMessage(fileContents,contentProtocol,protocolMessageSize);
							serverSendMessage(clisock,fileContents);
							dealocate(fileContents);
					}
			}
		}
			dealocate(protMsg);
			
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
		cerr << "Listening to client " << client_num << " on port number " << to_string(port) << endl;
      	client_num++;
	    thread handled(*handleConnection, newsock_con);
		handled.detach();
	}
	return result;
}
