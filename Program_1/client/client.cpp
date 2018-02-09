#include "fileserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <cctype>
#include <unistd.h>

using namespace std;



int main(int argc, char* argv[]){
  if( argc < 3){
    cout << "Missing server name or server port number command line argument!" << endl;
    return 0;
  }

  int port = atoi(argv[2]);
  int retries = 100;
  char* who = argv[1];

  ConnectionInfo info;
  if(connect_to_server( who, port, &info)){
    cout << "Failed to connect to server using port number " + to_string(port) + "!" << endl;
    cout << "Retrying from a different port" << endl;
    while(retries--)
    {
       port++;
       if(!connect_to_server(who, port, &info)){
          cout << "Connected to server using port number " + to_string(port) + "!" << endl;
          break;
       }
    }
    if(retries == 0){
      cout << "Unable to connect to server!" << endl;
      return 0;
    }
  }
  
  while( 1 ){
      string command = " ", fileName = " ";
      char space;
      getline(cin,command);
      space = command[3];

  	  while((strcasecmp(command.substr(0,4).c_str(), "quit") != 0 && strcasecmp(command.substr(0,3).c_str(), "put") != 0 && strcasecmp(command.substr(0,3).c_str(), "get") != 0) || (strcasecmp(command.substr(0,4).c_str(), "quit") != 0 && !isspace(space))) {
        		cout << "Invalid command try again"<< endl;
        		getline(cin,command);
            space = command[3];
    	}
  	
    	if(strcasecmp(command.substr(0,3).c_str(), "put") == 0){
    		fileName = command.substr(4);
    		put(&info,fileName);
    	}
  	  else if(strcasecmp(command.substr(0,3).c_str(), "get") == 0){
  		fileName = command.substr(4);
      get(&info,fileName);
  	  }
      else{
  		      close_socket(&info);
  		      return 0;
          }
    }
}
