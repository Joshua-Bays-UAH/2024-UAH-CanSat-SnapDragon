from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5.QtCore import QTimer, Qt
import sys

import serial
import time

MaxPackets = 15
State = "LAUNCH_WAIT"

sim = 0

# Connect the XBee
xbee = serial.Serial("/dev/ttyUSB0")
#xbee = serial.Serial("COM4")
xbee.timeout = .75

# Create break in packet log
f = open("packets.txt", "a")
f.write("")
f.write("")
f.close()

class MyWindow(QMainWindow):
    # Global variable
    global MaxPackets
    
    # Functional startup
    def __init__(self):
        super(MyWindow,self).__init__()
        self.initUI()
        self.readTimer = QTimer()
        self.readTimer.setInterval(100)
        self.readTimer.timeout.connect(self.read_packet)
        self.readTimer.start()
        self.c = 0
        
    # Create the UI
    def initUI(self):
        # Title
        self.setGeometry(0, 0, 1600, 720)
        self.setWindowTitle("Temporary GCS")
        
        # List of previous packets
        self.packet_list = QtWidgets.QListWidget(self)
        self.packet_list.move(0, 180)
        self.packet_list.resize(1000, 150)
        self.packet_list.setVerticalScrollBar(QtWidgets.QScrollBar())
        
        # Current state
        self.state_label = QtWidgets.QLabel(self)
        self.state_label.move(540, 0)
        self.state_label.resize(100, 200)
        self.state_label.setText("State: LW")

        # Simulation enable button
        self.sim_enable_btn = QtWidgets.QPushButton(self)
        self.sim_enable_btn.setText("Simulation enable")
        self.sim_enable_btn.clicked.connect(self.sim_enable)
        self.sim_enable_btn.setGeometry(0, 0, 160, 20)

        # Simulation activate button
        self.sim_activate_btn = QtWidgets.QPushButton(self)
        self.sim_activate_btn.setText("Simulation activate")
        self.sim_activate_btn.clicked.connect(self.sim_activate)
        self.sim_activate_btn.setGeometry(0, 30, 160, 20)

        # Simulation disable button
        self.sim_disable_btn = QtWidgets.QPushButton(self)
        self.sim_disable_btn.setText("Simulation disable")
        self.sim_disable_btn.clicked.connect(self.sim_disable)
        self.sim_disable_btn.setGeometry(0, 60, 160, 20)
        
        # Change to separate state button
        self.change_separate_btn = QtWidgets.QPushButton(self)
        self.change_separate_btn.setText("Separate")
        self.change_separate_btn.clicked.connect(self.change_separate)
        self.change_separate_btn.setGeometry(180, 0, 160, 20)
        
        # Change to release heat shield state button
        self.change_hs_release_btn = QtWidgets.QPushButton(self)
        self.change_hs_release_btn.setText("Heat Shield Release")
        self.change_hs_release_btn.clicked.connect(self.change_hs_release)
        self.change_hs_release_btn.setGeometry(180, 30, 160, 20)
        
        # Change to landed state button
        self.change_landed_btn = QtWidgets.QPushButton(self)
        self.change_landed_btn.setText("Land")
        self.change_landed_btn.clicked.connect(self.change_landed)
        self.change_landed_btn.setGeometry(180, 60, 160, 20)
        
        # Reconnect to XBee button
        self.reconnect_btn = QtWidgets.QPushButton(self)
        self.reconnect_btn.setText("Reconnect")
        self.reconnect_btn.clicked.connect(self.reconnect_xbee)
        self.reconnect_btn.setGeometry(360, 0, 160, 20)

    # Display and log packets (Runs automatically at 1Hz)
    def read_packet(self):
        p = ""
        p += xbee.read_until(b'!').decode()
        self.packet_list.addItem(QtWidgets.QListWidgetItem(p[0:len(p) - 1]))
        f = open("packets.txt", "a")
        f.write(p)
        f.close()
        if self.packet_list.count() > MaxPackets:
            self.packet_list.takeItem(0)

    def sim_enable(self):
        xbee.write(b"CMD,2079,SIM,ENABLE")
    
    def sim_activate(self):
        xbee.write(b"CMD,2079,SIM,ACTIVATE")
        sim = 1
    
    def sim_disable(self):
        xbee.write(b"CMD,2079,SIM,DISABLE")
    
    def change_separate(self):
        xbee.write(b"CMD,2079,STATE,SEPARATE")
    
    def change_hs_release(self):
        xbee.write(b"CMD,2079,STATE,HS_RELEASE")
    
    def change_landed(self):
        xbee.write(b"CMD,2079,STATE,LANDED")
    
    def reconnect_xbee(self):
        try:
            xbee.port = "/dev/USB0"
            xbee.open()
        except:
            print("could not reconnect")


def window():
    app = QApplication(sys.argv)
    win = MyWindow()
    win.show()
    sys.exit(app.exec_())

window()

xbee.close()
