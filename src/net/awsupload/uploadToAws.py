#!/usr/bin/env python
import time
import socket
import sys

serverAddressPort   = ("localhost", 51038)

CHUNK_SIZE = 1024

path = sys.argv[1]

UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
UDPClientSocket.connect(serverAddressPort)

start =  time.time()

with open(path, 'rb') as f:
	data = f.read(CHUNK_SIZE)
	while data:
	    UDPClientSocket.sendto(data, serverAddressPort)
	    print('Sent '+ str(CHUNK_SIZE))
	    data = f.read(CHUNK_SIZE)
	UDPClientSocket.close()

end =  time.time()
print( end - start)