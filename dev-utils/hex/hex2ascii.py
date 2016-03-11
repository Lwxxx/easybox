#!/usr/bin/env python

import sys
import binascii


'''
Usage:

lwx@linux:~$ hex2ascii 68 65 6c 6c 6f 20 77 6f 72 6c 64 21
lwx@linux:~$ HEX ==> ASCII: hello world!
'''

def hex2acsii(hex_list):
    ascii_list = [ binascii.a2b_hex(x) for x in hex_list ]
    return ''.join(ascii_list)

if __name__ == '__main__':
    hex_list = sys.argv[1:]
    print "HEX ==> ASCII: %s" % (hex2acsii(hex_list))
