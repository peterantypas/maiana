from threading import Thread
import wx
import os
import binascii
import sys
from enum import Enum


EVT_FWUPDATE_ID = wx.NewId()

class EventType(Enum):
    DFU_ENABLED = 1
    TRANSFER_STARTED = 2
    TRANSFER_PROGRESS = 3
    TRANSFER_COMPLETE = 4
    ERROR = 5

class FwUpdateEvent(wx.PyEvent):
    def __init__(self, eventType, data):
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_FWUPDATE_ID)
        self.eventType = eventType
        self.data = data


class FWUpdateThread(Thread):
    def __init__(self, window, port, filename):
        Thread.__init__(self)
        self.daemon = True
        self.window = window
        self.port = port
        self.filesize = 0
        self.crc32 = 0

        try:
            self.file = open(filename, "rb")
        except:
            print("Unable to open file {0}".format(filename))
            sys.exit(2)

        self.filesize = os.stat(filename).st_size
        # print filesize
        self.data = self.file.read()
        self.crc32 = (binascii.crc32(self.data) & 0xFFFFFFFF)
        self.file.seek(0)

    def is_unit_running(self):
        print("Checking if unit is running")
        s = self.port.readline().strip().decode('utf-8')
        if len(s) > 0:
            # print s
            tokens = s.split(',')
            if len(tokens) > 3:
                return True
        return False

    def drain_port(self):
        self.port.flushInput()
        s = self.port.readline()
        while len(s) > 0:
            s = self.port.readline()

    def enable_dfu(self):
        if self.is_unit_running():
            print("Unit is running, switching to DFU mode")
            self.port.write(b'dfu\r\n')
            self.drain_port()

        for x in range(5):
            self.port.write(b'\r\n')
            s = self.port.readline().strip()
            if s.find(b"MAIANA bootloader") >= 0:
                wx.PostEvent(self.window, FwUpdateEvent(EventType.DFU_ENABLED, None))
                return True

        wx.PostEvent(self.window, FwUpdateEvent(EventType.ERROR, b'Could not enter DFU mode'))
        return False

    def begin_transfer(self):
        self.drain_port()
        command = "load {0} {1:x}\r\n".format(self.filesize, self.crc32).encode('utf-8')
        self.port.write(command)
        print(command)

        s = self.port.readline().strip()
        if s == b"READY":
            wx.PostEvent(self.window, FwUpdateEvent(EventType.TRANSFER_STARTED, None))
            return True

        wx.PostEvent(self.window, FwUpdateEvent(EventType.ERROR, 'Failed:{}'.format(s)))
        return False

    def run(self):
        if not self.enable_dfu():
            print("Could not get unit into DFU mode")
            return

        print("Unit is in DFU mode")

        if not self.begin_transfer():
            print("Unable to begin transfer, restart the unit and retry")
            return

        print("Starting transfer")

        bytes = self.file.read(2048)
        resp = ''
        bytesSent = 0
        while len(bytes) > 0:
            self.port.write(bytes)
            resp = self.port.readline().strip()
            if resp.find(b"OK") < 0:
                break

            bytesSent += len(bytes)
            progress = bytesSent/self.filesize
            wx.PostEvent(self.window, FwUpdateEvent(EventType.TRANSFER_PROGRESS, progress))
            bytes = self.file.read(2048)

        wx.PostEvent(self.window, FwUpdateEvent(EventType.TRANSFER_COMPLETE, None))




