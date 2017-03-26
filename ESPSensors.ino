#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include <string.h>
#include "mqtt_cfg.h"

#define WEB_SERVER
#define MQTT
#define OLED
#define DHT22

/*************************************
 *   Adreces físiques dels sensors
 * 
 * (només n'hi pot haver un de definit)
*/
#define SENSOR_ID   "D1m_2"
#define SENSOR_LOC  0
#define SENSOR_ROOM 1

/*********************************************/

#ifdef MQTT
  #include <PubSubClient.h>
  #define MAX_MQTT_PLACES   10
  #define MAX_MQTT_ROOMS    15
  #define MAX_MQTT_STRING_LENGTH    8
#endif

#ifdef DHT22
  #include <DHT.h>
  #define DHTTYPE     DHT11   // DHT 11
  #define DHT_PIN     D4
#endif

#define HW_PIN_LED  13

char * dummyChar[32];

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

/***************************************************/

/********************************************
 * Connexió MQTT
 */
IPAddress brokerAddress(192,168,1,92);
PubSubClient MQTTclient(brokerAddress, 1883, espClient);

char places[MAX_MQTT_PLACES][MAX_MQTT_STRING_LENGTH+1]={"home", "gufe", "cosu", "mollo", "vella","","","","",""};
int places_count = 5;
char rooms[MAX_MQTT_ROOMS][MAX_MQTT_STRING_LENGTH+1]={"menjad","cuina","habit1","habit2","habit3","estudi","lavabo1","lavabo2","taller","golfes","traster","terras","jardi","soterr"};
int rooms_count = 14;
char MQTTvalue[64];
/************************************************/


/************************************************
 * Informació sobre el dispositiu
 */
char devId[MAX_MQTT_STRING_LENGTH+1] = {SENSOR_ID};
char devLocation[MAX_MQTT_STRING_LENGTH+1];
char devRoom[MAX_MQTT_STRING_LENGTH+1];

//memcpy(devLocation, &places[MAX_MQTT_PLACES], sizeof(char)*(MAX_MQTT_STRING_LENGTH+1));
//memcpy(devRoom, &rooms[MAX_MQTT_ROOMS], sizeof(char)*(MAX_MQTT_STRING_LENGTH+1));

/***********************************************/
#ifdef DHT22
  DHT tempSensor(DHT_PIN, DHTTYPE);
#endif

int counter;
char * strT;
char * strRH;

void setup(void){
  pinMode(HW_PIN_LED, OUTPUT);
  //digitalWrite(HW_PIN_LED, 0);
  Serial.begin(115200);
  
  Serial.println("");
  Serial.println("*** Sensor DATA ***");
  Serial.print("-> Sensor ID: "); Serial.println(devId);
  Serial.print("-> Sensor location: "); Serial.println(places[SENSOR_LOC]);//devLocation);
  Serial.print("-> Sensor room: "); Serial.println(rooms[SENSOR_ROOM]);
  Serial.println("*******************");
  
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

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  //WebServer[begin]
  webSrv.on("/", webSrv_handleRoot);
  webSrv.on("/inline", [](){
    webSrv.send(200, "text/plain", "this works as well");
  });
  webSrv.onNotFound(webSrv_handleNotFound);

  webSrv.begin();
  Serial.println("HTTP server started");
  //WebServer[end]


  //MQTT[begin]
  #ifdef MQTT
    MQTTclient.setServer(brokerAddress, 1883);
    MQTTclient.setCallback(mqtt_callback);
    char mqtt_buidTag();
  #endif
  //MQTT[end]

  #ifdef DHT22
    tempSensor.begin();
  #endif
}

void loop(void){

  counter ++;
  webSrv.handleClient();
  // put your main code here, to run repeatedly:
  if (!MQTTclient.connected()) {
    mqtt_reconnect();
    //MQTTclient.subscribe("home/temp2");
  }

  #ifdef DHT22
  
    //tempAns = tempSensor.read11(DHT11_PIN);
    float RH = tempSensor.readHumidity();
    
    // Read temperature as Celsius (the default)
    float T = tempSensor.readTemperature();



    // Check if any reads failed and exit early (to try again).
    if (not(isnan(RH))){
      //dtostrf(RH, 1, 1, strRH);
      //snprintf(MQTTvalue, 64, "%f", RH);
      dtostrf(RH, 1, 1, MQTTvalue);
      MQTTclient.publish("home/S3/RH", MQTTvalue);
      Serial.print("Humidity: ");
      Serial.println(RH);
    } else {
      Serial.println("Failed to read RH from DHT sensor!");
    }
      
      
    if (not(isnan(T))) {
      //dtostrf(T, 1, 1, MQTTvalue);
      //snprintf(MQTTvalue, 64, "%f", T);
      dtostrf(T, 1, 1, MQTTvalue);
      MQTTclient.publish("home/S3/T", MQTTvalue);
      Serial.print("Temperature: ");
      Serial.println(T);
     } else {
      Serial.println("Failed to read RH from DHT sensor!");
     }
   
  #endif
  
  snprintf(MQTTvalue, 64, "%d", counter);
  MQTTclient.publish("home/S3/counter", MQTTvalue);
  
  MQTTclient.loop();
  delay(5000);
}


void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!MQTTclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    if (MQTTclient.connect("monRa-ESPClient")) {
      Serial.println("connected");

    } else {
      Serial.print("failed, rc=");
      Serial.print(MQTTclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
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
