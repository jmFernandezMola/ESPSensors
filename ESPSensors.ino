#define WEB_SERVER
#define MQTT
#define OLED
#define DHT22

#ifdef MQTT
  #include <PubSubClient.h>
#endif

#ifdef DHT22
  #include <DHT.h>
  #define DHTTYPE     DHT11   // DHT 11
  #define DHT_PIN     D4
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


const char* ssid = "El Poni Pisador";
const char* password = "Anillo_Unico";

WiFiClient espClient;   //TCP connection
ESP8266WebServer server(80);    //
IPAddress brokerAddress(192,168,1,92);

PubSubClient MQTTclient(brokerAddress, 1883, espClient);

#ifdef DHT22
  DHT tempSensor(DHT_PIN, DHTTYPE);
  
#endif

int counter;
char MQTTvalue[64];


const int led = 13;

void handleRoot() {
  char value[20];
  digitalWrite(led, 1);
  snprintf(value, 20, "Value: %d", counter);
  server.send(200, "text/plain", value);
  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
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

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  //MQTT
  #ifdef MQTT
    MQTTclient.setServer(brokerAddress, 1883);
    MQTTclient.setCallback(callback);
  #endif

  #ifdef DHT22
    tempSensor.begin();
  #endif
}

void loop(void){

  counter ++;
  server.handleClient();
  // put your main code here, to run repeatedly:
  if (!MQTTclient.connected()) {
    reconnect();
    MQTTclient.subscribe("home/temp");
  }

  #ifdef DHT22
  
    //tempAns = tempSensor.read11(DHT11_PIN);
    float RH = tempSensor.readHumidity();
    // Read temperature as Celsius (the default)
    float T = tempSensor.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(RH) || isnan(T)) {
      Serial.println("Failed to read from DHT sensor!");
    }

    dtostrf(RH, 1, 1, MQTTvalue);
    //snprintf(MQTTvalue, 64, "%f", RH);
    MQTTclient.publish("home/RH", MQTTvalue);
    Serial.print("Humidity: ");
    Serial.println(RH);

    dtostrf(T, 1, 1, MQTTvalue);
    //snprintf(MQTTvalue, 64, "%f", T);
    MQTTclient.publish("home/T", MQTTvalue);
    Serial.print("Temperature: ");
    Serial.println(T);
    
  #endif

  snprintf(MQTTvalue, 64, "%d", counter);
  MQTTclient.publish("home/counter", MQTTvalue);
  
  MQTTclient.loop();
  delay(5000);
}


void reconnect() {
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
