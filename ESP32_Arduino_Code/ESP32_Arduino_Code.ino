#include <WiFi.h>
#include <PubSubClient.h>
#include "Intensity_Motor_Headers/Intensity_Motor_Interface.h"
#include "Position_Motor_Headers/Position_Motor_Interface.h"
#include "Wifi_Headers/Wifi_Credentials.h"

const char* mqtt_server = "..........";
const char* Broker_User_Name = ".....";
const char* Broker_Password = "..........";

// Define PWM properties
const int pwmChannel = 0;     // PWM channel
const int pwmFreq = 5000;     // PWM frequency in Hz
const int pwmResolution = 8;  // PWM resolution in bits (8 bits = 0-255)
const int motorPin = 18;      // GPIO pin connected to the motor driver PWM input


WiFiClient espClient;
PubSubClient client(espClient);
int position_Motor_Mood = 50;

void setup_wifi() {
  Intensity_Motor_Init();
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  if ((strcmp(topic, "esp32/intensity") == 0)) {
    setMotorSpeed(message.toInt());
  }
  if ((strcmp(topic, "esp32/looping") == 0)) {
    position_Motor_Mood = 200;
  }
  if ((strcmp(topic, "esp32/stop") == 0)) {
    position_Motor_Mood = 50;
  }
  if ((strcmp(topic, "esp32/position") == 0)) {
    position_Motor_Mood = message.toInt();
  }
}

void reconnect() {
  // If internet Connection Lost Switch OFF Motors For Safety !
  setMotorSpeed(0);
  position_Motor_Mood = 50;
  
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "IOT_Motors_Controller";
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    String Will_Topic = "status";
    if (client.connect(clientId.c_str(), Broker_User_Name, Broker_Password, Will_Topic.c_str(), 1, true, "0", false)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(Will_Topic.c_str(), "1", true);
      // ... and resubscribe
      client.subscribe("esp32/intensity");
      client.subscribe("esp32/position");
      client.subscribe("esp32/looping");
      client.subscribe("esp32/stop");
      client.publish("flask/mqtt/intensity","0");
      client.publish("flask/mqtt/position","50");
      client.publish("flask/mqtt/looping","0");
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
  Intensity_Motor_Init();
  Position_Motor_Init();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Position_Motor_Prog();
}
