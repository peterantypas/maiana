import serial
import sys
import glob
import time
import re
from enum import Enum


class MaianaStatus(Enum):
    UNKNOWN = 0
    RUNNING = 1
    DFU = 2

class MaianaClient:
    VESSEL_TYPES = [30, 34, 36, 37]

    @staticmethod
    def serial_ports():
        """ Lists serial port names

            :raises EnvironmentError:
                On unsupported or unknown platforms
            :returns:
                A list of the serial ports available on the system
        """
        if sys.platform.startswith('win'):
            ports = ['COM%s' % (i + 1) for i in range(256)]
        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
            # this excludes your current terminal "/dev/tty"
            ports = glob.glob('/dev/tty[A-Za-z]*')
        elif sys.platform.startswith('darwin'):
            ports = glob.glob('/dev/tty.*')
            print(ports)
        else:
            raise EnvironmentError('Unsupported platform')

        result = []
        for port in ports:
            # print port
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
            except (OSError, Exception) as e:
                print(e)
                pass
        return result

    @staticmethod
    def determineStatus(port):
        port.flushInput()
        port.flushOutput()
        s = port.readline().strip()
        port.write(b'\r\n')
        for i in range(5):
            s = port.readline().strip()
            if s.find(b"bootloader") > -1:
                return MaianaStatus.DFU
            else:
                tokens = s.decode('utf-8').split(',')
                if (tokens[0][0] == '$' or tokens[0][0] == '!') and len(tokens) >= 2:
                    return MaianaStatus.RUNNING

        return MaianaStatus.UNKNOWN

    @staticmethod
    def sendCmdWithResponse(port, cmd, resp):
        port.flushInput()
        port.flushOutput()
        port.write(cmd + b'\r\n')
        for i in range(25):
            s = port.readline().strip()
            #print(s)
            if s == b'':
                break
            if resp in s:
                return s
        return None

    @staticmethod
    def loadSys(port):
        for i in range(2):
            try:
                sysline = MaianaClient.sendCmdWithResponse(port, b'sys?', b'$PAISYS')
                if sysline is None:
                    return {}

                systokens = re.split(',|\\*', sysline.decode('utf-8'))
                sysd = {'fw': systokens[2],
                        'hw': systokens[1],
                        'cpu': systokens[4],
                        'newbrkout': int(systokens[5]),
                        'bootloader': int(systokens[6])}
                return sysd
            except:
                pass

        return {}

    @staticmethod
    def loadStation(port):
        for i in range(2):
            try:
                stationline = MaianaClient.sendCmdWithResponse(port, b'station?', b'$PAISTN')
                if stationline is None:
                    return {}

                stationtokens = re.split(',|\\*', stationline.decode('utf-8'))
                stad = {'mmsi': int(stationtokens[1]), 'name': stationtokens[2], 'callsign': stationtokens[3],
                        'type': int(stationtokens[4]), 'len': int(stationtokens[5]), 'beam': int(stationtokens[6]),
                        'portoffset': int(stationtokens[7]), 'bowoffset': int(stationtokens[8])}

                return stad
            except:
                pass
        return {}

    @staticmethod
    def setStationData(port, data):
        line = 'station {},{},{},{},{},{},{},{}\r\n'.format(
            data['mmsi'],
            data['name'],
            data['callsign'],
            data['type'],
            data['len'],
            data['beam'],
            data['portoffset'],
            data['bowoffset']
        )

        resp = MaianaClient.sendCmdWithResponse(port, line.encode('utf-8'), b'$PAISTN')
        #print(resp)
        if resp:
            resp = MaianaClient.sendCmdWithResponse(port, b'reboot', b'$PAISTN')
            #print(resp)
            return not resp is None

        return False