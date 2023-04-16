import logging
import logging.handlers
import os
import sys
import paho.mqtt.client as mqtt
import numpy as np
import json
from bluepy.btle import Scanner, DefaultDelegate, Peripheral, UUID
import time
import csv
from datetime import datetime
from scipy import signal
import statistics


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
topic_quality = "spec/cmd/quality"
topic_cmd = "spec/cmd/meas"
topic_beat = "protobeat/command"
topic_fever = "fever/command"
topic_bili = "protobili/command"
log.info("Subscribing to topic %s" %topic_cmd)
log.info("Subscribing to topic %s" %topic_beat)
log.info("Subscribing to topic %s" %topic_fever)
log.info("Subscribing to topic %s" %topic_bili)
client.subscribe(topic_quality)
client.subscribe(topic_cmd)
client.subscribe(topic_beat)
client.subscribe(topic_fever)
client.subscribe(topic_bili)


last_quality_msg = {}
def measure_quality(client, userdata, message):
    log.info('Quality measurement request callback')
    topic = "spec/reply/quality"
    log.info("MQTT: publishing message to topic %s", topic)
    client.publish(topic, json.dumps(last_quality_msg))

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
    log.info(f'Beat request callback: msg={msg}')
    log.debug(f'Samples to acquire: {msg["duration"]}')
    samples = msg["duration"]
    sendSamples = True
    # bleChar[1].write(str(msg["duration"]).encode())
    # topic = "wipy/reply"
    # log.info("MQTT: publishing message to topic %s - msg %s", topic, last_beat_msg)
    # client.publish(topic, json.dumps(last_beat_msg))

send_bili_config = False
bili_settings = {}
def bili_config(client, userdata, message):
    global send_bili_config, bili_settings
    msg = json.loads(message.payload.decode("utf-8"))
    log.info(f'ProtoBili request callback: msg={msg}')
    log.debug(f'Atime: {msg["Atime"]}')
    log.debug(f'Astep: {msg["Astep"]}')
    log.debug(f'Again: {msg["Again"]}')
    log.debug(f'controlLed: {msg["controlLed"]}')
    bili_settings["Atime"] = msg["Atime"]
    bili_settings["Astep"] = msg["Astep"]
    bili_settings["Again"] = msg["Again"]
    bili_settings["controlLed"] = msg["controlLed"]
    send_bili_config = True



client.message_callback_add(topic_quality, measure_quality)
client.message_callback_add(topic_cmd, measure_request)
client.message_callback_add(topic_beat, beat_request)
client.message_callback_add(topic_bili, bili_config)
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

class MyDelegate(DefaultDelegate):
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
        log.info("A notification was received: %s", data_decoded)
        if sensorType == "Proto-Quality":
            measures = data_decoded
            measures_splitted = measures.split('/')
            measures_formatted = [int(a) for a in measures_splitted]
            # log.debug('Measured Splited: %s', measures_formatted)
            wavelengths = [415, 445, 480, 515, 555, 590, 630, 680, 910]
            msg = {'data': measures_formatted, 'wavelengths': wavelengths}
            last_quality_msg = msg
            topic = "spec/reply/quality"
            log.info("MQTT: publishing message to topic %s", topic)
            client.publish(topic, json.dumps(last_quality_msg))
        if sensorType == "Proto-Light":
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
        if sensorType == "Proto-Beat":
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
                    log.info('Proto-Beat ready to be sent')
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
                    self._beat_timestamps = []
                    self._beat_array = []
                    self._beat_array_red = []
                    self._beat_array_amb1 = []
                    self._beat_array_amb2 = []
                    self._beat_start = None
        if sensorType == "Proto-Fever":
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
                ProtoDevice.disconnect()
                ProtoDevice = None
            except Exception as e: 
                log.error(e)




# ----------------------------------------------------------------------------------------------------------------------

def detectProtos(devices):
    global sensorType
    for dev in devices:
        # log.debug("Device %s (%s), RSSI=%d dB" % (dev.addr, dev.addrType, dev.rssi))
        for (adtype, desc, value) in dev.getScanData():
            # log.debug("  %s = %s" % (desc, value))
            if desc == "Complete Local Name":
                if value == "Proto-Fever" or value == "Proto-Light" or value == "Proto-Beat" or value == "Proto-Quality":
                    sensorType = value
                    device = Peripheral(dev.addr)
                    device.setMTU(60)
                    device.setDelegate(MyDelegate())
                    ble_device = ProtoBle(dev.addr, value, device.getServices())
                    for svc in ble_device.services:
                        log.debug(svc)
                    bleSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
                    bleService = device.getServiceByUUID(bleSensor)
                    bleChar = bleService.getCharacteristics()
                    for ch in bleChar:
                        log.debug('Properties: %s', ch.propertiesToString())
                    log.debug('Read: %s', bleChar[0].read())
                    device.writeCharacteristic(bleChar[0].valHandle+1, bytes.fromhex("0100"))
                    # bleChar[1].write(bytes.fromhex("0a"))
                    bleChar[1].write(bytes("50", 'ascii'))
                    # bleChar[1].write(bytes([0x0f]))
                    # device.writeCharacteristic(bleChar[1].valHandle+1, bytes.fromhex("3130"))
                    log.debug("\n\n")
                    
                    return device
    return None
                


scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(2.0)
ProtoDevice = detectProtos(devices)

while True:

    try:
        if ProtoDevice:
            ProtoDevice.waitForNotifications(1.0)
            if sendSamples:
                bleSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
                bleService = ProtoDevice.getServiceByUUID(bleSensor)
                bleChar = bleService.getCharacteristics()
                # bleChar[1].write(str(samples).encode())
                # bleChar[1].write(bytes([samples]))
                bleChar[1].write(bytes(str(samples), 'ascii'))
                # bleChar[1].write(bytes.fromhex(samples))
                sendSamples = False
            if send_bili_config:
                bleSensor = UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
                bleService = ProtoDevice.getServiceByUUID(bleSensor)
                bleChar = bleService.getCharacteristics()
                # bleChar[1].write(str(samples).encode())
                # bleChar[1].write(bytes([samples]))
                bleChar[1].write(bytes(str(bili_settings), 'ascii'))
                # bleChar[1].write(bytes.fromhex(samples))
                send_bili_config = False
        else:
            log.info("BLE Scanning for ProtoDevices")
            devices = scanner.scan(2.0)
            ProtoDevice = detectProtos(devices)
            time.sleep(2)
    except KeyboardInterrupt:
        if ProtoDevice:
            ProtoDevice.disconnect()
        ProtoDevice = None
        sys.exit(0)
    except Exception as e: 
        log.error(e)
        log.error("System error, disconnecting BLE device..")
        if ProtoDevice:
            ProtoDevice.disconnect()
        ProtoDevice = None
        # sys.exit(0)