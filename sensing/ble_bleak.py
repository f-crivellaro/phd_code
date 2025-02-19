import asyncio
import sys
from bleak import BleakScanner, BleakClient
from paho.mqtt import client as mqtt_client
import json
from universe import AdaLovelace
import random

DEVICE_NAME = "Proto-Quality"
SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
CHARACTERISTIC_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  # Notification characteristic

sensorType = ""
sensor = AdaLovelace()

broker = 'raspberrypi'
port = 1883
client_id = f'python-ble-{random.randint(0, 1000)}'
TOPIC_CMD_START = "ble/cmd/start"
TOPIC_CMD_QUALITY = "spec/cmd/quality"
TOPIC_CMD_MEASUREMENT = "spec/cmd/meas"
# topic_beat = "protobeat/command"
# topic_fever = "fever/command"
# topic_bili = "protobili/command"

def connect_mqtt():
    global client
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print(f"Failed to connect, return code {rc}\n")

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.connect(broker, port, keepalive=10)
    return client

def publish(client, topic, msg):

    result = client.publish(topic, msg)
    # result: [0, 1]
    status = result[0]
    if status == 0:
        print(f"Send `{msg}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")


def subscribe(client: mqtt_client, topic: str):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")

    print(f'Subscribing to topic {topic}')
    client.subscribe(topic)
    client.on_message = on_message

def run(client: mqtt_client):
    client.loop_start()


last_quality_msg = {}
def measure_quality(client, userdata, message):
    print('Quality measurement request callback')
    topic = "spec/reply/quality"
    print(f"MQTT: publishing message to topic {topic}", )
    client.publish(topic, json.dumps(last_quality_msg))

# last_msg = {}
def measure_request(client, userdata, message):
    print('Measure request callback')
    topic = "spec/reply/meas"
    print("MQTT: publishing message to topic %s", topic)
    client.publish(topic, json.dumps(last_quality_msg))


print(f"BLE creating MQTT client")
try:
    mqtt_ble_client = connect_mqtt()
    subscribe(mqtt_ble_client, TOPIC_CMD_QUALITY)
    mqtt_ble_client.message_callback_add(TOPIC_CMD_QUALITY, measure_quality)
    mqtt_ble_client.message_callback_add(TOPIC_CMD_MEASUREMENT, measure_request)
    # Starts the Spectrometer Driver
    publish(mqtt_ble_client, TOPIC_CMD_START, "start")
    run(mqtt_ble_client)
except Exception as e:
    print(f"BLE MQTT client creation error: {e}")
    # return False




async def find_device():
    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover()
    for device in devices:
        if device.name == DEVICE_NAME:
            print(f"Found device: {device.name} [{device.address}]")
            return device.address
    print("Device not found.")
    return None

def notification_handler(sender, data, device_name):
    """ Callback function triggered when notification is received """
    print(f"\nNotification from {device_name}: {data.decode(errors='ignore')}")

    measures = data.decode(errors='ignore')
    measures_splitted = measures.split('/')
    measures_formatted = [int(a) for a in measures_splitted]
    # log.debug('Measured Splited: %s', measures_formatted)
    wavelengths = [415, 445, 480, 515, 555, 590, 630, 680, 910]
    msg_pre = {'data': measures_formatted, 'wavelengths': wavelengths}
    compounds = sensor.compounds_calculation(msg_pre)
    msg = {'data': measures_formatted, 'wavelengths': wavelengths, 'compounds': compounds.tolist()}
    last_quality_msg = msg
    topic = "spec/reply/quality"
    # Call a function to calculate physiological parameters (different file)
    print(f"MQTT: publishing message to topic {topic}")
    client.publish(topic, json.dumps(last_quality_msg))

async def subscribe_to_notifications(address):
    async with BleakClient(address) as client:
        print(f"Connected to {DEVICE_NAME}")

        # Get available services & characteristics
        services = client.services
        service = services.get_service(SERVICE_UUID)

        if not service:
            print(f"Service {SERVICE_UUID} not found.")
            return

        # Find and subscribe to characteristic
        characteristic = next((char for char in service.characteristics if char.uuid.lower() == CHARACTERISTIC_UUID.lower()), None)

        if characteristic and "notify" in characteristic.properties:
            print(f"Subscribing to notifications for {CHARACTERISTIC_UUID}...")

            # Use lambda to pass the device name to the handler
            await client.start_notify(CHARACTERISTIC_UUID, lambda sender, data: notification_handler(sender, data, DEVICE_NAME))

            print("Waiting for notifications... (Press Ctrl+C to stop)")
            while True:
                await asyncio.sleep(1)  # Keep script running to receive notifications
        else:
            print(f"Characteristic {CHARACTERISTIC_UUID} does not support notifications.")

async def main():
    address = await find_device()
    if address:
        await subscribe_to_notifications(address)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nStopped by user.")
        sys.exit(0)