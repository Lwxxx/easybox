#!/usr/bin/env python

import socket
import time

class DMonitor(object):

    def __init__(self, dst_ip, dst_port):
        self.dst_ip = dst_ip
        self.dst_port = dst_port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

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
            self._show_cpu_info(info_name, timestamp, info_data)
        elif info_type == 'mem':
            self._show_mem_info(info_name, timestamp, info_data)
        else:
            pass


    def _show_cpu_info(self, name, timestamp, data):
        time_str = time.ctime(float(timestamp))
        cpu_usage = data.split(':')[-1]
        print '%s | CPU | %6s | %6s%%' % (time_str, name, cpu_usage)


    def _KB_to_pretty_str(self, data):
        if data > 1024:
            data = float(data) / 1024
            if data > 1024:
                data = float(data) / 1024
                pretty_str = "%.2f GB" % (data)
            else:
                pretty_str = "%.2f MB" % (data)
        else:
            pretty_str = "%.2f KB" % (data)

        return pretty_str


    def _show_mem_info(self, name, timestamp, data):
        time_str = time.ctime(float(timestamp))
        mem_data = data.split(',')
        if len(mem_data) >= 2:
            mem_total = mem_data[0].split(':')[-1]
            mem_used  = mem_data[1].split(':')[-1]
            mem_total_str = self._KB_to_pretty_str(int(mem_total))
            mem_used_str = self._KB_to_pretty_str(int(mem_used))
            used_ratio = (float(mem_used) * 100) / float(mem_total)
            print '%s | Mem |  usage | %s/%s [%.2f%%]' % \
                  (time_str, mem_used_str, mem_total_str, used_ratio)
        else:
            print 'Invalid memory data!'
