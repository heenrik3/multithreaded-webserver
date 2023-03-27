CC = gcc
CFLAGS = -Wall

all: server

server: server/main.c
	$(CC) $(CFLAGS) -o server/server server/main.c -lws2_32 -pthread

runServer:
	./server/server.exe

clean:
	del server\server.exe