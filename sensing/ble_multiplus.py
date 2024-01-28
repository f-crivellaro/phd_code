import logging
import logging.handlers
import os
import sys
import paho.mqtt.client as mqtt
import numpy as np
import json
from bluepy import btle
from bluepy.btle import Scanner, DefaultDelegate, Peripheral, UUID
import time
import csv
from datetime import datetime
from scipy import signal
import statistics
from struct import unpack


# ----------------------------------------------------------------------------------------------------------------------


LOG_PATH = './logs'
LOG_FILENAME = '/ble.log'
DATA_PATH = './data'
FEVER_FILE = '/fever.csv'

# Define time 0 for measurements
start_time = time.time()

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
topic_beat = "protobeat/command"
topic_fever = "fever/command"
log.info("Subscribing to topic %s" %topic_cmd)
log.info("Subscribing to topic %s" %topic_beat)
log.info("Subscribing to topic %s" %topic_fever)
client.subscribe(topic_cmd)
client.subscribe(topic_beat)
client.subscribe(topic_fever)

last_msg = {}
def measure_request(client, userdata, message):
    log.info('Measure request callback')
    topic = "spec/reply/meas"
    log.info("MQTT: publishing message to topic %s", topic)
    client.publish(topic, json.dumps(last_msg))

last_beat_msg = {}
samples = "" #100
sendSamples = False

def beat_request(client, userdata, message):
    global samples,sendSamples
    msg = json.loads(message.payload.decode("utf-8"))
    log.info(f'Proto-Beat: GUI request callback: msg={msg}')
    # log.debug(f'Samples to acquire: {msg["duration"]}')
    samples = msg["duration"]
    sendSamples = True
    # bleChar[1].write(str(msg["duration"]).encode())
    # topic = "wipy/reply"
    # log.info("MQTT: publishing message to topic %s - msg %s", topic, last_beat_msg)
    # client.publish(topic, json.dumps(last_beat_msg))


client.message_callback_add(topic_cmd, measure_request)
client.message_callback_add(topic_beat, beat_request)
client.loop_start()

# ----------------------------------------------------------------------------------------------------------------------


class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            log.debug("Discovered device %s", dev.addr)
        elif isNewData:
            log.debug("Received new data from %s", dev.addr)


class ProtoBle:
    def __init__(self, rd_address, rd_name, rd_services):
        self.address = rd_address
        self.name = rd_name
        self.services = rd_services


sensorType = ""

class ProtoFeverDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
        # self._device = device

    def enable_notify(self, chara_uuid):
        setup_data = b"\x01\x00"
        notify = self.ble_conn.getCharacteristics(uuid=chara_uuid)[0]
        notify_handle = notify.getHandle() + 1
        self.ble_conn.writeCharacteristic(notify_handle, setup_data, withResponse=True)

    def handleNotification(self, cHandle, data):
        global last_msg, last_beat_msg, sensorType, ProtoDevice
        data_decoded = data.decode("utf-8")
        log.info("A Proto-Fever notification was received: %s", data_decoded)
        # log.info(cHandle)
        try:
            now = time.time()
            data_formatted = float(data_decoded)
            # with open(DATA_PATH+FEVER_FILE, 'a') as f:
                # writer = csv.writer(f, delimiter=',')
                # msg_formatted = [now, data_formatted]
                # writer.writerow(msg_formatted)
            fever_msg = {'data': data_formatted, 'timestamp': now}
            topic = "fever/reply/meas"
            log.info("MQTT: publishing message to topic %s", topic)
            client.publish(topic, json.dumps(fever_msg))
            # self._device.disconnect()
            # self._device = None
        except Exception as e: 
            log.error(e)

class ProtoBiliDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        global last_msg, last_beat_msg, sensorType, ProtoDevice
        data_decoded = data.decode("utf-8")
        log.info("A ProtoBili notification was received: %s", data_decoded)
        log.info(cHandle)
        measures = data_decoded
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

class ProtoBeatDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
        self._beat_array = []
        self._beat_array_red = []
        self._beat_array_amb1 = []
        self._beat_array_amb2 = []
        self._beat_timestamps = []
        self._beat_start = None

    def handleNotification(self, cHandle, data):
        global last_msg, last_beat_msg, sensorType, ProtoDevice
        data_decoded = data.decode("utf-8")
        # log.debug("A Proto-Beat notification was received: %s", data_decoded)
        # log.debug(cHandle)
        if (data_decoded != "endArray") and (data_decoded != "Welcome to BLE Beat Sensor"):
            measures = data_decoded
            measures_splitted = measures.split('/')
            measures_formatted = [int(a) for a in measures_splitted]
            if not self._beat_start:
                self._beat_start = measures_formatted[1] #time.time()
                self._beat_timestamps.append(0)
            else:
                self._beat_timestamps.append(measures_formatted[1]-self._beat_start)

            self._beat_array.append(measures_formatted[0])
            self._beat_array_red.append(measures_formatted[2])
            self._beat_array_amb1.append(measures_formatted[3])
            self._beat_array_amb2.append(measures_formatted[4])
        else:
            if (len(self._beat_array) > 0):
                log.info('Proto-Beat: new buffer received')
                greenMeas = [int(a - statistics.mean(self._beat_array)) for a in self._beat_array]
                msg = {'series': ['PPG'], 'data': [self._beat_array], 'labels': self._beat_timestamps}
                # msg = {'series': ['PPG'], 'data': [greenMeas], 'labels': self._beat_timestamps}
                last_beat_msg = msg
                topic = "protobeat/reply/green"
                # log.info("MQTT: publishing message to topic %s - msg %s", topic, last_beat_msg)
                client.publish(topic, json.dumps(last_beat_msg))
                # hp_sos = signal.butter(3, 0.05, 'hp', fs=15, output='sos')
                # hp_filtered = signal.sosfilt(hp_sos, self._beat_array)
                redMeas = [int(a - statistics.mean(self._beat_array_red)) for a in self._beat_array_red]
                msg = {'series': ['PPG'], 'data': [self._beat_array_red], 'labels': self._beat_timestamps}
                # msg = {'series': ['PPG'], 'data': [redMeas], 'labels': self._beat_timestamps}
                last_beat_msg = msg
                topic = "protobeat/reply/red"
                client.publish(topic, json.dumps(last_beat_msg))
                msg = {'series': ['PPG'], 'data': [self._beat_array_amb1], 'labels': self._beat_timestamps}
                last_beat_msg = msg
                topic = "protobeat/reply/ambient1"
                client.publish(topic, json.dumps(last_beat_msg))
                msg = {'series': ['PPG'], 'data': [self._beat_array_amb2], 'labels': self._beat_timestamps}
                last_beat_msg = msg
                topic = "protobeat/reply/ambient2"
                client.publish(topic, json.dumps(last_beat_msg))
                # sos = signal.butter(3, 0.2, 'lp', fs=None, output='sos')
                # filtered = signal.sosfilt(sos, greenMeas)
                # clean_samples = 100
                # msg = {'series': ['PPG'], 'data': [filtered[clean_samples:].tolist()], 'labels': self._beat_timestamps[clean_samples:]}
                # last_beat_msg = msg
                # topic = "protobeat/reply/green/filter"
                # client.publish(topic, json.dumps(last_beat_msg))
                log.info('Proto-Beat: data series sent to GUI')
                self._beat_timestamps = []
                self._beat_array = []
                self._beat_array_red = []
                self._beat_array_amb1 = []
                self._beat_array_amb2 = []
                self._beat_start = None



# ----------------------------------------------------------------------------------------------------------------------

def detectProtos(devices):
    global sensorType
    bleDevices = []
    for dev in devices:
        log.debug("Device %s (%s), RSSI=%d dB" % (dev.addr, dev.addrType, dev.rssi))
        for (adtype, desc, value) in dev.getScanData():
            log.debug("  %s = %s" % (desc, value))
            if desc == "Complete Local Name":
                if value == "Proto-Fever":
                    log.info("Proto-Fever Sensor Found")
                    device = Peripheral(dev.addr, btle.ADDR_TYPE_PUBLIC)
                    device.setMTU(50)
                    # ble_device = ProtoBle(dev.addr, value, device.getServices())
                    bleSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
                    bleService = device.getServiceByUUID(bleSensor)
                    bleChar = bleService.getCharacteristics()
                    data = bleChar[0].read()
                    data_decoded = data.decode("utf-8")
                    temperature = float(data_decoded)
                    log.debug('Temperature: %s', temperature)
                    # Sending data to GUI
                    try:
                        now = time.time()
                        # with open(DATA_PATH+FEVER_FILE, 'a') as f:
                            # writer = csv.writer(f, delimiter=',')
                            # msg_formatted = [now, temperature]
                            # writer.writerow(msg_formatted)
                        fever_msg = {'data': temperature, 'timestamp': now}
                        topic = "fever/reply/meas"
                        log.info("MQTT: publishing message to topic %s", topic)
                        client.publish(topic, json.dumps(fever_msg))
                        device.disconnect()
                        device = None
                    except Exception as e: 
                        log.error(e)
                    # # Enabling the BLE Server notifications
                    # notify_handle = bleChar[0].valHandle+1
                    # device.writeCharacteristic(notify_handle, bytes.fromhex("0100"))
                    # device.withDelegate(ProtoFeverDelegate(device))
                    # bleChar[1].write(bytes("50", 'ascii'))
                    # log.debug("\n\n")
                    # bleDevices.append(device)
                if value == "Proto-Light":
                    log.info("Proto-Bili Sensor Found")
                    device = Peripheral(dev.addr)
                    device.setMTU(60)
                    device.name = "Proto-Light"
                    bleSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
                    bleService = device.getServiceByUUID(bleSensor)
                    bleChar = bleService.getCharacteristics()
                    log.debug('Read: %s', bleChar[0].read())
                    # Enabling the BLE Server notifications
                    notify_handle = bleChar[0].valHandle+1
                    device.writeCharacteristic(notify_handle, bytes.fromhex("0100"))
                    device.withDelegate(ProtoBiliDelegate())
                    bleChar[1].write(bytes("50", 'ascii'))
                    log.debug("\n\n")
                    bleDevices.append(device)
                if value == "Proto-Beat":
                    log.info("Proto-Beat Sensor Found")
                    device = Peripheral(dev.addr)
                    device.setMTU(50)
                    device.name = "Proto-Beat"
                    bleSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
                    bleService = device.getServiceByUUID(bleSensor)
                    bleChar = bleService.getCharacteristics()
                    log.debug('Read: %s', bleChar[0].read())
                    # Enabling the BLE Server notifications
                    notify_handle = bleChar[0].valHandle+1
                    device.writeCharacteristic(notify_handle, bytes.fromhex("0100"))
                    device.withDelegate(ProtoBeatDelegate())
                    bleChar[1].write(bytes("50", 'ascii'))
                    log.debug("\n\n")
                    bleDevices.append(device)

    return bleDevices
                


# scanner = Scanner().withDelegate(ScanDelegate())
# scanner.clear()
# scanner.start()
# scanner.process(2.0)
# scanner.stop()
# devices = scanner.getDevices()
# devices = scanner.scan(2.0)
# FoundDevices = detectProtos(devices)
# # scanner.clear()
# log.debug(FoundDevices)

FoundDevices = []

log.info("Proto-Beat Sensor Configuration")
device = Peripheral("7c:9e:bd:47:c0:da")
device.setMTU(50)
device.name = "Proto-Beat"
bleSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
bleService = device.getServiceByUUID(bleSensor)
bleChar = bleService.getCharacteristics()
log.debug('Read: %s', bleChar[0].read())
# Enabling the BLE Server notifications
notify_handle = bleChar[0].valHandle+1
device.writeCharacteristic(notify_handle, bytes.fromhex("0100"))
device.withDelegate(ProtoBeatDelegate())
bleChar[1].write(bytes("50", 'ascii'))
FoundDevices.append(device)
device = []
log.debug("\n\n")


log.info("Proto-Fever Sensor Configuration")
device = Peripheral("7c:9e:bd:47:b3:fa", btle.ADDR_TYPE_PUBLIC)
device.setMTU(50)
device.name = "Proto-Fever"
device.withDelegate(ProtoFeverDelegate())
# ble_device = ProtoBle(dev.addr, value, device.getServices())
bleSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
bleService = device.getServiceByUUID(bleSensor)
bleChar = bleService.getCharacteristics()
data = bleChar[0].read()
data_decoded = data.decode("utf-8")
temperature = float(data_decoded)
log.debug('Temperature: %s', temperature)
# Sending data to GUI
try:
    now = time.time()
    # with open(DATA_PATH+FEVER_FILE, 'a') as f:
        # writer = csv.writer(f, delimiter=',')
        # msg_formatted = [now, temperature]
        # writer.writerow(msg_formatted)
    fever_msg = {'data': temperature, 'timestamp': now}
    topic = "fever/reply/meas"
    log.info("MQTT: publishing message to topic %s", topic)
    client.publish(topic, json.dumps(fever_msg))
    # device.disconnect()
    # device = None
except Exception as e: 
    log.error(e)
FoundDevices.append(device)
device = []


# 7c:9e:bd:47:b3:fa

# scannertmp = Scanner().withDelegate(ScanDelegate())

while True:

    try:
        # log.info("BLE Scanning for ProtoDevices")
        # devices = scanner.scan(1, passive=True)
        # # scanner.start()
        # # scanner.process(scannertmp)
        # # scanner.stop()
        # FoundDevicesTmp = detectProtos(devices)
        # # scanner.clear()

        # time.sleep(3)
        if FoundDevices:
            # if FoundDevicesTmp:
            #     for ProtoDeviceTmp in FoundDevicesTmp:
            #         FoundDevices.append(ProtoDeviceTmp)
            for ProtoDevice in FoundDevices:
                # log.debug(ProtoDevice.name)
                # log.debug("Sensors already found, waiting notifications")
                ProtoDevice.waitForNotifications(2)
                if ProtoDevice.name == "Proto-Beat":
                    if sendSamples:
                        log.info("ProtoBeat: configuration of %s samples sent to device" % (samples))
                        bleSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
                        bleService = ProtoDevice.getServiceByUUID(bleSensor)
                        bleChar = bleService.getCharacteristics()
                        # bleChar[1].write(str(samples).encode())
                        # bleChar[1].write(bytes([samples]))
                        bleChar[1].write(bytes(str(samples), 'ascii'))
                        # bleChar[1].write(bytes.fromhex(samples))
                        sendSamples = False
        # else:
        #     log.info("BLE Scanning for ProtoDevices")
        #     devices = scanner.scan(2.0)
        #     FoundDevices = detectProtos(devices)
        #     time.sleep(2)
    except KeyboardInterrupt:
        log.error("User interruption, disconnecting BLE device..")
        if FoundDevices:
            for ProtoDevice in FoundDevices:
                ProtoDevice.disconnect()
            FoundDevices = []
        # scanner.stop()
        sys.exit(0)
    except Exception as e: 
        log.error(e)
        log.error("System error, disconnecting BLE device..")
        # if FoundDevices:
        #     for ProtoDevice in FoundDevices:
        #         ProtoDevice.disconnect()
        #     FoundDevices = []
        # sys.exit(0)