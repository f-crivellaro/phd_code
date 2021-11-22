import logging
import logging.handlers
import os
import sys
import paho.mqtt.client as mqtt
import numpy as np
import json
from bluepy.btle import Scanner, DefaultDelegate, Peripheral, UUID


# ----------------------------------------------------------------------------------------------------------------------


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


# ----------------------------------------------------------------------------------------------------------------------

broker_address = "localhost"
log.info("MQTT: creating a new MQTT client instance")
client = mqtt.Client("Proto-PC")
log.info("MQTT: connecting to broker - %s", broker_address)
client.connect(broker_address)
topic_cmd = "spec/cmd/meas"
log.info("Subscribing to topic %s" %topic_cmd)
client.subscribe(topic_cmd)

last_msg = {}
def measure_request(client, userdata, message):
    log.info('Measure request callback')
    topic = "spec/reply/meas"
    log.info("MQTT: publishing message to topic %s", topic)
    client.publish(topic, json.dumps(last_msg))


client.message_callback_add(topic_cmd, measure_request)
client.loop_start()

# ----------------------------------------------------------------------------------------------------------------------


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


sensorType = ""


class MyDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        global last_msg
        log.info("A notification was received: %s", data.decode("utf-8"))
        if sensorType == "Proto-Light":
            measures = data.decode("utf-8")
            measures_splitted = measures.split('/')
            measures_formatted = [int(a) for a in measures_splitted]
            log.debug('Measured Splited: %s', measures_formatted)
            # for part in measures.split('/'):
            wave_start = 340.904358
            wave_end = 1467.710083
            # x = np.linspace(425.00, 675.00, num=2048, retstep=False)
            x = np.linspace(wave_start, wave_end, num=2048, retstep=False)
            wavelengths = [415, 445, 480, 515, 555, 590, 630, 680, 910]
            full_data = np.interp(x, wavelengths, measures_formatted)
            # log.info('Measured Full: %s', full_data)
            # log.info('Wavelengths: %s', x)
            msg = {'data': full_data.tolist(), 'wavelengths': x.tolist()}
            last_msg = msg


scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(2.0)

for dev in devices:
    log.info("Device %s (%s), RSSI=%d dB" % (dev.addr, dev.addrType, dev.rssi))
    for (adtype, desc, value) in dev.getScanData():
        log.info("  %s = %s" % (desc, value))
        if desc == "Complete Local Name":
            if value == "Proto-Fever" or value == "Proto-Light":
                sensorType = value
                device = Peripheral(dev.addr)
                device.setMTU(50)
                device.setDelegate(MyDelegate())
                ble_device = ProtoBle(dev.addr, value, device.getServices())
                for svc in ble_device.services:
                    log.info(svc)
                bleSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
                bleService = device.getServiceByUUID(bleSensor)
                bleChar = bleService.getCharacteristics()
                for ch in bleChar:
                    log.info('Properties: %s', ch.propertiesToString())
                log.info('Read: %s', bleChar[0].read())
                device.writeCharacteristic(bleChar[0].valHandle+1, bytes.fromhex("0100"))
                log.info("\n\n")

# ----------------------------------------------------------------------------------------------------------------------

while True:
    if device.waitForNotifications(1.0):
        # handleNotification() was called
        continue

    # log.info("Waiting...")
    # Perhaps do something else here
