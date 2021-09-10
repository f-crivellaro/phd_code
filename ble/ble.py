import logging
import logging.handlers
import os
import sys
from bluepy.btle import Scanner, DefaultDelegate, Peripheral, UUID

LOG_PATH = './logs'
LOG_FILENAME = '/ble.log'

# Set up a specific logger with our desired output level
log = logging.getLogger('')
log.setLevel(logging.DEBUG)
formatter = logging.Formatter("%(asctime)-19s \t %(levelname)-8s \t %(message)s")

# Set logger directory
if not os.path.exists(LOG_PATH):
    os.makedirs(LOG_PATH)
handler = logging.handlers.RotatingFileHandler(
              LOG_PATH+LOG_FILENAME, maxBytes=20000, backupCount=5)
handler.setFormatter(formatter)
log.addHandler(handler)

ch = logging.StreamHandler(sys.stdout)
ch.setFormatter(formatter)
log.addHandler(ch)


class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            log.info("Discovered device %s", dev.addr)
        elif isNewData:
            log.info("Received new data from %s", dev.addr)


class ProtoBle:
    def __init__(self, rd_address, rd_name, rd_services):
        self.address = rd_address
        self.name = rd_name
        self.services = rd_services


class MyDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        log.info("A notification was received: %s" %data)


scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(2.0)

for dev in devices:
    log.info("Device %s (%s), RSSI=%d dB" % (dev.addr, dev.addrType, dev.rssi))
    for (adtype, desc, value) in dev.getScanData():
        log.info("  %s = %s" % (desc, value))
        if desc == "Complete Local Name":
            if value == "Proto-Fever":
                device = Peripheral(dev.addr)
                device.setDelegate(MyDelegate())
                fever_device = ProtoBle(dev.addr, value, device.getServices())
                for svc in fever_device.services:
                    log.info(svc)
                tempSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
                tempService = device.getServiceByUUID(tempSensor)
                tempChar = tempService.getCharacteristics()
                for ch in tempChar:
                    log.info('Properties: %s', ch.propertiesToString())
                log.info('Read: %s', tempChar[0].read())
                device.writeCharacteristic(tempChar[0].valHandle+1, bytes.fromhex("0100"))
                # configChar = tempService.getCharacteristics(UUID("0x2902"))[0]
                # log.info('Properties of Config: %s', configChar.propertiesToString())
                # log.info('Read: %s', configChar.read())
                # tempChar.write(bytes.fromhex("0200"))
                # for ch in tempService.getCharacteristics():
                #     print(ch)
                log.info("\n\n")

while True:
    if device.waitForNotifications(1.0):
        # handleNotification() was called
        continue

    # log.info("Waiting...")
    # Perhaps do something else here
