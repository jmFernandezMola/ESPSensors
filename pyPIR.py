import paho.mqtt.client as mqtt
import time
from gpiozero import LED, MotionSensor

pir = MotionSensor(24)
led = LED(17)

#pir.queue_len = 10
#pir.partial = True
#pir.threshold = 0.1

def on_move():
    led.blink(on_time = 0.5, off_time = 0.5, n = 1, background = True)
    print(time.asctime(time.localtime()) + " - YOU MOVED!")
    mqttClient.publish("home/py/motion", payload="Motion", qos=0, retain=False)


mqttClient = mqtt.Client(client_id="pyPIR_sensor", clean_session=True, userdata=None, protocol="MQTTv311")
mqttClient.connect("192.168.1.92", port=1883, keepalive=60)


while True:
    
    if pir.motion_detected:
        on_move()    
    #pir.wait_for_motion()
        #led.blink(on_time = 0.5, off_time = 0.5, n = 1, background = True)
        #print("You moved!!")
    #pir.wait_for_no_motion()
    mqttClient.publish("home/py/motion", payload="Quiet", qos=0, retain=False)
    time.sleep(5)
    #pir.wait_for_motion()
    #led.toggle()
    #print("You moved")
    #pir.wait_for_no_motion()