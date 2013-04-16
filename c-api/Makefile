all: apps

apps: client server

CFLAGS=-std=gnu99 -I/opt/local/include -g3 -O0
LDFLAGS=-std=gnu99 -lccn -lcrypto -lssl -L/opt/local/lib

client: client.o
	$(CC) -o client client.o $(LDFLAGS)

server: server.o
	$(CC) -o server server.o $(LDFLAGS)

