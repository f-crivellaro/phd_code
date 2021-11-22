import pycom
import time
import machine
from machine import Pin,I2C, Timer, RTC
from network import WLAN
from mqtt import MQTTClient
import array as arr
import json
import afe

measure_requested = 0
measurements = []
timestamps = []
measurement_timestamp = 0
chrono = Timer.Chrono()

print('\n\n')
print('--------------------')
print('PPG Node Begin')
print('--------------------')
print('\n')

# afe.start()
# afe.config()

wlan = WLAN(mode=WLAN.STA)
wlan.connect("Smart Devices", auth=(WLAN.WPA2, "Compta202004"), timeout=5000)

print('\nLooking for Wifi networks...')
while not wlan.isconnected():  
    machine.idle()
print("Connected to WiFi\n")
print(wlan.ifconfig())
time.sleep(1)

def alarm_handler(alarm):
    global measure_requested, measurements, timestamps, measurement_timestamp
    measure_requested = 0
    print('Measurement ended')
    print('Sending data')
    client.publish(topic="wipy/reply", msg=json.dumps({'series': ['PPG'], 'data': [measurements], 'labels': timestamps}))
    print('Data sent')
    measurements = []
    measurement_timestamp = 0
    timestamps = []
    chrono.stop()
    chrono.reset()
    alarm.cancel()


def sub_cb(topic, msg):
    global measure_requested
    print('MQTT message received in topic: ' + topic.decode('utf-8'))
    print('MQTT message received with payload: ' + msg.decode('utf-8'))
    message = json.loads(msg.decode('utf-8'))
    if message['action'] == "start":
        chrono.start()
        measure_requested = 1
        measure_alarm = Timer.Alarm(alarm_handler, message['duration'])
        print('MQTT measurement requested')


print('\nMQTT configuration started')
try:
    client = MQTTClient("wipy", "192.168.0.177", port=1883)
    client.set_callback(sub_cb)
    if client.connect() == 0:
        print('MQTT client connected')
    else:
        print('MQTT client connection error')
    time.sleep(1)
    client.subscribe(topic="wipy/command")
    print('MQTT configuration ended')
except:
    print("MQTT Error!")


def pin_handler(arg):
    global measure_requested, measurements, measurement_timestamp, timestamps
    if measure_requested:
        measurement_timestamp = chrono.read()
        timestamps.append(measurement_timestamp)
        measurements.append(afe.read_adc(0x2F))
        print('Measured ' + str(afe.read_adc(0x2F)) + ' at ' + str(measurement_timestamp))
        measure_start = time.ticks_ms()
        # client.publish(topic="wipy/reply", msg=str(afe.read_adc(0x2F)))

p_in = Pin('P6', mode=Pin.IN, pull=Pin.PULL_UP)
p_in.callback(Pin.IRQ_RISING, pin_handler)

while True:
    try:
        client.check_msg()
        # print('\n')
        # print('LED2VAL: ' + str(afe.read_adc(0x2A)))
        # print('ALED2VAL\LED3VAL: ' + str(afe.read_adc(0x2B)))
        # print('LED1VAL: ' + str(afe.read_adc(0x2C)))
        # print('ALED1VAL: ' + str(afe.read_adc(0x2D)))
        # print('LED2-ALED2VAL: ' + str(afe.read_adc(0x2E)))
        # print('LED1-ALED1VAL: ' + str(afe.read_adc(0x2F)))
        # print('-------------')
        # client.publish(topic="wipy/reply", msg=str(afe.read_adc(0x2D)))
        # print("Msg sent")
        # print('-------------')
        time.sleep(0.1)
    except:
        print("System Error")
        time.sleep(5)
        print("Entering in Deep Sleep")
        machine.deepsleep(10000)

