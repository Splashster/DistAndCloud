ARG1 = 10003

all: p1 run 

p1 : server.cpp
	g++ -std=c++11 server.cpp serverfilesystem.cpp -o server -lpthread

.PHONY run : server
	./server $(ARG1)

.PHONY:	all
