#!/usr/bin/env python
# -*- coding: utf-8 -*-

import socket

for i in range(10000):
    sock = socket.socket()
    sock.connect(('localhost', 8080))
    sock.send(b'Hello world!\n')
    sock.close()

sock = socket.socket()
sock.connect(('localhost', 8080))
sock.send(b'test')

data = sock.recv(1024)
sock.close()

print(data)
