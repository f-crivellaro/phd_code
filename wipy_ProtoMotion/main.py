import pycom
import time
import machine
from machine import Pin,I2C, Timer, RTC
from network import WLAN
from mqtt import MQTTClient
import array as arr
import json

pycom.bootmgr(safeboot=True)

measure_requested = 0
measurements = []
timestamps = []
measurement_timestamp = 0
chrono = Timer.Chrono()
mqtt_connected = False

print('\n\n')
print('--------------------')
print('ProtoMotion')
print('--------------------')
print('\n')

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
    client = MQTTClient("wipy", "192.168.0.177", port=1883, keepalive= 100)
    if client.connect(True) == 0:
        print('MQTT client connected')
        client.set_callback(sub_cb)
        client.subscribe(topic="wipy/command")
        mqtt_connected = True
        print('MQTT configuration ended')
    else:
        client.disconnect()
        print('MQTT client connection error')
    time.sleep(1)
except Exception as e:
    print("MQTT Error!")
    print(e)


while True:
    try:
        if mqtt_connected:
            client.ping()
            client.check_msg()
        # else:
        #     print('\nNew MQTT Connection Trial')
        #     try:
        #         if client.connect() == 0:
        #             print('MQTT client connected')
        #             client.subscribe(topic="wipy/command")
        #             print('MQTT Connected')
        #             mqtt_connected = True
        #         else:
        #             print('MQTT client connection error')
        #     except:
        #         print("MQTT Error!")
            
        time.sleep(0.1)
    except:
        print("System Error")
        time.sleep(5)
        print("Entering in Deep Sleep")
        machine.deepsleep(10000)

