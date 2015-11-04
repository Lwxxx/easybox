#!/usr/bin/env python

import os, sys
import time
import thread
from optparse import OptionParser

sys.path.append('../common')
from utils import *
from ddd_portmap import DPortmap
from ddd_command import DCommand
from ddd_tftp    import DTftp
from ddd_monitor import DMonitor


def ddd_get_port(ipaddr, module):
    portmap_client = DPortmap(ipaddr)
    command_port = portmap_client.get_port(module)
    print 'port of %s is %d' % ('command', command_port)
    return command_port


def ddd_run_command(ipaddr, command):
    command_port   = ddd_get_port(ipaddr, 'command')
    command_client = DCommand(ipaddr, command_port)

    def _show_output(data):
        print data,
        sys.stdout.flush()

    print 'command return: \n'
    command_client.run_command(command, _show_output)


def ddd_get_file(ipaddr, remote_name, local_path):
    tftp_port   = ddd_get_port(ipaddr, 'tftp')
    tftp_client = DTftp(ipaddr, tftp_port)
    tftp_client.get(remote_name, local_path)


def ddd_put_file(ipaddr, local_name, remote_path):
    tftp_port   = ddd_get_port(ipaddr, 'tftp')
    tftp_client = DTftp(ipaddr, tftp_port)
    tftp_client.put(local_name, remote_path)


def ddd_run_monitor(ipaddr, flag):
    monitor_port = ddd_get_port(ipaddr, 'monitor')
    monitor_client = DMonitor(ipaddr, monitor_port)

    def _show_monitor_data(flag):
        MIN_WIDTH_REQUIRED = 40

        print ''
        while True:
            # get current terminal size
            term_width, term_height = get_terminal_size()
            if term_width < MIN_WIDTH_REQUIRED:
                print 'terminal width (%d) is too small' % (term_width)
                return

            # get current monitor data
            cpu_usage = monitor_client.get_cpu_usage()
            mem_usage = monitor_client.get_mem_usage()

            # show current data
            cpu_pb_width = (term_width / 2) - 16                  # progress bar
            cpu_vl_width = int((cpu_usage/100.0) * cpu_pb_width)  # vertical line
            cpu_sp_width = cpu_pb_width - cpu_vl_width            # space
            cpu_usage_str = 'CPU [%s%s] %6.2f%%' % ('|' * cpu_vl_width,
                            ' ' * cpu_sp_width, cpu_usage)

            mem_pb_width = (term_width / 2) - 16                  # progress bar
            mem_vl_width = int((mem_usage/100.0) * mem_pb_width)  # vertical line
            mem_sp_width = mem_pb_width - mem_vl_width            # space
            mem_usage_str = 'MEM [%s%s] %6.2f%%' % ('|' * mem_vl_width,
                            ' ' * mem_sp_width, mem_usage)

            monitor_line =  cpu_usage_str + '   ' + mem_usage_str
            print monitor_line,
            if '--oneline' == flag:
                sys.stdout.flush()
            else:
                print ''

            time.sleep(1)
            if '--oneline' == flag:
                print '\r',


    # start a new thread to show data
    thread.start_new_thread(_show_monitor_data, (flag,))

    monitor_client.send_command('start')
    monitor_client.recv_data()


def show_usage():
    usage = 'usage: ddc ip_addr <action> <param>\n\n'                 \
            'action list: \n\n'                                       \
            '  run "xxx"                   -- run a shell command\n'  \
            '  get REMOTE_FILE LOCAL_PATH  -- get file from server\n' \
            '  put LOCAL_FILE REMOTE_PATH  -- put file to server\n\n'

    print usage
    sys.exit(1)


if __name__ == '__main__':

    if len(sys.argv) < 3:
        show_usage()

    ipaddr = sys.argv[1]
    action = sys.argv[2]

    if action == 'run':
        cmd = sys.argv[3]
        if '"' == cmd[0] and '"' == cmd[-1]:
            cmd = cmd[1:-1]
        ddd_run_command(ipaddr, cmd)
    elif action == 'put':
        if (5 != len(sys.argv)):
            show_usage()
        ddd_put_file(ipaddr, sys.argv[3], sys.argv[4])
    elif action == 'get':
        if (5 != len(sys.argv)):
            show_usage()
        ddd_get_file(ipaddr, sys.argv[3], sys.argv[4])
    elif action == 'monitor':
        if (4 <= len(sys.argv)):
            monitor_flag = sys.argv[3]
        else:
            monitor_flag = ''
        ddd_run_monitor(ipaddr, monitor_flag)
    else:
        print 'Invalid action: %s' % (action)
        show_usage()
