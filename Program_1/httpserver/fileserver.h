#include <string>

struct ConnectionInfo{
		int cli_socket_num;
		//char* the_msg;
};

int connect_to_server(char* who, int port, ConnectionInfo* con);
int run_server(int port);
void close_socket(ConnectionInfo *con);
