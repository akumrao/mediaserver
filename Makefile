all:
	g++ -g main.cpp build/libsocketio.a -I./client/src/signal/include/ -lpthread -ldl -lssl -lcrypto -o runSokeioClient
