#!/usr/bin/env python

import sys
import thread
from bottle import route, post, request, run, template, static_file

sys.path.append('../common')
from utils import *
from ddd_portmap import DPortmap
from ddd_command import DCommand
from ddd_monitor import DMonitor

#+TODO: do not use global variable
dds_ip_addr = ''
monitor_client = None


''' portmap wrapper
'''
def ddd_get_port(ipaddr, module):
    portmap_client = DPortmap(ipaddr)
    command_port = portmap_client.get_port(module)
    return command_port


''' creata a monitor client
'''
def create_monitor_client():
    global monitor_client

    if '' == dds_ip_addr:
        return None

    if monitor_client is None:
        monitor_port   = ddd_get_port(dds_ip_addr, 'monitor')
        monitor_client = DMonitor(dds_ip_addr, monitor_port)
        thread.start_new_thread(monitor_client.recv_data, ())

    return monitor_client


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
@post('/dds')
def dds_connect():
    global dds_ip_addr
    dds_ip_addr = request.body.buf
    return "ok"


''' return current monitor data
'''
@post('/monitor/data')
def current_monitor_data():
    cpu_usage = 0.0
    mem_usage = 0.0
    monitor = create_monitor_client()

    if monitor is not None:
        cpu_usage = monitor.get_cpu_usage()
        mem_usage = monitor.get_mem_usage()

    response_json = '{"cpu":%.2f, "mem":%.2f}' % (cpu_usage, mem_usage)
    return response_json


''' start monitor
'''
@post('/monitor/start')
def start_monitor():
    monitor = create_monitor_client()

    if monitor is not None:
        monitor.send_command('start')

    return "ok"


''' stop monitor
'''
@post('/monitor/stop')
def stop_monitor():
    monitor = create_monitor_client()

    if monitor is not None:
        monitor.send_command('stop')

    return "ok"


if __name__ == '__main__':
    run(host='localhost', port=8080)
