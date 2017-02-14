# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""
import paho.mqtt.client as mqtt
import time

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    #client.subscribe("$SYS/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    
    

mqttClient = mqtt.Client(client_id="python_client", clean_session=True, userdata=None, protocol="MQTTv311")

mqttClient.on_connect = on_connect
mqttClient.on_message = on_message

mqttClient.connect("192.168.1.92",port=1883,keepalive=60)

counter = 0

#mqttClient.loop_start()
while True:
    counter = counter + 1
    mqttClient.publish("home/py", payload=counter, qos=0, retain=False)
    mqttClient.subscribe("home/temp", 0)
    time.sleep(2)
    mqttClient.loop()
    
#mqttClient.loop_forever()



    

