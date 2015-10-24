#!/usr/bin/env python

import socket

class DCommand(object):

    def __init__(self, dstip, dstport):
        self.dstip = dstip
        self.dstport = dstport

    def run_command(self, cmd):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(cmd, (self.dstip, self.dstport))
        data, (ip, port) = sock.recvfrom(1024)
        sock.close()

        return data
