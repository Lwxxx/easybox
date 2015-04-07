#!/usr/bin/env python

import sys

if __name__ == '__main__':
    sys.path.append("../common")
    from ddd_portmap import DPortmap
    from ddd_command import DCommand

    portmap_client = DPortmap('127.0.0.1')
    command_port = portmap_client.get_port("command")
    print "port of %s is %d" % ("command", command_port)

    command_client = DCommand('127.0.0.1', command_port)
    result = command_client.run_command("ls -l")
    print "command return: \n\n", result, "\n"
