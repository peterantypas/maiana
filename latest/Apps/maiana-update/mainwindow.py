from mainframe import MainFrame
from maianaclient import MaianaClient, MaianaStatus
import serial
import wx
from fwUpdateThread import *

class MainWindow(MainFrame):
    def __init__(self):
        MainFrame.__init__(self, None)
        self.m_SerialPortChoice.Set(MaianaClient.serial_ports())
        self.portname = None
        self.port = None
        self.data = None

    def onSerialPortSelection(self, event):
        self.m_SerialBtn.Enable()

    def onSerialBtnClick(self, event):
        if self.port is None:
            try:
                self.portname = self.m_SerialPortChoice.GetString(self.m_SerialPortChoice.GetSelection())
                self.port = serial.Serial(self.portname, 38400, timeout=1)
            except:
                wx.MessageBox(b'Unable to open port, it may be in use', 'Error', wx.OK | wx.ICON_ERROR)
                return

            status = MaianaClient.determineStatus(self.port)

            if status == MaianaStatus.UNKNOWN:
                pass
            elif status == MaianaStatus.DFU:
                wx.MessageBox(b'MAIANA is currently in firmware update mode. This is the only task you can perform.',
                              'DFU warning', wx.OK)
                self.enableUI()
                self.m_SerialBtn.SetLabel(b'Disconnect')
                self.m_StationSaveBtn.Disable()
            else:
                #Assuming status is RUNNING
                self.m_SerialBtn.SetLabel(b'Disconnect')
                self.enableUI()
                if self.refreshSys():
                    self.refreshStation()
                    self.m_StationSaveBtn.Disable()
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
        #print(newdata)

        if MaianaClient.setStationData(self.port, newdata):
            self.stationdata = newdata
            self.m_StationSaveBtn.Disable()


    def onFWBinarySelection(self, event):
        self.m_FWUpdateBtn.Enable()

    def onFWUpdateBtnClick(self, event):
        self.Connect(-1, -1, EVT_FWUPDATE_ID, self.onFwUpdateEvent)
        thr = FWUpdateThread(self, self.port, self.m_FWBinaryPicker.Path)
        thr.start()
        self.m_FWUpdateBtn.Disable()

    def onFwUpdateEvent(self, evt):
        print(evt.eventType, evt.data)
        if evt.eventType == EventType.DFU_ENABLED:
            self.m_FWUpdateStatusLbl.SetLabel("DFU enabled")
        elif evt.eventType == EventType.TRANSFER_STARTED:
            self.m_FWUpdateStatusLbl.SetLabel("Transferring")
        elif evt.eventType == EventType.TRANSFER_PROGRESS:
            self.m_FWProgress.SetValue(evt.data * 100)
        elif evt.eventType == EventType.TRANSFER_COMPLETE:
            self.m_FWProgress.SetValue(0)
            self.m_FWUpdateStatusLbl.SetLabel("Transfer completed")
            self.refreshSys()
            self.refreshStation()
        elif evt.eventType == EventType.ERROR:
            self.m_FWUpdateStatusLbl.SetLabel(evt.data)

    def enableUI(self):
        self.m_StationPnl.Enable()
        self.m_FWUpdatePnl.Enable()

    def disableUI(self):
        self.m_StationPnl.Disable()
        self.m_FWUpdatePnl.Disable()

    def refreshSys(self):
        self.sysdata = MaianaClient.loadSys(self.port)
        return self.renderSys()

    def refreshStation(self):
        self.stationdata = MaianaClient.loadStation(self.port)
        return self.renderStation()

    def renderSys(self):
        if not 'hw' in self.sysdata:
            wx.MessageDialog(self, b'There was no response from MAIANA. Please check connections and try again.',
                             'Timeout', wx.OK | wx.STAY_ON_TOP|wx.CENTRE).ShowModal()
            return False

        self.m_HWRevLbl.SetLabel(self.sysdata['hw'])
        self.m_FWRevLbl.SetLabel(self.sysdata['fw'])
        self.m_CPULbl.SetLabel(self.sysdata['cpu'])
        return True

    def renderStation(self):
        if not 'mmsi' in self.stationdata:
            wx.MessageDialog(self, b'There was no response from MAIANA. Please check connections and try again.',
                             'Timeout', wx.OK | wx.STAY_ON_TOP|wx.CENTRE).ShowModal()
            return False

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
        return True

    def validateStationInputs(self):
        return True

    def onStationEdit( self, event ):
        self.m_StationSaveBtn.Enable()
        event.Skip()
