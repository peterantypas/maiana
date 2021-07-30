#!/usr/bin/env python

import serial
import sys
import time
import os

port = None

ACK  = 0x79
NACK = 0x1F
GET  = 0x00

PAGE_SIZE       = 2048
FLASH_BASE      = 0x08000000
BAUD_RATE       = 115200
MAX_IMAGE_SIZE  = 126*1024

# These defaults will be overwritten with results of GET command
GET_VER_CMD             = 0x01
GET_ID_CMD              = 0x02
READ_MEM_CMD            = 0x11
GO_CMD                  = 0x21
WRITE_MEM_CMD           = 0x31
ERASE_CMD               = 0x43          # This may me 0x44 in some chips
WRITE_PROT_CMD          = 0x63
WRITE_UNPROT_CMD        = 0x73
READ_PROT_CMD           = 0x82
READ_UNPROT_CMD         = 0x92

def print_packet(d):
    for c in d:
        sys.stdout.write("0x{:02x} ".format(c))
    sys.stdout.write('\n')

def configure_commands(data):
    global GET_VER_CMD
    global GET_ID_CMD
    global READ_MEM_CMD
    global GO_CMD
    global WRITE_MEM_CMD
    global ERASE_CMD
    global WRITE_PROT_CMD
    global WRITE_UNPROT_CMD
    global READ_PROT_CMD
    global READ_UNPROT_CMD
    
    GET_VER_CMD         = data[0]
    GET_ID_CMD          = data[1]
    READ_MEM_CMD        = data[2]
    GO_CMD              = data[3]
    WRITE_MEM_CMD       = data[4]
    ERASE_CMD           = data[5]
    WRITE_PROT_CMD      = data[6]
    WRITE_UNPROT_CMD    = data[7]
    READ_PROT_CMD       = data[8]
    READ_UNPROT_CMD     = data[9]

    """
    s = "Commands: "
    s += "".join(["0x%x "%d for d in data])
    print s
    """
    
    
def packet_checksum(p):
    x = 0
    for b in p:
        x ^= b

    return x
    
def read_byte():
    r = port.read(1)
    if len(r) > 0:
        return (True, ord(r[0]))
    else:
        return (False, 0)

def drain():
    keepreading = True
    while True:
        (r, keepreading) = read_byte()
        if r == False:
            break
    
def do_handshake():
    port.write([0x7f])
    (success, b) = read_byte()
    if not success:
        return False

    return b == ACK

def complement(cmd):
    c = ~cmd
    if c < 0:
        c += 256
    return c

def send_command(cmd):
    packet = [cmd, complement(cmd)]
    #print packet
    port.write(packet)
    (success, r) = read_byte()
    #print r
    if not success:
        print "Failed to send command 0x{0:2x}".format(cmd)
        return False
    
    if r != ACK:
        print "Got NACK for command 0x{0:2x}".format(cmd)
        return False

    #print "Got ACK"
    return True
    

def send_data(packet):
    port.write(packet)    
    (success, b) = read_byte()
    if not success:
        print "No ACK or NACK for data packet"
        return False

    if b == ACK:
        return True
    else:
        print "Got NACK for data packet"
        return False


def send_get():
    if not send_command(GET):
        print "Failed to send command GET"
        return (False, [])

    (success, bytes) = read_byte()

    if success:
        bytes += 1
    else:
        return (False, [])

    data = port.read(bytes)
    if len(data) < bytes:
        return (False, [])

    (success, b) = read_byte()
    if not success:
        return (False, [])

    ba = bytearray()
    ba.extend(data)
    #print ba
    return (b == ACK, ba)


def hibyte(s):
    return s >> 8

def lobyte(s):
    return s & 0xFF

def send_erase_cmd(startpage, numpages):
    return False

def send_ext_erase_cmd(startpage, numpages):
    if not send_command(ERASE_CMD):
        return False
        
    packet = bytearray()
    packet.append(hibyte(numpages-1))
    packet.append(lobyte(numpages-1))
    
    for p in range(startpage, startpage+numpages):
        packet.append(hibyte(p))
        packet.append(lobyte(p))
        
    packet.append(packet_checksum(packet))

    #print "Sending packet:"
    #print_packet(packet)
    
    if not send_data(packet):
        return False
    else:
        return True
    
def write_chunk(address, chunk):
    #print "Writing chunk at 0x{0:08x}".format(address)
    if not send_command(WRITE_MEM_CMD):
        return False

    #print "Sending address"
    packet = bytearray()
    packet.append(address >> 24)
    packet.append((address >> 16) & 0xff)
    packet.append((address >> 8) & 0xff)
    packet.append(address & 0xff)
    packet.append(packet_checksum(packet))
    if not send_data(packet):
        return False

    #print "Sending chunk"
    packet = bytearray()
    packet.append(len(chunk)-1)
    packet.extend(chunk)
    packet.append(packet_checksum(packet))
    return send_data(packet)

def boot(address):
    if not send_command(GO_CMD):
        return False

    packet = bytearray()
    packet.append(address >> 24)
    packet.append((address >> 16) & 0xff)
    packet.append((address >> 8) & 0xff)
    packet.append(address & 0xff)
    packet.append(packet_checksum(packet))
    if not send_data(packet):
        return False

    return True
    

def enter_dfu(portname):
    p = serial.Serial(portname, 38400, timeout=2, parity=serial.PARITY_NONE, stopbits=1)
    if not p.is_open:
        return False

    p.write('dfu\r\n')
    time.sleep(1)
    s = p.readline()
    p.close()
    return len(s) == 0
    
if __name__ == '__main__':
    if len(sys.argv) < 4:
        print "Usage: {0} port image address".format(sys.argv[0])
        sys.exit(1)

    # Determine which pages of Flash must be erased to write the image
    address = int(sys.argv[3], 16)
    if address % PAGE_SIZE:
        print "Invalid address: Must be aligned at 2K boundaries"
        sys.exit(1)

    if not os.path.isfile(sys.argv[2]):
        print "File not found: {0}".format(sys.argv[2])
        sys.exit(2)

    st = os.stat(sys.argv[2])
    if st.st_size > MAX_IMAGE_SIZE:
        print "Image file too large"
        sys.exit(1)

    
    port = serial.Serial(sys.argv[1], BAUD_RATE, timeout=2, parity=serial.PARITY_EVEN, stopbits=1)
    if not port.is_open:
        print "Failed to open port"
        sys.exit(2)

    bl_present = False
    for i in range(10):
        if do_handshake():
            print "Bootloader is present"
            bl_present = True
            break
        else:
            print "No response from bootloader"
            
    if not bl_present:
        sys.exit(1)

    drain()
        
    (success, data) = send_get()
    if success:
        configure_commands(data[2:])
    else:
        print "Failed to configure command table"
        sys.exit(1)
        
    
    startpage = (address - FLASH_BASE)/PAGE_SIZE
    numpages = st.st_size / PAGE_SIZE
    if st.st_size % PAGE_SIZE > 0:
        numpages += 1

    if ERASE_CMD == 0x43:
        r = send_erase_cmd(startpage, numpages)
    else:
        r = send_ext_erase_cmd(startpage, numpages)


    if r == False:
        print "Failed to erase pages"
    else:
        print "Erased {0} flash pages".format(numpages)

    
    """
    if send_command(WRITE_UNPROT_CMD):
        print "Write unprotect"
    else:
        print "Failed to unprotect flash"
        sys.exit(1)
   """

    addr = address
    with open(sys.argv[2], "rb") as f:
        while True:
            chunk = f.read(256)
            if len(chunk) == 0:
                break
            
            rem = len(chunk) % 4
            for i in range(rem):
                chunk += 0xff

            if not write_chunk(addr, chunk):
                print "Write failed"
                sys.exit(1)
                
            addr += len(chunk)

            sys.stdout.write("Flashing: {0:3d}%\r".format(100*(addr-address)/st.st_size))
            sys.stdout.flush()

    print

    """
    if send_command(WRITE_PROT_CMD):
        print "Write protect"
    else:
        print "Failed to protect flash"
        sys.exit(1)
    """

    if not boot(address):
        print "Failed to send GO command"
        sys.exit(1)

    print "Booted"
        
        

    

    
