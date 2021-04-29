import pycom
import time
from machine import Pin,I2C


reset_pin = Pin('P5', mode=Pin.OUT)
i2c_afe_id = None
i2c_bus = None

def reset():
    global reset_pin
    print('AFE4404 - Resetting started')
    reset_pin.value(1)
    reset_pin.value(0)
    time.sleep(0.000025)
    reset_pin.value(1)
    print('AFE4404 - Resetting done')

def i2c_init():
    global i2c_afe_id
    print('AFE4404 - I2C configuration started')
    i2c = I2C(0, I2C.MASTER)
    i2c.init(I2C.MASTER, baudrate=400000)
    i2c_slaves = i2c.scan()
    print('AFE4404 - I2C founded slaves: %s' % i2c_slaves)
    for x in i2c_slaves:
        if x == 88:
            print('AFE4404 - Device is connected!')
            i2c_afe_id = x
            time.sleep(1)
            print('AFE4404 - I2C configuration ended')
            return i2c
        else:
            print('Error: AFE4404 is not connected!')
            return 'error'

def start():
    global i2c_bus
    print('*** System configuration started ***\n')
    pycom.heartbeat(True)
    reset()
    i2c_bus = i2c_init()
    if i2c_bus == 'error':
        print('\nSystem configuration failed\n')
        return 0
    else:
        print('\n*** System configuration ended ***\n')
        return 1

def write(addr, data):
    global i2c_afe_id, i2c_bus
    buf = bytearray(3)
    buf[0], mod = divmod(data, 0x10000)
    buf[1], buf[2]  = divmod(mod, 0x100)
    # buf[0] = data[0]
    # buf[1] = data[1]
    # buf[2] = data[2]
    nbytes = i2c_bus.writeto_mem(i2c_afe_id, addr, buf, addrsize=8)
    # time.sleep(1)
    print('N bytes written to AFE4404: %d' % nbytes)

def read(addr):
    data = bytearray(3)
    nbytes = i2c_bus.readfrom_mem_into(i2c_afe_id, addr, data, addrsize=8)
    print('N bytes read: %d' % nbytes)
    print('Data Read from I2C Address 0x' + hex(addr) + ': ' + str(data))
    return data

def pin_handler(arg):
    print("Interrupt received")
    print(read(0x2A))
    print(read(0x2B))
    print(read(0x2C))
    print(read(0x2D))
    print(read(0x2E))
    print(read(0x2F))
    print('\n-------------\n')
    # time.sleep(1)

p_in = Pin('P6', mode=Pin.IN, pull=Pin.PULL_UP)
p_in.callback(Pin.IRQ_RISING, pin_handler)

def config():
    global i2c_afe_id, i2c_bus
    print("Enabling Register write mode")
    # write(0x00, [0x00, 0x00, 0x00])
    write(0x00, 0)

    print("Enable internal oscillator (4 MHz)")
    # write(0x23, [0x00, 0x02, 0x00])
    write(0x23, 131584)

    print("Enable the readout of write registers")
    # write(0x00, [0x00, 0x00, 0x01])
    write(0x00, 1)

    print("Read Addr 0x23 to check OSC_ENABLE (bit 9)")
    if read(0x23) == bytearray([0x00, 0x02, 0x00]):
        print('AFE4404 internal oscillator (4 MHz) configured')

    print('\n')

    print("AFE4404 - Final config started")
    write(0x39, 0)
    write(0x1D, 39999)
    write(0x09, 0)
    write(0x0A, 398)
    write(0x01, 100)
    write(0x02, 398)
    write(0x15, 5600)
    write(0x16, 5606)
    write(0x0D, 5608)
    write(0x0E, 6067)
    write(0x36, 400)
    write(0x37, 798)
    write(0x05, 500)
    write(0x06, 798)
    write(0x17, 6069)
    write(0x18, 6075)
    write(0x0F, 6077)
    write(0x10, 6536)
    write(0x03, 800)
    write(0x04, 1198)
    write(0x07, 900)
    write(0x08, 1198)
    write(0x19, 6538)
    write(0x1A, 6544)
    write(0x11, 6546)
    write(0x12, 7006)
    write(0x0B, 1300)
    write(0x0C, 1598)
    write(0x1B, 7008)
    write(0x1C, 7014)
    write(0x13, 7016)
    write(0x14, 7475)
    write(0x32, 7675)
    write(0x33, 39199)

    write(0x1E, 258)
    write(0x20, 32772)
    write(0x21, 3)
    write(0x22, 12495)
    print("AFE4404 - Final config ended")

