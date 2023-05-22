#/bin/python3

import paho.mqtt.client as mqtt
from paho.mqtt.properties import Properties
from paho.mqtt.packettypes import PacketTypes

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print("Failed to connect, return code %d\n", rc)

def on_message(client, userdata, msg):
    print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")

    client.publish('rtt_sub',"hello word 123")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.100.41",1883)


properties = Properties(PacketTypes.PUBLISH)
properties.MessageExpiryInterval=30

#c.publish('rtt_sub',"hello word 123")#,2,properties=properties)

client.subscribe('rtt_pub')
client.loop_forever()