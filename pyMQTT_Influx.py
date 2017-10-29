# -*- coding: utf-8 -*-
"""
Created on Sun Oct 22 22:49:17 2017

@author: Ramon
"""

import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
#from datetime import datetime
import logging
import logging.handlers
import sys

# The callback for when the client receives a CONNACK response from the server.
def on_connect(MQTTclient, userdata, flags, rc):
	log.debug("Connected with result code %s", str(rc))
	# Subscribing in on_connect() means that if we lose the connection and
	# reconnect then subscriptions will be renewed.
	MQTTclient.subscribe("home/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(MQTTclient, userdata, msg):
	#print(str(datetime.now()) + " - " + msg.topic+" "+str(msg.payload))
	
	line = ['measurement,type=' + msg.topic.split('/')[2] + ',location=' + msg.topic.split('/')[1] + ' value=' + str(float(msg.payload))]

	try:
		log.info("New input: type: %s, location: %s, value: %s", msg.topic.split('/')[2], msg.topic.split('/')[1], str(float(msg.payload)))
		ifClient.write_points(line, database='openhab', protocol='line')
	except :
		log.exception("Error sending data to InfluxDB: %s", sys.exc_info()[1])
		#print(str(datetime.now()), " - Error sending data to InfluxDB")


#line = [msg.topic.split('/')[2] + "_test " + float(msg.payload)]

urlRaspi_local = "localhost"
urlRaspi_remot = "http://elponipisador.no-ip.org"
devSerialPort = '/dev/serial0'

#LOGGER setup
log = logging.getLogger('__name__')
log.setLevel(logging.DEBUG)
logHdlr = logging.handlers.SysLogHandler('/dev/log')
logFormat = logging.Formatter('MQTT-Influx: %(levelname)s - %(message)s')
logHdlr.setFormatter(logFormat)
log.addHandler(logHdlr)

#MQTT
MQTTclient = mqtt.Client()
MQTTclient.on_connect = on_connect
MQTTclient.on_message = on_message

MQTTclient.connect(urlRaspi_local, 1883, 60)


#INFLUX
ifClient = InfluxDBClient(host=urlRaspi_local, port=8086, username='root', password='root', database='openhab')
#print(ifClient.get_list_database())
ifClient.switch_database('openhab')

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
MQTTclient.loop_forever()