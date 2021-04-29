import pycom
import time
from machine import Pin,I2C
import afe

print('\n\n')
print('--------------------')
print('PPG Node Begin')
print('--------------------')
print('\n')

afe.start()
afe.config()

while True:
    # #colors in hexadecimal (0xRRGGBB)
    # pycom.rgbled(0xFF0000)  # Red
    # time.sleep(1)
    # pycom.rgbled(0x00FF00)  # Green
    # time.sleep(1)
    # pycom.rgbled(0xCC0066)  # Pink
    # time.sleep(1)
    # pycom.rgbled(0x0000FF)  # Blue
    # time.sleep(1)
    # pycom.rgbled(0x330033)  # Purple
    # time.sleep(1)
    # pycom.rgbled(0x999900)  # Yellow
    # print('\n')
    # print(afe.read(0x2A))
    # print(afe.read(0x2B))
    # print(afe.read(0x2C))
    # print(afe.read(0x2D))
    # print(afe.read(0x2E))
    # print(afe.read(0x2F))
    # print('\n-------------')
    time.sleep(0.1)



