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

print('\nLooking for Wifi networks...')
while not wlan.isconnected():  
    machine.idle()
print("Connected to WiFi\n")
time.sleep(1)

def sub_cb(topic, msg):
   print(msg)

print('\nMQTT configuration started')
client = MQTTClient("wipy", "192.168.0.252", port=1883)
client.set_callback(sub_cb)
if client.connect() == 0:
    print('MQTT client connected')
else:
    print('MQTT client connection error')
time.sleep(1)
client.subscribe(topic="wipy/command")
print('MQTT configuration ended')


def pin_handler(arg):
    client.publish(topic="wipy/reply", msg=str(afe.read_adc(0x2F)))

p_in = Pin('P6', mode=Pin.IN, pull=Pin.PULL_UP)
p_in.callback(Pin.IRQ_RISING, pin_handler)

while True:
    continue
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
    # time.sleep(0.001)
