ARG1 = localhost
ARG2 = 10004

all: p1 run 

p1 : client.cpp
	g++ -std=c++11 client.cpp clientfilesystem.cpp -o client

.PHONY run : client
	./client $(ARG1) $(ARG2)

.PHONY:	all
