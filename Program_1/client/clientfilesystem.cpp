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

//Sends message to server. Returns 0 if sucessful and 1 if not.
void sendMessage(ConnectionInfo *con, char *msg){
	int result = 0;
	int msgSize = 0;
	
	if(msg == NULL){
		cerr << "Message null" << endl;
		exit;
	}
	else if(con == NULL){
		cerr << "ConnectionInfo object null" << endl;
		exit;
	}

	if((msgSize = send(con->cli_socket_num, msg, strlen(msg)+1, 0)) < 0) {
		cerr << "Send error." << endl;
		exit;
	}
}

//Receives message from server. Returns received message if sucessful or NULL if not.
char* receiveMessage(ConnectionInfo* con, int size){
	int msgSize = 0;
	int totalSize = 0;
	char* temp = new char[size];
	memset(temp, '\0', size);
	string wrapper = "";
	do{
		if((msgSize = recv(con->cli_socket_num, temp, size, 0)) < 0){
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


//Transfers file from local machine (client) to remote machine (server). 
//First checks to see if file exists on the local machine. 
//If file does not exist, error message appears on client screen and client must try another file. 
//If file exist it attempts to transfer the file to the remote machine.
void put(ConnectionInfo *con, string file){
	if(!ifstream(file)){
		cerr << "ERR:901 Cannot transfer file " + file + " because it does not exist on local machine!" << endl;
	}
	else{
		char* protMsg = new char[file.length()+6];
		char* message;
		int fileSize = 0;
		int protocolMessageSize = 0;
		
		gernerateProtocolMessage(protMsg,"STOR:" + file,file.length()+6);
		sendMessage(con,protMsg);
		protocolMessageSize = file.length()+6;
		message = receiveMessage(con,protocolMessageSize);
		string protocolMessage(message);

		if(protocolMessage.substr(0,3) == "CTS"){
			
			string contentProtocol = "",theContents = "";
			protocolMessage = "CONT:";

			fileSize = getFileSize(file);
			theContents = readFile(file);

			contentProtocol = protocolMessage + to_string(fileSize) + ":" + theContents;
			protocolMessageSize = fileSize + protocolMessage.length()+1 + to_string(fileSize).length()+1;
			char* the_message = new char[protocolMessageSize];
			gernerateProtocolMessage(the_message,contentProtocol,protocolMessageSize);
			sendMessage(con,the_message);
			dealocate(the_message);
		}
		else if(protocolMessage.substr(0,3) == "ERR"){
			cerr << protocolMessage << endl;
		}
		dealocate(protMsg);
	}
	
}

//Retrieve file from remote machine (server) to local machine (client). 
//First checks to see if file exists on the remote machine. 
//If file does not exist, error message appears on client screen and client must try another file. 
//If file exist it attempts to transfer the file to the local machine.
void get(ConnectionInfo *con, string file){
	if(ifstream(file)){
		cerr << "ERR:929 Cannot store file " + file + " because it already exists on local machine!" << endl;
	}
	else{
		char* protMsg = new char[file.length()+6];
		char* message;
		gernerateProtocolMessage(protMsg,"RTRV:" + file,file.length()+6);
		sendMessage(con,protMsg);
		message = receiveMessage(con, PROTOCOL_MESSAGE_SIZE);
		string protocolMessage(message);
			
		if(protocolMessage.substr(0,3) == "RTS"){
			char* fileContents = receiveMessage(con,FILE_CONTENT_SIZE);
			char* contents = new char[strlen(fileContents)+1];
			memset(contents, '\0', strlen(fileContents)+1);
			memcpy(contents,fileContents, strlen(fileContents));
			writeFile(file,contents);
			dealocate(contents);
		}
		else if(protocolMessage.substr(0,3) == "ERR"){
			cerr << protocolMessage << endl;
		}
		dealocate(protMsg);
	}
}

//Closes connection between client and server. Then closes client program.
void close_socket(ConnectionInfo *con){
	char* the_message = new char[5];
	gernerateProtocolMessage(the_message, "QUIT",5);
	sendMessage(con,the_message);
	dealocate(the_message);
	close(con->cli_socket_num);
}