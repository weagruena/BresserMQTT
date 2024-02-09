/////////////////////////////////////////////////////////////////
// BresserMQTT | mcwea | 2024-02-09
//
/////////////////////////////////////////////////////////////////
// Based on: Thomas Edlinger for www.edistechlab.com
// Based on: 
// https://github.com/matthias-bs/BresserWeatherSensorReceiver
/////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "WeatherSensorCfg.h"
#include "WeatherSensor.h"
// #include <ArduinoOTA.h>

#define wifi_ssid "WEAHOME"
#define wifi_password "ivonne01"
#define mqtt_server "192.168.6.194"
#define mqtt_user ""         
#define mqtt_password ""
#define ESPHostname "BresserESP32"
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */ 
#define TIME_TO_SLEEP 60 /* Time ESP32 will go to sleep (in seconds) */

// (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";
String mainTopic = "homeassistant/sensor/bresser/";
String stateTopic = mainTopic + "state/";
String inTopic = mainTopic + "cmd";
char sendestring[30];

WiFiClient espClient;
PubSubClient client(espClient);
WeatherSensor ws;

void getValues() {
  int sensorNumber = 1;
  float wind_max = 0.00;
  float wind_avg = 0.00;
  float wind_dir = 0.0;

  ws.clearSlots();
  bool decode_ok = ws.getData(60000, DATA_COMPLETE);
  
  if (!decode_ok)   {
	Serial.printf("Sensor timeout");
	String st = stateTopic + "sensor"
    client.publish(st, "timeout");
  }
  if (ws.sensor[sensorNumber].valid) {	
    
	String st = stateTopic + "sensor"
    client.publish(st, "online");

	Serial.printf("Id: [%8X] Typ: [%X] Ch: [%d] St: [%d] Bat: [%-3s] RSSI: [%6.1fdBm] ",
        (unsigned int)ws.sensor[sensorNumber].sensor_id,
        ws.sensor[sensorNumber].s_type,
        ws.sensor[sensorNumber].chan,
        ws.sensor[sensorNumber].startup,
        ws.sensor[sensorNumber].battery_ok ? "OK " : "Low",
        ws.sensor[sensorNumber].rssi);
		
		String st1 = stateTopic + "id"
		sprintf(sendestring, "%8X", static_cast<int> (ws.sensor[sensorNumber].sensor_id));
		mqttClient.publish(s1, sendestring);
		String st2 = stateTopic + "batt"
		sprintf(sendestring, "%-3s", ws.sensor[sensorNumber].battery_ok ? "OK " : "Low");
		mqttClient.publish(s2, sendestring); 
		String st3 = stateTopic + "rssi"
		sprintf(sendestring, "%6.1f", ws.sensor[sensorNumber].rssi);
		mqttClient.publish(st3, sendestring); 		
    
	if (ws.sensor[sensorNumber].w.wind_ok) {
		
		String windTopic = mainTopic + "wind/";
		wind_max = ws.sensor[sensorNumber].w.wind_gust_meter_sec;
		wind_avg = ws.sensor[sensorNumber].w.wind_avg_meter_sec;
		wind_dir = ws.sensor[sensorNumber].w.wind_direction_deg;
      
		Serial.println(wind_max);
		String w1 = windTopic + "max"
		client.publish(w1, String(wind_max).c_str());
		Serial.println(wind_avg);
		String w2 = windTopic + "avg"
		client.publish(w2, String(wind_avg).c_str());
		Serial.println(wind_dir);
		String w3 = windTopic + "dir"
		client.publish(w3, String(wind_dir).c_str());		
    }
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  // if ((char)payload[0] == '1') {
    //Room for Code
  // } else {
    // Room for Code
  // }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "BresserESP32-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
	  String st = stateTopic + "host"
      client.publish(st, ESPHostname);
	  String ip = stateTopic + "ip"
      client.publish(st, String(WiFi.localIP()));
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
   // ArduinoOTA.setHostname(ESPHostname);
  // ArduinoOTA.setPassword("admin");
  // ArduinoOTA.begin();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  if (!ws.begin()) {
    Serial.println("Sensor nicht gefunden");
    while (true);
  }
  if (!client.connected()) {
    reconnect();
  } else {
	String st = stateTopic + "host"
    client.publish(st, ESPHostname);
	String ip = stateTopic + "ip"
    client.publish(st, String(WiFi.localIP()));
    client.subscribe(inTopic);
  }
  client.loop();
  // ArduinoOTA.handle(); 
  
  delay(500);
  getValues();  
  delay(5000);
  // Go into deep sleep mode for 60 seconds
  Serial.println("Deep sleep mode for 60 seconds");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
	// client.loop();
}
