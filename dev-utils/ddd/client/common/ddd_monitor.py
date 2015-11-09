#!/usr/bin/env python

import socket
import time


class DMonitor(object):

    cpu_usage      = 0.0
    cpu_lavg_1min  = 0.0
    cpu_lavg_5min  = 0.0
    cpu_lavg_15min = 0.0
    mem_usage      = 0.0
    mem_total      = 0
    mem_used       = 0

    def __init__(self, dst_ip, dst_port):
        self.dst_ip = dst_ip
        self.dst_port = dst_port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        
    def get_cpu_usage(self):
        return self.cpu_usage


    def get_cpu_lavg(self):
        return self.cpu_lavg_1min, self.cpu_lavg_5min, self.cpu_lavg_15min


    def get_mem_usage(self):
        return self.mem_usage


    def get_mem_total(self):
        return self.mem_total

    
    def get_mem_used(self):
        return self.mem_used


    def send_command(self, command):
        self.sock.sendto(command, (self.dst_ip, self.dst_port))


    def recv_data(self):
        while True:
            data = self.sock.recv(1024)
            self._handle_data(data)


    def _handle_data(self, data):
        splited_data = data.split('$')
        if 4 != len(splited_data):
            print 'recv invalid data'

        info_type = splited_data[0]
        info_name = splited_data[1]
        timestamp = splited_data[2]
        info_data = splited_data[3][:-1] # remove '\n'

        if info_type == 'cpu':
            self._handle_cpu_info(info_name, timestamp, info_data)
        elif info_type == 'mem':
            self._handle_mem_info(info_name, timestamp, info_data)
        else:
            pass


    def _handle_cpu_info(self, name, timestamp, data):
        if name == 'total':
            self.cpu_usage = float(data.split(':')[-1])
        elif name == 'loadavg':
            loadavg_data = data.split(',')
            self.cpu_lavg_1min  = float(loadavg_data[0].split(':')[-1])
            self.cpu_lavg_5min  = float(loadavg_data[1].split(':')[-1])
            self.cpu_lavg_15min = float(loadavg_data[2].split(':')[-1])


    def _handle_mem_info(self, name, timestamp, data):
        mem_data = data.split(',')
        if len(mem_data) >= 2:
            self.mem_total = int(mem_data[0].split(':')[-1]) * 1024
            self.mem_used  = int(mem_data[1].split(':')[-1]) * 1024
            self.mem_usage = (float(self.mem_used) * 100) / float(self.mem_total)
        else:
            print 'Invalid memory data!'
