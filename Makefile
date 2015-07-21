CC=g++
PRE=-std=c++98

all:ftpclient

ftpclient.o: ipk.cpp
	$(CC) $(PRE) -c ipk.cpp -o ftpclient.o

ftpclient:  ftpclient.o
	$(CC) $(PRE) ftpclient.o -o ftpclient
