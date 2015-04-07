#!/usr/bin/env python

import socket

PORTMAP_PORT = 36925

class DPortmap(object):

    def __init__(self, dstip):
        self.dstip = dstip

    def get_port(self, name):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(name, (self.dstip, PORTMAP_PORT))
        data, (ip, port) = sock.recvfrom(1024)
        component_port = socket.ntohs(int(data))
        sock.close()

        return component_port
