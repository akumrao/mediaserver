#!/usr/bin/env python
import time
import socket
import sys

#serverAddressPort   = ("18.228.58.178", 51038)

port = 51038
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('', port))
print 'waiting on port:', port
while 1:
	data, addr = s.recvfrom(1024)
	print data

