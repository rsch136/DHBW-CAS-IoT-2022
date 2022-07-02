#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

/* WiFi-Data */ 
const char *ssid = "IoT-Netz";
const char *password = "CASIOT22";
WiFiMulti wifiMulti;

/* MQTT-Data */ 
const char *MQTTSERVER = "mq.jreichwald.de"; 
int MQTTPORT = 1883;
const char *mqttuser = "dbt"; 
const char *mqttpasswd = "dbt"; 
const char *mqttdevice = "RJLED";  // Please use a unique name here!
const char *outTopic = "rjLED"; 
WiFiClient wifiClient; 
PubSubClient client(wifiClient); 


/* JSON-Document-Size for incoming JSON (object size may be increased for larger JSON files) */ 
const int capacity = JSON_OBJECT_SIZE(6); 


/* Outgoing JSON Documents */ 
DynamicJsonDocument doc(capacity);


#define MSG_BUFFER_SIZE  (256) // Define the message buffer max size
char msg[MSG_BUFFER_SIZE]; // Define the message buffer 

#define PIN_RED    23 // GIOP23
#define PIN_GREEN  16 // GIOP16
#define PIN_BLUE   21 // GIOP21


/**
 * This function is called when a MQTT-Message arrives. 
 */
void mqtt_callback(char* topic, byte* payload, unsigned int length) {   //callback includes topic and payload ( from which (topic) the payload is comming)
  Serial.print("Message arrived ["); 
  Serial.print(topic); 
  Serial.print("]: ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println(""); 

  
  // Create a JSON document on call stack 
  StaticJsonDocument<capacity> doc; 
  String jsonInput = String((char*)payload); 

  // try to deserialize JSON 
  DeserializationError err = deserializeJson(doc, jsonInput); 

  // if an error occurs, print it out 
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
    return; 
  }

  // Read out a name from JSON content (assuming JSON doc: {"SensorType" : "SomeSensorType", "value" : 42}) 
  const char* messageLed = doc["messageLed"]; 

  if(String(topic) == "rjOutput") {
    Serial.print("Changing output to ");
  
    if (String(messageLed) =="on") {
       Serial.println("on");

  // color code RED RJ
       analogWrite(PIN_RED,   255);
       analogWrite(PIN_GREEN, 0);
       analogWrite(PIN_BLUE,  0);

    }  
    else if(String(messageLed) =="off") {
      Serial.println("off");
      
        // color code RED RJ
       analogWrite(PIN_RED,   0);
       analogWrite(PIN_GREEN, 255);
       analogWrite(PIN_BLUE,  0);
    }

    else {
      Serial.print("Kein fall getroffen"); 
    }
  }
}


/**
 * This function is called from setup() and establishes a MQTT connection.
 */ 
void initMqtt() {
  client.setServer(MQTTSERVER, MQTTPORT); 

  // Set the callback-Function when new messages are received.
  client.setCallback(mqtt_callback); 
  

  client.connect(mqttdevice, mqttuser, mqttpasswd); 
  while (!client.connected()) {
    Serial.print("."); 
    delay(500); 
  }

  // subscribe to a certain topic
  client.subscribe("rjOutput"); 
}

/**
 * This function is called from setup() and establishes a WLAN connection
 */ 

void initWifi() {
  Serial.println("Connecting to WiFi ..."); 
  wifiMulti.addAP(ssid, password);
  
  while (wifiMulti.run() != WL_CONNECTED) {
        Serial.print("."); 
        delay(500); 
    }

   Serial.println("");
   Serial.println("WiFi connected");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
}

/**
 * This function is called when the ESP is powered on. 
 */
void setup()
{
  // Set serial port speed to 115200 Baud 
  Serial.begin(115200);

  // Connect to WLAN 
  initWifi();  

  // Connect to MQTT server
  initMqtt(); 

/**
 * RGB setup RJ
 */
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);        

  // Print to console 
  Serial.println("Setup completed.");
  delay(2000); 

}

/**
 * This function is the main function and loops forever. 
 */
void loop()
{
  // loop the mqtt client so it can maintain its connection and send and receive messages 
  client.loop(); 

   // set measured data to preprared JSON document 
  // setJSONData(Humidity, Temperature); 

  // serialize JSON document to a string representation 
  serializeJsonPretty(doc, msg);   

  // publish to MQTT broker 
  client.publish(outTopic, msg);
  client.loop();

  // wait a second before the next loop.
  delay(1000);   
}
