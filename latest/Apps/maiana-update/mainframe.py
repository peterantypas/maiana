# -*- coding: utf-8 -*-

###########################################################################
## Python code generated with wxFormBuilder (version 3.10.1)
## http://www.wxformbuilder.org/
##
## PLEASE DO *NOT* EDIT THIS FILE!
###########################################################################

import wx
import wx.xrc

###########################################################################
## Class MainFrame
###########################################################################

class MainFrame ( wx.Frame ):

    def __init__( self, parent ):
        wx.Frame.__init__ ( self, parent, id = wx.ID_ANY, title = u"MAIANA Transponder Manager ver 0.2", pos = wx.DefaultPosition, size = wx.Size( 620,329 ), style = wx.CAPTION|wx.CLOSE_BOX|wx.MINIMIZE_BOX|wx.TAB_TRAVERSAL )

        self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

        bSizer1 = wx.BoxSizer( wx.VERTICAL )

        self.m_notebook2 = wx.Notebook( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_StatusPnl = wx.Panel( self.m_notebook2, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
        gSizer1 = wx.GridSizer( 6, 2, 0, 0 )

        self.m_staticText1 = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"Serial Port", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText1.Wrap( -1 )

        gSizer1.Add( self.m_staticText1, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        bSizer4 = wx.BoxSizer( wx.HORIZONTAL )

        m_SerialPortChoiceChoices = []
        self.m_SerialPortChoice = wx.Choice( self.m_StatusPnl, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, m_SerialPortChoiceChoices, 0 )
        self.m_SerialPortChoice.SetSelection( 0 )
        self.m_SerialPortChoice.SetMinSize( wx.Size( 180,-1 ) )

        bSizer4.Add( self.m_SerialPortChoice, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_SerialBtn = wx.Button( self.m_StatusPnl, wx.ID_ANY, u"Connect", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_SerialBtn.Enable( False )

        bSizer4.Add( self.m_SerialBtn, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )


        gSizer1.Add( bSizer4, 1, wx.EXPAND, 5 )

        self.m_staticText7 = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"CPU", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText7.Wrap( -1 )

        gSizer1.Add( self.m_staticText7, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_CPULbl = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"(unknown)", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_CPULbl.Wrap( -1 )

        self.m_CPULbl.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, wx.EmptyString ) )

        gSizer1.Add( self.m_CPULbl, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_staticText3 = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"Hardware Revision", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText3.Wrap( -1 )

        gSizer1.Add( self.m_staticText3, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_HWRevLbl = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"(unknown)", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_HWRevLbl.Wrap( -1 )

        self.m_HWRevLbl.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, wx.EmptyString ) )

        gSizer1.Add( self.m_HWRevLbl, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_staticText5 = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"Firmware Revision", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText5.Wrap( -1 )

        gSizer1.Add( self.m_staticText5, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_FWRevLbl = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"(unknown)", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_FWRevLbl.Wrap( -1 )

        self.m_FWRevLbl.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, wx.EmptyString ) )

        gSizer1.Add( self.m_FWRevLbl, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_staticText21 = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"Breakout Board Generation", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText21.Wrap( -1 )

        gSizer1.Add( self.m_staticText21, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_breakoutLbl = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"(unknown)", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_breakoutLbl.Wrap( -1 )

        gSizer1.Add( self.m_breakoutLbl, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )

        self.m_staticText23 = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"Bootloader Present", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText23.Wrap( -1 )

        gSizer1.Add( self.m_staticText23, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_bootloaderLbl = wx.StaticText( self.m_StatusPnl, wx.ID_ANY, u"(unknown)", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_bootloaderLbl.Wrap( -1 )

        gSizer1.Add( self.m_bootloaderLbl, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )


        self.m_StatusPnl.SetSizer( gSizer1 )
        self.m_StatusPnl.Layout()
        gSizer1.Fit( self.m_StatusPnl )
        self.m_notebook2.AddPage( self.m_StatusPnl, u"Status", True )
        self.m_StationPnl = wx.Panel( self.m_notebook2, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
        self.m_StationPnl.Enable( False )

        gSizer2 = wx.GridSizer( 0, 4, 0, 0 )

        self.m_staticText101 = wx.StaticText( self.m_StationPnl, wx.ID_ANY, u"MMSI", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText101.Wrap( -1 )

        gSizer2.Add( self.m_staticText101, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_MMSIText = wx.TextCtrl( self.m_StationPnl, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( -1,-1 ), 0 )
        self.m_MMSIText.SetMinSize( wx.Size( 180,-1 ) )

        gSizer2.Add( self.m_MMSIText, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_staticText11 = wx.StaticText( self.m_StationPnl, wx.ID_ANY, u"Call Sign", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText11.Wrap( -1 )

        gSizer2.Add( self.m_staticText11, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_CallsignText = wx.TextCtrl( self.m_StationPnl, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_CallsignText.SetMinSize( wx.Size( 120,-1 ) )

        gSizer2.Add( self.m_CallsignText, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_staticText12 = wx.StaticText( self.m_StationPnl, wx.ID_ANY, u"Name", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText12.Wrap( -1 )

        gSizer2.Add( self.m_staticText12, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_NameText = wx.TextCtrl( self.m_StationPnl, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_NameText.SetMaxLength( 20 )
        self.m_NameText.SetMinSize( wx.Size( 180,-1 ) )

        gSizer2.Add( self.m_NameText, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_staticText13 = wx.StaticText( self.m_StationPnl, wx.ID_ANY, u"Vessel Type", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText13.Wrap( -1 )

        gSizer2.Add( self.m_staticText13, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        m_VesselTypeChoiceChoices = [ u"Fishing", u"Diving", u"Sailing", u"Pleasure" ]
        self.m_VesselTypeChoice = wx.Choice( self.m_StationPnl, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, m_VesselTypeChoiceChoices, 0 )
        self.m_VesselTypeChoice.SetSelection( 0 )
        gSizer2.Add( self.m_VesselTypeChoice, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self._ = wx.StaticText( self.m_StationPnl, wx.ID_ANY, u"Length (m)", wx.DefaultPosition, wx.DefaultSize, 0 )
        self._.Wrap( -1 )

        gSizer2.Add( self._, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_LengthText = wx.TextCtrl( self.m_StationPnl, wx.ID_ANY, u"0", wx.DefaultPosition, wx.DefaultSize, 0 )
        gSizer2.Add( self.m_LengthText, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_staticText15 = wx.StaticText( self.m_StationPnl, wx.ID_ANY, u"Beam (m)", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText15.Wrap( -1 )

        gSizer2.Add( self.m_staticText15, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_BeamText = wx.TextCtrl( self.m_StationPnl, wx.ID_ANY, u"0", wx.DefaultPosition, wx.DefaultSize, 0 )
        gSizer2.Add( self.m_BeamText, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_staticText17 = wx.StaticText( self.m_StationPnl, wx.ID_ANY, u"Port Offset (m)", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText17.Wrap( -1 )

        gSizer2.Add( self.m_staticText17, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_PortOffsetText = wx.TextCtrl( self.m_StationPnl, wx.ID_ANY, u"0", wx.DefaultPosition, wx.DefaultSize, 0 )
        gSizer2.Add( self.m_PortOffsetText, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_staticText16 = wx.StaticText( self.m_StationPnl, wx.ID_ANY, u"Bow Offset (m)", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText16.Wrap( -1 )

        gSizer2.Add( self.m_staticText16, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_BowOffsetText = wx.TextCtrl( self.m_StationPnl, wx.ID_ANY, u"0", wx.DefaultPosition, wx.DefaultSize, 0 )
        gSizer2.Add( self.m_BowOffsetText, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_staticText18 = wx.StaticText( self.m_StationPnl, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText18.Wrap( -1 )

        gSizer2.Add( self.m_staticText18, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_staticText19 = wx.StaticText( self.m_StationPnl, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText19.Wrap( -1 )

        gSizer2.Add( self.m_staticText19, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_LEFT|wx.ALL, 5 )

        self.m_staticText20 = wx.StaticText( self.m_StationPnl, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText20.Wrap( -1 )

        gSizer2.Add( self.m_staticText20, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_StationSaveBtn = wx.Button( self.m_StationPnl, wx.ID_ANY, u"Save", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_StationSaveBtn.Enable( False )

        gSizer2.Add( self.m_StationSaveBtn, 0, wx.ALL|wx.ALIGN_BOTTOM, 5 )


        self.m_StationPnl.SetSizer( gSizer2 )
        self.m_StationPnl.Layout()
        gSizer2.Fit( self.m_StationPnl )
        self.m_notebook2.AddPage( self.m_StationPnl, u"Station Data", False )
        self.m_FWUpdatePnl = wx.Panel( self.m_notebook2, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
        self.m_FWUpdatePnl.Enable( False )

        bSizer41 = wx.BoxSizer( wx.VERTICAL )

        gSizer4 = wx.GridSizer( 1, 2, 0, 0 )

        gSizer4.SetMinSize( wx.Size( -1,90 ) )
        self.m_staticText28 = wx.StaticText( self.m_FWUpdatePnl, wx.ID_ANY, u"Binary File:", wx.DefaultPosition, wx.Size( -1,-1 ), 0 )
        self.m_staticText28.Wrap( -1 )

        gSizer4.Add( self.m_staticText28, 1, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )

        self.m_FWBinaryPicker = wx.FilePickerCtrl( self.m_FWUpdatePnl, wx.ID_ANY, wx.EmptyString, u"Select a file", u"*.bin", wx.DefaultPosition, wx.Size( -1,-1 ), wx.FLP_CHANGE_DIR|wx.FLP_DEFAULT_STYLE|wx.FLP_FILE_MUST_EXIST|wx.FLP_OPEN )
        gSizer4.Add( self.m_FWBinaryPicker, 1, wx.ALL|wx.ALIGN_CENTER_VERTICAL|wx.EXPAND, 5 )


        bSizer41.Add( gSizer4, 1, wx.EXPAND, 5 )

        gSizer5 = wx.GridSizer( 1, 2, 0, 0 )

        self.m_FWProgress = wx.Gauge( self.m_FWUpdatePnl, wx.ID_ANY, 100, wx.DefaultPosition, wx.Size( 350,-1 ), wx.GA_HORIZONTAL )
        self.m_FWProgress.SetValue( 0 )
        gSizer5.Add( self.m_FWProgress, 0, wx.ALL, 5 )

        self.m_FWUpdateBtn = wx.Button( self.m_FWUpdatePnl, wx.ID_ANY, u"Update", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_FWUpdateBtn.Enable( False )

        gSizer5.Add( self.m_FWUpdateBtn, 0, wx.ALL|wx.ALIGN_RIGHT, 5 )


        bSizer41.Add( gSizer5, 1, wx.EXPAND, 5 )

        self.m_FWUpdateStatusLbl = wx.StaticText( self.m_FWUpdatePnl, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_FWUpdateStatusLbl.Wrap( -1 )

        self.m_FWUpdateStatusLbl.SetMinSize( wx.Size( 520,-1 ) )

        bSizer41.Add( self.m_FWUpdateStatusLbl, 0, wx.ALL, 5 )


        self.m_FWUpdatePnl.SetSizer( bSizer41 )
        self.m_FWUpdatePnl.Layout()
        bSizer41.Fit( self.m_FWUpdatePnl )
        self.m_notebook2.AddPage( self.m_FWUpdatePnl, u"FW Update", False )

        bSizer1.Add( self.m_notebook2, 1, wx.EXPAND |wx.ALL, 5 )


        self.SetSizer( bSizer1 )
        self.Layout()

        self.Centre( wx.BOTH )

        # Connect Events
        self.m_SerialPortChoice.Bind( wx.EVT_CHOICE, self.onSerialPortSelection )
        self.m_SerialBtn.Bind( wx.EVT_BUTTON, self.onSerialBtnClick )
        self.m_MMSIText.Bind( wx.EVT_TEXT, self.onStationEdit )
        self.m_CallsignText.Bind( wx.EVT_TEXT, self.onStationEdit )
        self.m_NameText.Bind( wx.EVT_TEXT, self.onStationEdit )
        self.m_VesselTypeChoice.Bind( wx.EVT_CHOICE, self.onStationEdit )
        self.m_LengthText.Bind( wx.EVT_TEXT, self.onStationEdit )
        self.m_BeamText.Bind( wx.EVT_TEXT, self.onStationEdit )
        self.m_PortOffsetText.Bind( wx.EVT_TEXT, self.onStationEdit )
        self.m_BowOffsetText.Bind( wx.EVT_TEXT, self.onStationEdit )
        self.m_StationSaveBtn.Bind( wx.EVT_BUTTON, self.onStationSaveBtnClick )
        self.m_FWBinaryPicker.Bind( wx.EVT_FILEPICKER_CHANGED, self.onFWBinarySelection )
        self.m_FWUpdateBtn.Bind( wx.EVT_BUTTON, self.onFWUpdateBtnClick )

    def __del__( self ):
        pass


    # Virtual event handlers, override them in your derived class
    def onSerialPortSelection( self, event ):
        event.Skip()

    def onSerialBtnClick( self, event ):
        event.Skip()

    def onStationEdit( self, event ):
        event.Skip()








    def onStationSaveBtnClick( self, event ):
        event.Skip()

    def onFWBinarySelection( self, event ):
        event.Skip()

    def onFWUpdateBtnClick( self, event ):
        event.Skip()


