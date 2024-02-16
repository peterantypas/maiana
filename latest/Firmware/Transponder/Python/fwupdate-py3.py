#!/usr/bin/env python3

import serial
import sys
import os
import time
import struct
import binascii

port = None
file = None
filesize = 0
crc32 = 0

def is_unit_running():
    print("Checking if unit is running")
    s = port.readline().strip().decode('utf-8')
    if len(s) > 0:
        #print s
        tokens = s.split(',')
        if len(tokens) > 3:
            return True        
    return False

def drain_port():
    s = port.readline()
    while len(s) > 0:
        s = port.readline()

def enable_dfu():
    if is_unit_running():
        print("Unit is running, switching to DFU mode")
        port.write(b'dfu\r\n')
        drain_port()

    for x in range(5):
        port.write(b'\r\n')
        s = port.readline().strip()
        if s.find(b"MAIANA bootloader") >= 0:
            return True

    return False

    

def begin_transfer():
    command = "load {0} {1:x}\r\n".format(filesize, crc32).encode('utf-8')
    port.write(command)

    s = port.readline().strip()
    return s == b"READY"


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: {0} port imagefile".format(sys.argv[0]))
        sys.exit(1)

    portname = sys.argv[1]
    filename = sys.argv[2]

    port = serial.Serial(portname, 38400, timeout=2)
    if not port.is_open:
        print("Unable to open serial port")
        sys.exit(2)

    file = None

    try:
        file = open(filename, "rb")
    except:
        print("Unable to open file {0}".format(filename))
        sys.exit(2)

    filesize = os.stat(filename).st_size
    #print filesize
    data = file.read()
    crc32 = (binascii.crc32(data) & 0xFFFFFFFF)
    file.seek(0)

    print("File size: {0}, CRC32: 0x{1:x}".format(filesize, crc32))


    if not enable_dfu():
        print ("Could not get unit into DFU mode")
        sys.exit(3)

    print("Unit is in DFU mode")

    
    if not begin_transfer():
        print("Unable to begin transfer, restart the unit and retry")
        sys.exit(3)

    print("Starting transfer")
    
    bytes = file.read(2048)
    resp = ''
    while len(bytes) > 0:
        port.write(bytes)
        #print "Sent {0} bytes".format(len(bytes))
        sys.stdout.write('.')
        sys.stdout.flush()
        resp = port.readline().strip()
        if resp.find(b"OK") < 0:
            break
        #print s
        bytes = file.read(2048)

    print()
    print(resp.decode('utf-8'))
    
        
    
