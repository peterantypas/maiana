import wx 
from mainwindow import MainWindow


if __name__ == '__main__':
    app = wx.App()
    window = MainWindow()
    window.Show(True)
    app.MainLoop()
