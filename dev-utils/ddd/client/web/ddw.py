#!/usr/bin/env python

import sys
from bottle import route, post, request, run, template, static_file

sys.path.append('../common')
from utils import *
from ddd_portmap import DPortmap
from ddd_command import DCommand

''' dds ip address
'''
dds_ip_addr = ''


def ddd_get_port(ipaddr, module):
    portmap_client = DPortmap(ipaddr)
    command_port = portmap_client.get_port(module)
    return command_port


''' css files
'''
@route('/css/<filename>')
def css_file(filename):
    return static_file(filename, root='css/')


''' JS scripts
'''
@route('/js/<filename>')
def js_file(filename):
    return static_file(filename, root='js/')


''' index page
'''
@route('/')
@route('/index')
def index_page():
    global dds_ip_addr
    return template('index', ipaddr=dds_ip_addr)

''' handle connect post
'''
@post('/config')
def dds_config():
    global dds_ip_addr
    dds_ip_addr = request.body.buf
    return "ok"


@route('/command')
def command_page():
    return template('command')


@post('/command/execute')
def execute_command():
    global dds_ip_addr
    command_text   = request.body.buf
    command_port   = ddd_get_port(dds_ip_addr, 'command')
    command_client = DCommand(dds_ip_addr, command_port)
    command_result = command_client.run_command(command_text)

    return command_result


@route('/monitor')
def monitor_page():
    return template('empty')


@route('/terminal')
def terminal_page():
    return template('empty')


@route('/log')
def log_pag():
    return template('empty')


@route('/setting')
def setting_page():
    return template('empty')


@route('/about')
def about_page():
    return template('empty')


if __name__ == '__main__':
    run(host='localhost', port=8080)
