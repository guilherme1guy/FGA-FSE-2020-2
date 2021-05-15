from socket import TIPC_SUB_PORTS
import paho.mqtt.client as mqtt

import json
import time
import os
import random

SELF_SENDER_NAME = "abababab"
MQTT_BASE_TOPIC = "fse2020/160123186/"
MQTT_HOST = "test.mosquitto.org"
MQTT_PORT = 1883

default_mem = {"registered": False, "location": ""}

registered = False
location = ""


def mqtt_publish(client: mqtt.Client, topic, msg):

    print(f">>>>>{topic}: \n\t{msg}")
    client.publish(topic, msg)


def get_from_nvs(key):

    with open("nvs.json", "r") as file:
        mem_data = json.load(file)

        return mem_data[key]


def set_on_nvs(key, value):

    mem_data = None
    with open("nvs.json", "r") as file:
        mem_data = json.load(file)

    mem_data[key] = value

    with open("nvs.json", "w") as file:
        json.dump(mem_data, file)


def on_connect(client: mqtt.Client, userdata, flags, rc, properties=None):
    client.subscribe(MQTT_BASE_TOPIC + "#")


def on_message(client: mqtt.Client, userdata, msg: mqtt.MQTTMessage):

    global location
    global registered

    topic_structure = msg.topic.split("/")
    payload = json.loads(msg.payload.decode())

    if payload["sender"] == SELF_SENDER_NAME:
        return

    print(f"<<<<<{msg.topic}: \n\t{msg.payload.decode()}")

    if "location" in payload and not registered:

        set_on_nvs("location", payload["location"])
        set_on_nvs("registered", True)

        location = payload["location"]
        registered = True

    elif "unregister" in payload and payload["unregister"] == True:

        set_on_nvs("location", default_mem["location"])
        set_on_nvs("registered", default_mem[registered])

        location = default_mem["location"]
        registered = False


if __name__ == "__main__":

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    if not os.path.exists("nvs.json"):
        with open("nvs.json", "w") as file:
            json.dump(default_mem, file)

    client.connect(MQTT_HOST, MQTT_PORT, 60)
    client.subscribe(MQTT_BASE_TOPIC + f"dispositivos/{SELF_SENDER_NAME}")
    client.loop_start()

    while not registered:
        mqtt_publish(
            client,
            MQTT_BASE_TOPIC + f"dispositivos/{SELF_SENDER_NAME}",
            json.dumps(
                {
                    "sender": SELF_SENDER_NAME,
                    "is_dht_present": True,
                    "io_info": {"input": [0, 2], "output": [2]},
                }
            ),
        )

        time.sleep(1)

    while True:

        mqtt_publish(
            client,
            MQTT_BASE_TOPIC + f"{location}/temperatura",
            json.dumps({"sender": SELF_SENDER_NAME, "value": random.randint(20, 35)}),
        )

        mqtt_publish(
            client,
            MQTT_BASE_TOPIC + f"{location}/umidade",
            json.dumps({"sender": SELF_SENDER_NAME, "value": random.randint(0, 100)}),
        )

        mqtt_publish(
            client,
            MQTT_BASE_TOPIC + f"{location}/estado",
            json.dumps({"sender": SELF_SENDER_NAME, "value": random.randint(0, 1)}),
        )

        time.sleep(2)
