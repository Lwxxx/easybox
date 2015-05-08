#!/usr/bin/env python

import os, sys
import socket
import struct


TFTP_OP_RRQ = 1
TFTP_OP_WRQ = 2
TFTP_OP_DAT = 3
TFTP_OP_ACK = 4
TFTP_OP_ERR = 5

TFTP_FMT_RRQ = 'H%ds%ds' # opcode | filename | mode
TFTP_FMT_WRQ = 'H%ds%ds' # opcode | filename | mode
TFTP_FMT_DAT = 'HH%ds'   # opcode | block no | data
TFTP_FMT_ACK = 'HH'      # opcode | block no
TFTP_FMT_ERR = 'HH%ds'   # opcode | error no | error msg

TFTP_MODE_ASCII = 'ascii'
TFTP_MODE_OCTET = 'octet'
TFTP_MODE_MAIL  = 'mail'

TFTP_ERR_NOTDEFINE    = 0
TFTP_ERR_FILENOTFOUND = 1
TFTP_ERR_ACCESVILOA   = 2
TFTP_ERR_DISKFULL     = 3
TFTP_ERR_ILEGALOP     = 4
TFTP_ERR_UNKNOWNID    = 5
TFTP_ERR_FILEEXIST    = 6
TFTP_ERR_NOUSER       = 7

TFTP_ERROR = \
[
    'Not defined',
    'File not found',
    'Access violation',
    'Disk full or allocation exceeded',
    'Illegal TFTP operation',
    'Unknown transfer ID',
    'File already exists',
    'No such user'
]


class DTftp(object):

    ''' construct method
    '''
    def __init__(self, ip_addr, port):
        self.server_addr = (ip_addr, port)
        self.sock = None
        self.block_number = 0
        self.end_block = -1
        self.read_file = None
        self.write_file = None

    ''' init socket
    '''
    def _connect(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        except Exception, e:
            print 'ERROR: create socket failed ', e
            sys.exit(1)


    ''' close socket
    '''
    def _disconnect(self):
        if self.sock is not None:
            self.sock.close()


    ''' send a tftp packet and handle reply
    '''
    def _send_packet(self, packet):
        if self.sock is None:
            print 'ERROR: invalid socket'

        try:
            self.sock.sendto(packet, self.server_addr)
        except Exception, e:
            print 'ERROR: send data failed, ', e

    ''' send error packet
    '''
    def _send_error(self):
        pass


    ''' handle tftp error packet
    '''
    def _handle_error(self, data):
        data_len = len(data)
        err_template = TFTP_FMT_ERR % (data_len - 4)
        opcode, err_code, err_msg = struct.unpack(err_template, data)
        err_code = socket.ntohs(int(err_code))

        print 'recv error packet: [%d] %s' % (err_code, err_msg)


    ''' handle tftp ack packet
    '''
    def _handle_ack(self, data):
        ack_template = TFTP_FMT_ACK
        opcode, block = struct.unpack(ack_template, data)

        block = socket.ntohs(int(block))
        self.block_number = block + 1
        if (self.block_number == self.end_block):
            return False

        try:
            send_data = self.read_file.read(512)
        except Exception, e:
            print 'read from file failed', e
            return False

        data_template = TFTP_FMT_DAT % len(send_data)
        data_packet = struct.pack(data_template, socket.htons(TFTP_OP_DAT),
                                  socket.htons(self.block_number), send_data)
        self._send_packet(data_packet)

        # final block
        if len(send_data) != 512:
            self.end_block = self.block_number
            return False

        return True


    ''' handle tftp data packet
    '''
    def _handle_data(self, data):
        packet_len = len(data)
        data_len = packet_len - 4
        data_template = TFTP_FMT_DAT % (data_len)
        opcode, block, data = struct.unpack(data_template, data)

        block = socket.ntohs(int(block))
        self.block_number = block

        # write file
        try:
            self.write_file.write(data)
        except Exception, e:
            print 'write file failed'
            self._send_error(TFTP_ERR_NOTDEFINE)
            return False

        # send ack
        ack_template = TFTP_FMT_ACK
        ack_packet = struct.pack(ack_template, socket.htons(TFTP_OP_ACK),\
                                 socket.htons(self.block_number))
        self._send_packet(ack_packet)

        # final part
        if data_len < 512:
            self.write_file.close()
            return False

        return True


    ''' handle reply packet
    '''
    def _handle_packet(self):
        try:
            data,addr = self.sock.recvfrom(1024)
        except Exception, e:
            print 'recv data failed: ', e
            return False

        if len(data) < 4: # minial packet is ACK, 4 bytes
            print 'recv a invalid packed: ', data
            return False

        opcode, = struct.unpack('h', data[:2])
        opcode  = socket.ntohs(int(opcode))

        if opcode == TFTP_OP_DAT:
            return self._handle_data(data)
        elif opcode == TFTP_OP_ACK:
            return self._handle_ack(data)
        elif opcode == TFTP_OP_ERR:
            self._handle_error(data)
            return False
        else:
            print 'unsurport opcode: ', opcode

        return True


    ''' get file from tftp server
    '''
    def get(self, file_path):
        self.block_number = 0
        try:
            self.write_file = open(file_path, 'wb')
        except Exception, e:
            print 'ERROR: create file %s failed, ' % (file_path), e
            return

        # only support binary mode (octet)
        rrq_template = TFTP_FMT_RRQ % (len(file_path)+1, len(TFTP_MODE_OCTET)+1)
        rrq_packet = struct.pack(rrq_template, socket.htons(TFTP_OP_RRQ), \
                                 file_path, TFTP_MODE_OCTET)
        self._connect()
        self._send_packet(rrq_packet)
        while self._handle_packet():pass
        self._disconnect()


    ''' put file to tftp server
    '''
    def put(self, file_path):
        self.block_number = 0
        self.end_block    = -1
        try:
            self.read_file = open(file_path, 'rb')
        except Exception, e:
            print 'ERROR: read file: %s failed, ' % (file_path), e
            return

        wrq_template = TFTP_FMT_WRQ % (len(file_path)+1, len(TFTP_MODE_OCTET)+1)
        wrq_packet = struct.pack(wrq_template, socket.htons(TFTP_OP_WRQ), \
                                 file_path, TFTP_MODE_OCTET)

        self._connect()
        self._send_packet(wrq_packet)
        while self._handle_packet():pass
        self._disconnect()

# Test
if __name__ == '__main__':
    # tftp xxx.xxx.xxx.xxx [put|get] xxxx
    tftpc = DTftp('127.0.0.1', 65437)

    # test get
    tftpc.get("test_get.dat")

    # test put
    tftpc.put("test_put.dat")
