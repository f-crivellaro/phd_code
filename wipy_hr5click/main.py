import pycom
import time
import machine
from machine import Pin,I2C
from network import WLAN
from mqtt import MQTTClient
import afe

print('\n\n')
print('--------------------')
print('PPG Node Begin')
print('--------------------')
print('\n')

afe.start()
afe.config()

wlan = WLAN(mode=WLAN.STA)
wlan.connect("Smart Devices", auth=(WLAN.WPA2, "comptaroot"), timeout=5000)

while not wlan.isconnected():  
    machine.idle()
print("\nConnected to WiFi\n")
time.sleep(1)

def sub_cb(topic, msg):
   print(msg)

client = MQTTClient("wipy", "192.168.0.252", port=1883)
client.set_callback(sub_cb)
print(client.connect())
time.sleep(1)
client.subscribe(topic="wipy/command")

while True:
    print('\n')
    print(afe.read(0x2A))
    print(afe.read(0x2B))
    print(afe.read(0x2C))
    print(afe.read(0x2D))
    print(afe.read(0x2E))
    print(afe.read(0x2F))
    print('\n-------------')
    print("Sending Msg")
    client.publish(topic="wipy/reply", msg=str(afe.read(0x2D)))
    time.sleep(1)



