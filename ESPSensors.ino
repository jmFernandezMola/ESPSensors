//#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdateServer.h>
//#include <map>

//#ifdef MQTT
	#include "mqtt_cfg.h"
//#endif // MQTT

#include "dataPoint.h"

#define FW_VERSION	"1.0"

#define WEB_SERVER
#define MQTT
#define OLED
#define DHT22

/*************************************
 *   Adreces físiques dels sensors
 * 
 * (només n'hi pot haver un de definit)
*/
#define SENSOR_ID   "S2"
#define SENSOR_LOC  0
#define SENSOR_ROOM 2

/*********************************************/

#ifdef DHT22
  #include <DHT.h>
  #define DHTTYPE     DHT11   // DHT 11
  #define DHT_PIN     D4
#endif

#define HW_PIN_LED  13
#define MAX_DATA_POINTS 15

/********************************************
 * Informació sobre la WiFi
 */
WiFiClient espClient;   //TCP connection

const char* ssid = "El Poni Pisador";
const char* password = "Anillo_Unico";
/**************************************************/


/********************************************
 * Servidor web
 */
ESP8266WebServer webSrv(80);
ESP8266HTTPUpdateServer httpUpdater;
/***************************************************/

/********************************************
 * Connexió MQTT
 */
/*local*/
IPAddress brokerAddress(192,168,1,92);
uint16_t brokerPort = 1883;
	/*remote*/
	//char brokerAddress[] = { "elponipisador.ddns.net" };
	//uint16_t brokerPort = 61883;
PubSubClient MQTTclient(brokerAddress, brokerPort, espClient);

char locations[MAX_MQTT_LOCATIONS][MAX_MQTT_STRING_LENGTH+1]={"home", "gufe", "cosu", "mollo", "vella","","","","",""};
int locations_count = 5;
char rooms[MAX_MQTT_ROOMS][MAX_MQTT_STRING_LENGTH+1]={"menjad","cuina","habit1","habit2","habit3","estudi","lavabo1","lavabo2","taller","golfes","traster","terras","jardi","soterr"};
int rooms_count = 14;
char MQTTvalue[64];
char MQTTtag[MAX_MQTT_TAG_LENGTH];
/************************************************/

/************************************************
* Definició de les variables / DataPoints
*/
#define DP_ID			0
#define DP_IPADDR		1
#define DP_SSID			2
#define DP_WEBURL		3
#define DP_LOCATION		4
#define	DP_ROOM			5
#define DP_UPDATERATE	6
#define DP_TEMP			7
#define DP_HR			8
#define DP_UPDATEREQUEST	9
#define DP_UPDATEDEVICE		10
#define DP_UPDATEVERSION	11
uint usedDataPoints = 12;
/***********************************************/


/************************************************
 * Informació sobre el dispositiu
 */
char devId[MAX_MQTT_STRING_LENGTH+1] = {SENSOR_ID};
char devLocation[MAX_MQTT_STRING_LENGTH+1];
char devRoom[MAX_MQTT_STRING_LENGTH+1];

dataPoint dataPoints[MAX_DATA_POINTS];

/***********************************************/

#ifdef DHT22
  DHT tempSensor(DHT_PIN, DHTTYPE);
#endif

/************************************************
* Informació sobre el sistema de sensors
*/
	unsigned long updateRate = 15000;  //in ms
	int counter;
	char * strT;
	char * strRH;
	byte updateRequested = 0;
/***********************************************/


void setup(void){

  pinMode(HW_PIN_LED, OUTPUT);
  //digitalWrite(HW_PIN_LED, 0);
  Serial.begin(115200);

  //Inicialització de les variables del sistema
  strcpy(devLocation, locations[SENSOR_LOC]);
  strcpy(devRoom, rooms[SENSOR_ROOM]);

  //Inicialització de les variables monitoritzades
  initDataPoints();
  
  //Mostra les dades per la consola
  Serial.println("");
  Serial.println("*** Sensor DATA ***");
  Serial.print("-> Sensor ID: "); Serial.println(devId);
  Serial.print("-> Sensor location: "); Serial.println(devLocation);//devLocation);
  Serial.print("-> Sensor room: "); Serial.println(devRoom);
  Serial.print("-> Update rate: "); Serial.println(updateRate);
  Serial.println("*******************");
  
  //Arrenca la Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

	  // Wait for connection
	  while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	  }
  
	  Serial.println("");
	  Serial.print("Connected to ");
	  Serial.println(ssid);
	  Serial.print("IP address: ");
	  Serial.println(WiFi.localIP());
	  //Arrenca la WiFi [end]

  //Multicast DNS
	if (MDNS.begin(strcat("ESP_", devId))) {
		Serial.println("MDNS responder started");
	}
	MDNS.addService("http", "tcp", 80);
  //Multicast DNS[end]

  //WebServer[begin]
  webSrv.on("/", webSrv_handleRoot);
  webSrv.on("/inline", [](){
    webSrv.send(200, "text/plain", "this works as well");
  });
  webSrv.onNotFound(webSrv_handleNotFound);

  httpUpdater.setup(&webSrv);

  webSrv.begin();
  Serial.println("HTTP server started");
  Serial.println("HTTPUpdateServer ready! Update at http://<ipaddress>/update");
  //WebServer[end]


  //MQTT[begin]
  #ifdef MQTT
    MQTTclient.setServer(brokerAddress, brokerPort);
    MQTTclient.setCallback(mqtt_callback);
    char mqtt_buidTag();

  #endif
  //MQTT[end]

  #ifdef DHT22
    tempSensor.begin();
	delay(5000); //deixa un temps per estabilitzar el sensor
  #endif

	//Data points
	for (int idx = 0; idx < usedDataPoints; idx++)
	{
		char cDummy[MAX_MQTT_TAG_LENGTH];

		if (dataPoints[idx].GetType() == deviceParameter) {
			sprintf(cDummy, "sensor/%s/%s", devId, dataPoints[idx].GetName());
		}
		else if (dataPoints[idx].GetType() == sensedValue) {
			sprintf(cDummy, "%s/%s/%s", devLocation, devRoom, dataPoints[idx].GetName());
		}
		else if (dataPoints[idx].GetType() == systemValue) {
			sprintf(cDummy, "system/update/%s", dataPoints[idx].GetName());
		}
		else {
			strcpy(cDummy, "other");
		}

		dataPoints[idx].SetTag(cDummy);
		Serial.println(cDummy);
	}

	char cDummy[MAX_MQTT_TAG_LENGTH];
		WiFi.localIP().toString().toCharArray(cDummy, MAX_MQTT_TAG_LENGTH);
	dataPoints[DP_IPADDR].SetcValue(cDummy);
		WiFi.SSID().toCharArray(cDummy, MAX_MQTT_TAG_LENGTH);
	dataPoints[DP_SSID].SetcValue(cDummy);
		MDNS.hostname(0).toCharArray(cDummy, MAX_MQTT_TAG_LENGTH);
	dataPoints[DP_WEBURL].SetcValue(cDummy);
	dataPoints[DP_LOCATION].SetcValue(devLocation);
	dataPoints[DP_ROOM].SetcValue(devRoom);
	dataPoints[DP_UPDATERATE].SetuiValue(updateRate);
	//Data points [END]
}

void loop(void){

  counter ++;
  webSrv.handleClient();


  if (!MQTTclient.connected()) {
    mqtt_reconnect();
  }

  #ifdef DHT22
  
    //Read relative humidity [%]
    float RH = tempSensor.readHumidity();
    
    // Check if any reads failed and exit early (to try again).
    if (!(isnan(RH))){
		char cDummy[MAX_MQTT_TAG_LENGTH];
      dtostrf(RH, 1, 1, cDummy);
	  dataPoints[DP_HR].SetcValue(cDummy);

	  Serial.print(dataPoints[DP_HR].GetName());
	  Serial.print(": ");
	  Serial.println(RH);
    } else {
      Serial.println("Failed to read RH from DHT sensor!");
    }
      
	// Read temperature as Celsius (the default)
	float T = tempSensor.readTemperature();
    if (!(isnan(T))) {
		char cDummy[MAX_MQTT_TAG_LENGTH];
      dtostrf(T, 1, 1, cDummy);
	  dataPoints[DP_TEMP].SetcValue(cDummy);

	  Serial.print(dataPoints[DP_TEMP].GetName());
	  Serial.print(": ");
	  Serial.println(T);

     } else {
      Serial.println("Failed to read Temperature from DHT sensor!");
     }
   
  #endif //DHT22

	//MQTT Publish
	if (MQTTclient.connected()) {
		Serial.println("MQTT is connected. Proceeding with the MQTT update");
		for (uint idx = 0; idx < usedDataPoints; idx++)
		{
			//Serial.println("Printing from the MQTT for loop");
			//char topic[MAX_MQTT_TAG_LENGTH + 1], value[MAX_MQTT_TAG_LENGTH + 1];
			//strcpy(topic, dataPoints[idx].GetTag());
			//strcpy(value, dataPoints[idx].GetcValue());

			//Serial.print("Topic: ");
			//Serial.println(topic);

			//Serial.print("Value: ");
			//Serial.println(value);

			MQTTclient.publish(dataPoints[idx].GetTag(), dataPoints[idx].GetcValue(), false);
		}

		//Subscriptions
		MQTTclient.subscribe(dataPoints[DP_UPDATEREQUEST].GetTag());
		MQTTclient.subscribe(dataPoints[DP_UPDATEDEVICE].GetTag());
		MQTTclient.subscribe(dataPoints[DP_UPDATEVERSION].GetTag());


		MQTTclient.loop();
	}
	else {
		Serial.println("Error in the MQTT connection... values not sent.");
	}
  

  Serial.println("A dormir!!!");
  Serial.flush();

   
  //ESP.deepSleep(updateRate*1000);
  delay(updateRate);
}


void initDataPoints(){

	dataPoints[DP_ID].ID = DP_ID;
	dataPoints[DP_ID].SetName("ID");
	dataPoints[DP_ID].SetcValue(devId);
	dataPoints[DP_ID].SetType(deviceParameter);

	dataPoints[DP_IPADDR].ID = DP_IPADDR;
	dataPoints[DP_IPADDR].SetName("IPAddr");
	//dataPoints[DP_IPADDR].SetcValue("192.168.1.113");//WiFi.localIP().toString().toCharArray());
	dataPoints[DP_IPADDR].SetType(deviceParameter);

	dataPoints[DP_SSID].ID = DP_SSID;
	dataPoints[DP_SSID].SetName("SSID");
	//dataPoints[DP_SSID].SetcValue(ssid);
	dataPoints[DP_SSID].SetType(deviceParameter);
	
	dataPoints[DP_WEBURL].ID = DP_WEBURL;
	dataPoints[DP_WEBURL].SetName("webURL");
	//dataPoints[DP_WEBURL].SetcValue("");
	dataPoints[DP_WEBURL].SetType(deviceParameter);
	
	dataPoints[DP_LOCATION].ID = DP_LOCATION;
	dataPoints[DP_LOCATION].SetName("location");
	//dataPoints[DP_LOCATION].SetcValue("");
	dataPoints[DP_LOCATION].SetType(deviceParameter);

	dataPoints[DP_ROOM].ID = DP_ROOM;
	dataPoints[DP_ROOM].SetName("room");
	//dataPoints[DP_ROOM].SetcValue("");
	dataPoints[DP_ROOM].SetType(deviceParameter);

	dataPoints[DP_UPDATERATE].ID = DP_UPDATERATE;
	dataPoints[DP_UPDATERATE].SetName("updateRate");
	//dataPoints[DP_ROOM].SetcValue("");
	dataPoints[DP_UPDATERATE].SetType(deviceParameter);

	dataPoints[DP_TEMP].ID = DP_TEMP;
	dataPoints[DP_TEMP].SetName("temp");
	//dataPoints[DP_TEMP].SetcValue("");
	dataPoints[DP_TEMP].SetType(sensedValue);
	
	dataPoints[DP_HR].ID = DP_HR;
	dataPoints[DP_HR].SetName("hr");
	//dataPoints[DP_HR].SetcValue("");
	dataPoints[DP_HR].SetType(sensedValue);

	dataPoints[DP_UPDATEREQUEST].ID = DP_UPDATEREQUEST;
	dataPoints[DP_UPDATEREQUEST].SetName("upReq");
	//dataPoints[DP_HR].SetcValue("");
	dataPoints[DP_UPDATEREQUEST].SetType(systemValue);

	dataPoints[DP_UPDATEDEVICE].ID = DP_UPDATEDEVICE;
	dataPoints[DP_UPDATEDEVICE].SetName("upDev");
	//dataPoints[DP_HR].SetcValue("");
	dataPoints[DP_UPDATEDEVICE].SetType(systemValue);

	dataPoints[DP_UPDATEVERSION].ID = DP_UPDATEVERSION;
	dataPoints[DP_UPDATEVERSION].SetName("upVer");
	//dataPoints[DP_HR].SetcValue("");
	dataPoints[DP_UPDATEVERSION].SetType(systemValue);

	usedDataPoints = 12;
	}

void mqtt_reconnect() {
	static int numRetries = 0;

  // Loop until we're reconnected
  while (!MQTTclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    if (MQTTclient.connect("monRa-ESPClient")) {
      Serial.println("connected");

    } else {
		if (WiFi.status() != WL_CONNECTED) {
			//do nothing
			Serial.println("AT mqtt_reconnect - WiFi is connected");
			ESP.restart();
		}
		else if (numRetries <= MAX_MQTT_RETRIES) {
			numRetries++;
			Serial.print("failed, rc=");
			Serial.print(MQTTclient.state());
			Serial.println(" try again in 5 seconds");

			Serial.println("AT mqtt_reconnect - Min retries");
			// Wait 5 seconds before retrying
			delay(5000);
		}
		else {
			//do nothing
			Serial.println("AT mqtt_reconnect - ELSE -> Restarting");
			ESP.restart();
		}
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void mqtt_buidTag(char * tag){
    tag = strcat(devLocation,"/");
    tag = strcat(tag,devRoom);
}

void webSrv_handleRoot() {
  char value[20];
  String message;
  message = "Device ID:";
  message += devId;
  message += "\n\n";
  message += "Temperature: ";
  message += "___";
  message += "ºC";
  message += "\n";
  message += "Humidity: ";
  message += "___";
  message += "%";
  message += "\n";
  
  //digitalWrite(HW_PIN_LED, 1);

  webSrv.send(200, "text/plain", message);
  //digitalWrite(HW_PIN_LED, 0);
}

void webSrv_handleNotFound(){
  //digitalWrite(HW_PIN_LED, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webSrv.uri();
  message += "\nMethod: ";
  message += (webSrv.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += webSrv.args();
  message += "\n";
  for (uint8_t i=0; i<webSrv.args(); i++){
    message += " " + webSrv.argName(i) + ": " + webSrv.arg(i) + "\n";
  }
  webSrv.send(404, "text/plain", message);
  //digitalWrite(HW_PIN_LED, 0);
}
