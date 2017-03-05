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

def getCCdata(port='/dev/ttyAMA0', verbose=False):
	"""Gets temperature and power usage data from an attached CurrentCost meter.

	Parameters:
	 port: the port that the CurrentCost meter is attached to. Somthing like /dev/ttyUSB0 or COM1

    Returns:
    (temperature, usage), with temperature in degrees C, and usage in Watts
    """
	ser = serial.Serial(port, 57600)
	xmldata = ser.readline()
	if verbose:
		print(xmldata)
	ser.close()
	
	try :
		p = untangle.parse(xmldata.decode("utf-8"))
		temperature = float(p.msg.tmpr.cdata)
		watts = int(p.msg.ch1.watts.cdata)
	except IndexError:
		print("Error parsing XML, maybe a history packet: IndexError")
		temperature = None
		watts = None
	except :
		print("Error with the XML from CurrentCost: Exception")
		temperature = None
		watts = None
	finally:
		return (watts, temperature)


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
    (CCpow, CCtemp) = getCCdata(port='/dev/ttyAMA0',verbose=True)
    #print('Temp: ', CCtemp,'- Power: ', CCpow)
    mqttClient.publish("home/CC/power", payload=CCpow, qos=0, retain=False)
    mqttClient.publish("home/CC/temp", payload=CCtemp, qos=0, retain=False)
    time.sleep(2)
    mqttClient.loop()

#mqttClient.loop_forever()





