# -*- coding: utf-8 -*-
"""
Created on Sun Oct 22 22:49:17 2017

@author: Ramon
"""

import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
from datetime import datetime


# The callback for when the client receives a CONNACK response from the server.
def on_connect(MQTTclient, userdata, flags, rc):
	print("Connected with result code "+str(rc))

	# Subscribing in on_connect() means that if we lose the connection and
	# reconnect then subscriptions will be renewed.
	MQTTclient.subscribe("home/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(MQTTclient, userdata, msg):
	print(str(datetime.now()) + " - " + msg.topic+" "+str(msg.payload))
#	json_body = [
#		{
#		"measurement": "measurement",
#		"tags": {
#			"type": msg.topic.split('/')[2],
#			"location": msg.topic.split('/')[1]
#		},
#		"fields": {
#			"value": (float(msg.payload))
#			}
#		}
#	]
	#print(json_body)
	#ifClient.write(json_body,protocol='json')
	#ifClient.write_points(json_body, database='openhab', protocol='json')
	
	line = ['measurement,type=' + msg.topic.split('/')[2] + ',location=' + msg.topic.split('/')[1] + ' value=' + str(float(msg.payload))]
	#print(line)
	try:
		ifClient.write_points(line, database='openhab', protocol='line')
	except :
		print(str(datetime.now()), " - Error sending data to InfluxDB")


#line = [msg.topic.split('/')[2] + "_test " + float(msg.payload)]


#MQTT
MQTTclient = mqtt.Client()
MQTTclient.on_connect = on_connect
MQTTclient.on_message = on_message

MQTTclient.connect("192.168.1.92", 1883, 60)


#INFLUX
ifClient = InfluxDBClient(host='192.168.1.92', port=8086, username='root', password='root', database='openhab')
print(ifClient.get_list_database())
ifClient.switch_database('openhab')

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
MQTTclient.loop_forever()