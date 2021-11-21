#include "DHT.h"
#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include "secrets.h"

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

WiFiClient WiFiclient;
MQTTClient client;

unsigned long timeSinceLastRead = 0;
const name = "s1";
const database = "telegraf";

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(2000);

  Serial.print("connecting to MQTT broker...");
  client.begin("mqtt.leetserve.com", WiFiclient);
  connect();

  dht.begin();
}

void connect() {
  while (!client.connect("nodemcu", "try", "try")) {
    Serial.print(".");
  }

  Serial.println("\nconnected!");
  client.subscribe("nodemcu");
}

void loop() {
  
  client.loop();
  if(!client.connected()) {
     connect();
  }

  getData();
}

void getData() {

  if(timeSinceLastRead > 3000) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
    }

    char htopic[100];
    char hstr[10];
    sprintf(hstr, "%f", h);
    snprintf(htopic, sizeof(htopic), "%s/%s%s",database,name, "/humidity");
    client.publish(htopic, hstr);

    char ttopic[100];
    char tstr[10];
    sprintf(tstr, "%f", t);
    snprintf(ttopic, sizeof(ttopic), "%s/%s%s",database,name, "/temperature");
    client.publish(ttopic, tstr);

    
    Serial.print("Success");

    timeSinceLastRead = 0;
  }
  delay(100);
  timeSinceLastRead += 100;
}
