#include "fileserver.h"
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char* argv[]){
  if(argc < 2)
  {
    cout << "Missing server port number!" << endl;
    return 0;
  }
  
  int port = atoi(argv[1]);
  int retries = 100;

  while(port <= 10000){
    cout << "Sever port number must be greater than 10000. Try again!" << endl;
    cin >> port;
  }

 

  while( retries -- ){
    if( ! run_server( port ) ){} 
    else {
      port ++; 
      cout << " Changing server port number to " + to_string(port) + "!" << endl; }
    }

  cout << "Failed to start server" << endl;

  return 1;
}
