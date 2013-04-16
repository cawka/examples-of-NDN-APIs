all: apps

apps: client server

CXXFLAGS=-I/opt/local/include -I/usr/local/include/ccnx-cpp -g3 -O0
LDFLAGS=-lccn -lcrypto -lssl -lccnx-cpp -lboost_system-mt -levent -lboost_thread-mt -L/opt/local/lib -L/usr/local/lib

client: client.o
	$(CXX) -o client client.o $(LDFLAGS)

server: server.o
	$(CXX) -o server server.o $(LDFLAGS)

