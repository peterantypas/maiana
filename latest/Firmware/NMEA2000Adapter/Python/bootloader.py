import serial
import sys
import time
import os

ACK  = 0x79
NACK = 0x1F
GET  = 0x00

PAGE_SIZE       = 2048
FLASH_BASE      = 0x08000000
BAUD_RATE       = 115200
MAX_IMAGE_SIZE  = 128*1024

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

"""
def print_packet(d):
    for c in d:
        sys.stdout.write("0x{:02x} ".format(c))
    sys.stdout.write('\n')
"""

error = "No error"

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
    
def read_byte(port):
    r = port.read(1)
    if len(r) > 0:
        return (True, r[0])
    else:
        return (False, 0)

def drain(port):
    keepreading = True
    while True:
        (r, keepreading) = read_byte(port)
        if r == False:
            break
    
def do_handshake(port):
    port.write([0x7f])
    (success, b) = read_byte(port)
    if not success:
        return False

    return b == ACK

def complement(cmd):
    c = ~cmd
    if c < 0:
        c += 256
    return c

def send_command(port, cmd):
    packet = [cmd, complement(cmd)]
    #print packet
    port.write(packet)
    (success, r) = read_byte(port)
    #print r
    if not success:
        #print("Failed to send command 0x{0:2x}".format(cmd))
        return False
    
    if r != ACK:
        #print("Got NACK for command 0x{0:2x}".format(cmd))
        return False

    #print "Got ACK"
    return True
    

def send_data(port, packet):
    port.write(packet)    
    (success, b) = read_byte(port)
    if not success:
        #print("No ACK or NACK for data packet")
        return False

    if b == ACK:
        return True
    else:
        #print("Got NACK for data packet")
        return False


def send_get(port):
    if not send_command(port, GET):
        #print("Failed to send command GET")
        return (False, [])

    (success, bytes) = read_byte(port)

    if success:
        bytes += 1
    else:
        return (False, [])

    data = port.read(bytes)
    if len(data) < bytes:
        return (False, [])

    (success, b) = read_byte(port)
    if not success:
        return (False, [])

    ba = bytearray()
    ba.extend(data)
    
    return (b == ACK, ba)


def hibyte(s):
    return s >> 8

def lobyte(s):
    return s & 0xFF

def send_erase_cmd(startpage, numpages):
    return False

def send_ext_erase_cmd(port, startpage, numpages):
    if not send_command(port, ERASE_CMD):
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
    
    if not send_data(port, packet):
        return False
    else:
        return True
    
def write_chunk(port, address, chunk):
    #print "Writing chunk at 0x{0:08x}".format(address)
    if not send_command(port, WRITE_MEM_CMD):
        return False

    #print "Sending address"
    packet = bytearray()
    packet.append(address >> 24)
    packet.append((address >> 16) & 0xff)
    packet.append((address >> 8) & 0xff)
    packet.append(address & 0xff)
    packet.append(packet_checksum(packet))
    if not send_data(port, packet):
        return False

    #print "Sending chunk"
    packet = bytearray()
    packet.append(len(chunk)-1)
    packet.extend(chunk)
    packet.append(packet_checksum(packet))
    return send_data(port, packet)

def boot(port, address):
    if not send_command(port, GO_CMD):
        return False

    packet = bytearray()
    packet.append(address >> 24)
    packet.append((address >> 16) & 0xff)
    packet.append((address >> 8) & 0xff)
    packet.append(address & 0xff)
    packet.append(packet_checksum(packet))
    if not send_data(port, packet):
        return False

    return True
    

def programFirmware(portname, filename):
    address = FLASH_BASE                # Always!

    global error
    
    st = os.stat(filename)
    if st.st_size > MAX_IMAGE_SIZE:
        error = "Image file too large"
        return False

    port = serial.Serial(portname, baudrate=BAUD_RATE, timeout=2, parity=serial.PARITY_EVEN, stopbits=1)
    if port is None or not port.is_open:
        error = "Failed to open port"
        return False

    bl_present = False
    for i in range(5):
        if do_handshake(port):
            #print("Bootloader is present")
            bl_present = True
            break
        else:
            pass
            #print("No response from bootloader")
            
    if not bl_present:
        port.close()
        error = "No response from MCU bootloader"
        return False

    drain(port)
        
    (success, data) = send_get(port)
    if success:
        configure_commands(data[2:])
    else:
        #print("Failed to configure command table")
        error = "Failed to configure command table"
        port.close()
        return False
        
    
    startpage = 0
    numpages = int(st.st_size / PAGE_SIZE)
    if st.st_size % PAGE_SIZE > 0:
        numpages += 1

    if ERASE_CMD == 0x43:
        r = send_erase_cmd(port, startpage, numpages)
    else:
        r = send_ext_erase_cmd(port, startpage, numpages)


    if r == False:
        #print("Failed to erase pages")
        pass
    else:
        #print("Erased {0} flash pages".format(numpages))
        pass

    
    addr = address
    with open(filename, "rb") as f:
        while True:
            chunk = f.read(256)
            if len(chunk) == 0:
                break
            
            rem = len(chunk) % 4
            for i in range(rem):
                chunk += 0xff

            if not write_chunk(port, addr, chunk):
                #print("Write failed")
                port.close()
                error = "Write failed"
                return False
                
            addr += len(chunk)

            sys.stdout.write("Flashing: {0:3d}%\r".format(int(100*(addr-address)/st.st_size)))
            sys.stdout.flush()

    print()

    if not boot(port, address):
        error = "Failed to send GO command"
        port.close()
        return False

    port.close()    
    return True

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: {0} port image".format(sys.argv[0]))
        sys.exit(1)

    if not programFirmware(sys.argv[1], sys.argv[2]):
        print("Error: {}".format(error))

        

    

    
