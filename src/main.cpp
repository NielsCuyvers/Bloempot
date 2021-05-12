#pragma region Includes
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h> // MQTT
#pragma endregion Includes

#pragma region Define
// Water sensor
#define waterlvlPIN A0 // PIN 13
#define enableWaterlvl 15

// MQTT
#define mqtt_port 1883
#define MQTT_USER "mqtt-user"
#define MQTT_PASSWORD "mqqt-user-wrong-user"
#define MQTT_SERIAL_PUBLISH_CH "SmartBloempot"
#pragma endregion Define

#pragma region GlobaleVariabelen
int waterlvl{};

//soil moisture
const int AirValue{3500};   //you need to replace this value with Value_1
const int WaterValue{1300}; //you need to replace this value with Value_2
int soilMoistureValue{};
int soilmoisturepercent{};

// WiFi
WiFiClient wifiClient;
const char* ssid = "Cuyvers-Thielemans 0";
const char* password = "j7BMpXJP0Ssf";
const char* mqtt_server = "192.168.1.247";

// MQTT
PubSubClient client(wifiClient);
#pragma endregion GlobaleVariabelen

#pragma region FunctieDeclaraties
void setupWiFi();

void grondvochtigheidsSensorEnWaterSensor(void* parameters);

// MQTT
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void publishDataViaMqtt(void* parameters);
#pragma endregion FunctieDeclaraties

void setup()
{
  Serial.begin(9600);
  pinMode(waterlvlPIN, INPUT);
  pinMode(enableWaterlvl, OUTPUT);

  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

  xTaskCreatePinnedToCore(
      grondvochtigheidsSensorEnWaterSensor,
      "grondvochtigheidsSensorEnWaterSensor",
      1024,
      NULL,
      1,
      NULL,
      1);

  xTaskCreatePinnedToCore(
      publishDataViaMqtt,
      "publishDataViaMqtt",
      1024,
      NULL,
      2,
      NULL,
      0);
}

void loop()
{
}

void setupWiFi()
{
  vTaskDelay(10 / portTICK_PERIOD_MS);
  // We start by connecting to a WiFi network
  Serial.println("Connecting to ");
  Serial.print(ssid);
  Serial.println();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(200 / portTICK_PERIOD_MS);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println();
}

void grondvochtigheidsSensorEnWaterSensor(void* parameters)
{
  for (;;)
  {
    digitalWrite(enableWaterlvl, HIGH);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    waterlvl = analogRead(waterlvlPIN);
    //Serial.println(waterlvl);
    digitalWrite(enableWaterlvl, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    soilMoistureValue = analogRead(A12);
    soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
    Serial.println(soilMoistureValue);
    //Serial.println(soilmoisturepercent);
  }
  vTaskDelete(NULL);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD))
    {
      Serial.println("connected");
      //Once connected, publish an announcement
      client.publish("/icircuit/presence/ESP32/", "hello world");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

// MQTT callback functie
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println("-------new message from broker-----");
  Serial.print("channel:");
  Serial.println(topic);
  Serial.print("data:");
  Serial.write(payload, length);
  Serial.println();
}

void publishDataViaMqtt(void* parameters)
{
  for (;;)
  {
    client.loop();
    if (!client.connected())
    {
      reconnect();
    }
    String tempStr = String(soilmoisturepercent);
    char tempCharArr[tempStr.length() + 1];
    tempStr.toCharArray(tempCharArr, tempStr.length() + 1);
    client.publish("SmartBloempot/soilMoisturePercent", tempCharArr);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}