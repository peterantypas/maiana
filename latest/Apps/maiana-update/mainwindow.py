from mainframe import MainFrame
from model import MaianaClient
import serial
import wx

class MainWindow(MainFrame):
    def __init__(self):
        MainFrame.__init__(self, None)
        self.m_SerialPortChoice.Set(MaianaClient.serial_ports())
        self.port = None
        self.data = None

    def onSerialPortSelection(self, event):
        self.m_SerialBtn.Enable()

    def onSerialBtnClick(self, event):
        if self.port is None:
            self.port = serial.Serial(self.m_SerialPortChoice.GetString(self.m_SerialPortChoice.GetSelection()), 38400, timeout=2)
            if self.port is None:
                wx.MessageBox(b'Unable to open port, it may be in use', 'Info', wx.OK | wx.ICON_ERROR)
            else:
                self.m_SerialBtn.SetLabel(b'Disconnect')
                self.enableUI()
                self.refreshSys()
                self.refreshStation()
        else:
            self.port.close()
            self.port = None
            self.m_SerialBtn.SetLabel(b'Connect')
            self.disableUI()

    def onStationSaveBtnClick(self, event):
        # Let's validate
        if not self.validateStationInputs():
            return

        newdata = {'mmsi': int(self.m_MMSIText.Value),
                   'name': self.m_NameText.Value,
                   'callsign': self.m_CallsignText.Value,
                   'len': int(self.m_LengthText.Value),
                   'beam': int(self.m_BeamText.Value),
                   'portoffset': int(self.m_PortOffsetText.Value),
                   'bowoffset': int(self.m_BowOffsetText.Value),
                   'type': MaianaClient.VESSEL_TYPES[self.m_VesselTypeChoice.Selection]}
        print(newdata)

        if MaianaClient.setStationData(self.port, newdata):
            self.stationdata = newdata


    def onFWBinarySelection(self, event):
        event.Skip()

    def onFWUpdateBtnClick(self, event):
        event.Skip()

    def enableUI(self):
        self.m_StationPnl.Enable()
        self.m_FWUpdatePnl.Enable()

    def disableUI(self):
        self.m_StationPnl.Disable()
        self.m_FWUpdatePnl.Disable()

    def refreshSys(self):
        self.sysdata = MaianaClient.loadSys(self.port)
        self.renderSys()

    def refreshStation(self):
        self.stationdata = MaianaClient.loadStation(self.port)
        self.renderStation()

    def renderSys(self):
        self.m_HWRevLbl.SetLabel(self.sysdata['hw'])
        self.m_FWRevLbl.SetLabel(self.sysdata['fw'])
        self.m_CPULbl.SetLabel(self.sysdata['cpu'])

    def renderStation(self):
        self.m_MMSIText.SetValue('{}'.format(self.stationdata['mmsi']))
        self.m_NameText.SetValue(self.stationdata['name'])
        self.m_CallsignText.SetValue(self.stationdata['callsign'])
        self.m_LengthText.SetValue('{}'.format(self.stationdata['len']))
        self.m_BeamText.SetValue('{}'.format(self.stationdata['beam']))
        self.m_PortOffsetText.SetValue('{}'.format(self.stationdata['portoffset']))
        self.m_BowOffsetText.SetValue('{}'.format(self.stationdata['bowoffset']))

        t = self.stationdata['type']
        i = MaianaClient.VESSEL_TYPES.index(t)
        self.m_VesselTypeChoice.SetSelection(i)

    def validateStationInputs(self):
        return True


