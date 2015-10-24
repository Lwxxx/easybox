#!/usr/bin/env python

import sys
from optparse import OptionParser

sys.path.append('../common')
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
    command_result  = command_client.run_command(command)
    print 'command return: \n\n', command_result, '\n'


def ddd_get_file(ipaddr, remote_name, local_path):
    tftp_port   = ddd_get_port(ipaddr, 'tftp')
    tftp_client = DTftp(ipaddr, tftp_port)
    tftp_client.get(remote_name, local_path)


def ddd_put_file(ipaddr, local_name, remote_path):
    tftp_port   = ddd_get_port(ipaddr, 'tftp')
    tftp_client = DTftp(ipaddr, tftp_port)
    tftp_client.put(local_name, remote_path)


def ddd_run_monitor(ipaddr):
    monitor_port = ddd_get_port(ipaddr, 'monitor')
    monitor_client = DMonitor(ipaddr, monitor_port)
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
        ddd_run_monitor(ipaddr)
    else:
        print 'Invalid action: %s' % (action)
        show_usage()
