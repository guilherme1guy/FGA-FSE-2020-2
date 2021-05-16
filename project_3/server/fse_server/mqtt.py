import paho.mqtt.client as mqtt

import json
import os
import django

# allows importing Models and Settings from django from
# the script part at the end of the file
os.environ["DJANGO_SETTINGS_MODULE"] = "fse_server.settings"
django.setup()

from fse_server import settings
from fse_server.models import DeviceRegisterRequest, Device, Location

SELF_SENDER_NAME = "server"


def send_location_to_device(device_mac, location_name):
    topic = settings.MQTT_BASE_TOPIC + f"dispositivos/{device_mac}"

    client.publish(
        topic,
        json.dumps(
            {
                "sender": SELF_SENDER_NAME,
                "location": location_name,
            }
        ),
    )


def send_unregister_to_device(device):
    topic = settings.MQTT_BASE_TOPIC + f"dispositivos/{device}"

    client.publish(
        topic,
        json.dumps({"sender": SELF_SENDER_NAME, "unregister": True}),
    )


def on_connect(client: mqtt.Client, userdata, flags, rc, properties=None):
    client.subscribe(settings.MQTT_BASE_TOPIC + "#")


def route_dispositivos(client: mqtt.Client, userdata, msg: mqtt.MQTTMessage):
    payload = json.loads(msg.payload.decode())

    if payload["sender"] == SELF_SENDER_NAME:
        # don't handle self messages
        return

    mac_address = payload["sender"]

    # check if an register request already exists
    request_search = DeviceRegisterRequest.objects.filter(id=mac_address)
    if request_search.count() > 0:
        register_request = request_search.first()

        # if user completed registration, inform device
        if register_request.device is not None:
            send_location_to_device(mac_address, register_request.device.location.name)

    else:
        # create register request
        register_request = DeviceRegisterRequest(
            id=mac_address,
            is_dht_present=payload["is_dht_present"],
            io_info=json.dumps(payload["io_info"]),
        )

        register_request.save()


def route_location(client: mqtt.Client, userdata, msg: mqtt.MQTTMessage):
    payload = json.loads(msg.payload.decode())
    topic_structure = msg.topic.split("/")

    device_mac = payload["sender"]

    device = Device.objects.filter(id=device_mac).first()
    if device is None or device.location.name != topic_structure[2]:
        # device not registered/deleted
        # or in wrong location
        # re route to invalid_route
        route_invalid(client, userdata, msg)
        return

    if topic_structure[3] == "temperatura":
        device.set_temperature(payload["value"])
    elif topic_structure[3] == "umidade":
        device.set_humidity(payload["value"])
    elif topic_structure[3] == "estado":

        if "gpio" in payload:

            changed_gpio = payload["gpio"]
            input = device.inputs.filter(gpio_id=changed_gpio).first()
            input.update_state(payload["value"])

    # TODO: sync device with server output states


def route_invalid(client: mqtt.Client, userdata, msg: mqtt.MQTTMessage):

    payload = json.loads(msg.payload.decode())
    topic_structure = msg.topic.split("/")

    if payload["sender"] == SELF_SENDER_NAME:
        # don't handle self messages
        return

    # verify if device is registered
    device = Device.objects.filter(id=payload["sender"]).first()
    if device is None:
        # send unregister to clean device
        send_unregister_to_device(payload["sender"])
    else:
        # user changed device room, update this data on device
        send_location_to_device(device.id, device.location.name)


def on_message(client: mqtt.Client, userdata, msg: mqtt.MQTTMessage):
    topic_structure = msg.topic.split("/")

    # example topic structure
    # topic_structure[0] == 'fse2020'
    # topic_structure[1] == '160123186'

    routes = {
        "dispositivos": route_dispositivos,
    }

    # populate with registered locations
    for location in Location.objects.all():
        routes[location.name] = route_location

    # dispatch
    topic = topic_structure[2]
    if topic in routes:
        routes[topic](client, userdata, msg)
    else:
        # invalid topic
        route_invalid(client, userdata, msg)


# script part
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(settings.MQTT_HOST, settings.MQTT_PORT, 60)
