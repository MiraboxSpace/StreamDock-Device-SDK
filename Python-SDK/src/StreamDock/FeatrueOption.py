from enum import Enum

class device_type(Enum):
    dock_universal = 0
    dock_293 = 1
    dock_293v3=2
    dock_293s=3
    dock_293sv3=4
    dock_m3=5
    dock_m18=6
    dock_n1=7
    dock_n3=8
    dock_n4=9
    dock_n4pro=10
    dock_xl=11
    k1pro=12
    
class FeatrueOption:
    def __init__(self):
        self.hasRGBLed = False
        self.ledCounts = 0
        self.supportConfig = False
        self.supportBackgroundImage = True
        self.deviceType = device_type.dock_universal
