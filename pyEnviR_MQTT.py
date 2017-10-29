# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt
import time
import untangle #per processar XMLs
import serial
import logging
import logging.handlers
import sys
#from gpiozero import LED, Button

def on_connect(client, userdata, flags, rc):
	log.debug("Connected with result code "+str(rc))

	# Subscribing in on_connect() means that if we lose the connection and
	# reconnect then subscriptions will be renewed.
	#client.subscribe("$SYS/#")

	# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
	log.debug(msg.topic+" "+str(msg.payload))

	
def getCCdata(port, verbose=False):
#	ser = serial.Serial(port, 57600)
	#if not port.is_open():
	#	port.open()
	
	try :
		if (serialPort == None) :
			log.debug("Serial port found closed!. Opening...")
			serialPort.open()
		
		xmldata = serialPort.readline()
		
		log.debug(xmldata)
		
		p = untangle.parse(xmldata.decode("utf-8"))
		
		#Getting data from the received xml stream
		temperature = float(p.msg.tmpr.cdata)
		watts = int(p.msg.ch1.watts.cdata)
		#print("P: ", watts, "W; ", "T: ", temperature, "ºC")
		log.info('Power: %s W; Temp: %s C', watts, temperature)


	except IndexError:
		log.exception("Error parsing XML, maybe a history packet: IndexError")
		temperature = None
		watts = None
	except :
		log.exception('Reading data: %s', sys.exc_info()[1])
		#print("Exception: General")
		temperature = None
		watts = None
	finally:
		return (watts, temperature)


urlRaspi_local = "localhost"
urlRaspi_remot = "http://elponipisador.no-ip.org"
devSerialPort = '/dev/serial0'

#LOGGER setup
log = logging.getLogger('__name__')
log.setLevel(logging.INFO)
logHdlr = logging.handlers.SysLogHandler('/dev/log')
logFormat = logging.Formatter('EnviR: %(levelname)s - %(message)s')
logHdlr.setFormatter(logFormat)
log.addHandler(logHdlr)


try :
	#SERIAL PORT
	serialPort = serial.Serial(port=devSerialPort, baudrate=57600, timeout=10)

	#MQTT
	mqttClient = mqtt.Client() #client_id="python_client", clean_session=True, userdata=None, protocol="MQTTv311")
	mqttClient.on_connect = on_connect
	mqttClient.on_message = on_message
	mqttClient.connect(urlRaspi_local,port=1883,keepalive=60)
	
except :
	log.exception('Opening connections: %s', sys.exc_info()[1])



while True:
	(CCpow, CCtemp) = getCCdata(port=serialPort,verbose=False)
	
	if CCpow != None:
		mqttClient.publish("home/CC/power", payload=CCpow, qos=0, retain=False)
	
	if CCtemp != None:
		mqttClient.publish("home/CC/temp", payload=CCtemp, qos=0, retain=False)
	
	mqttClient.loop()
	time.sleep(2)






