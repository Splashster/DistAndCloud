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
 while(1){}

}  
