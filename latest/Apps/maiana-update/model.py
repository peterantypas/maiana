import serial
import sys
import glob
import time
import re


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
            ports = glob.glob('/dev/tty\..*')
        else:
            raise EnvironmentError('Unsupported platform')

        result = []
        for port in ports:
            # print port
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
            except (OSError, Exception):
                pass
        return result

    @staticmethod
    def sendCmdWithResponse(port, cmd, resp):
        port.write(cmd + b'\r\n')
        for i in range(10):
            s = port.readline().strip()
            if resp in s:
                return s
        return None

    @staticmethod
    def loadSys(port):
        sysline = MaianaClient.sendCmdWithResponse(port, b'sys?', b'$PAISYS')
        if sysline is None:
            return {}

        systokens = re.split(',|\\*', sysline.decode('utf-8'))
        sysd = {'fw': systokens[2], 'hw': systokens[1], 'cpu': systokens[4]}

        return sysd

    @staticmethod
    def loadStation(port):
        stationline = MaianaClient.sendCmdWithResponse(port, b'station?', b'$PAISTN')
        if stationline is None:
            return {}

        stationtokens = re.split(',|\\*', stationline.decode('utf-8'))
        stad = {'mmsi': int(stationtokens[1]), 'name': stationtokens[2], 'callsign': stationtokens[3],
                'type': int(stationtokens[4]), 'len': int(stationtokens[5]), 'beam': int(stationtokens[6]),
                'portoffset': int(stationtokens[7]), 'bowoffset': int(stationtokens[8])}

        return stad

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
        if resp:
            return MaianaClient.sendCmdWithResponse(port, b'reboot', b'$PAISTN')

        return False