# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt
import time
import untangle #per processar XMLs
import serial


def on_connect(client, userdata, flags, rc):
	print("Connected with result code "+str(rc))

	# Subscribing in on_connect() means that if we lose the connection and
	# reconnect then subscriptions will be renewed.
	#client.subscribe("$SYS/#")

	# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
	print(msg.topic+" "+str(msg.payload))

def getCCdata(port, verbose=False):
#	ser = serial.Serial(port, 57600)
	#if not port.is_open():
	#	port.open()
	
	try :
		port.open()
		xmldata = port.readline()

		if verbose:
			print(xmldata)
			
		port.close()
		
		p = untangle.parse(xmldata.decode("utf-8"))
		temperature = float(p.msg.tmpr.cdata)
		watts = int(p.msg.ch1.watts.cdata)
		print("P: ", watts, "W; ", "T: ", temperature, "ºC")
	except IndexError:
		print("Error parsing XML, maybe a history packet: IndexError")
		temperature = None
		watts = None
	except :
		print("Exception: General")
		temperature = None
		watts = None
	finally:
		return (watts, temperature)


ser = serial.Serial(port='/dev/ttyAMA0', baudrate=57600, timeout=10)

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
	(CCpow, CCtemp) = getCCdata(port=ser,verbose=False)
	#print('Temp: ', CCtemp,'- Power: ', CCpow)
	mqttClient.publish("home/CC/power", payload=CCpow, qos=0, retain=False)
	mqttClient.publish("home/CC/temp", payload=CCtemp, qos=0, retain=False)
	time.sleep(2)
	mqttClient.loop()

#mqttClient.loop_forever()





