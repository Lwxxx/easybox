#!/usr/bin/env python

import socket

class DCommand(object):

    def __init__(self, dstip, dstport):
        self.dstip = dstip
        self.dstport = dstport

    def run_command(self, cmd, callback):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(cmd, (self.dstip, self.dstport))

        while True:
            data = sock.recv(1024)
            if 0 == len(data):
                break

            callback(data)

        sock.close()
        return data
