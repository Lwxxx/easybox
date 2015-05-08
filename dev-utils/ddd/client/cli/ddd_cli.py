#!/usr/bin/env python

import sys
from optparse import OptionParser

sys.path.append('../common')
from ddd_portmap import DPortmap
from ddd_command import DCommand
from ddd_tftp    import DTftp



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


def ddd_get_file(ipaddr, filepath):
    tftp_port   = ddd_get_port(ipaddr, 'tftp')
    tftp_client = DTftp(ipaddr, tftp_port)
    tftp_client.get(filepath)


def ddd_put_file(ipaddr, filepath):
    tftp_port   = ddd_get_port(ipaddr, 'tftp')
    tftp_client = DTftp(ipaddr, tftp_port)
    tftp_client.put(filepath)


def show_usage():
    usage = 'usage: ddc ip_addr <action> <arg>\n\n' \
            'action list: \n\n'                     \
            '  run "xxx" -- run a shell command\n'  \
            '  get xxx   -- get file from server\n' \
            '  put xxx   -- put file to server\n\n'

    print usage
    sys.exit(1)


if __name__ == '__main__':

    if len(sys.argv) != 4:
        show_usage()

    ipaddr = sys.argv[1]
    action = sys.argv[2]
    arg    = sys.argv[3]

    if action == 'run':
        if '"' == arg[0] and '"' == argv[-1]:
            arg = arg[1:-1]
        ddd_run_command(ipaddr, arg)
    elif action == 'put':
        ddd_put_file(ipaddr, arg)
    elif action == 'get':
        ddd_get_file(ipaddr, arg)
    else:
        show_usage()
