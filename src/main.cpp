#pragma region Includes
#include <Arduino.h>
#pragma endregion Includes

#pragma region Define
#define waterlvlPIN A14 // PIN 13
#define enableWaterlvl 15
#pragma endregion Define

#pragma region GlobaleVariabelen
int waterlvl{};

//soil moisture
const int AirValue{3500};   //you need to replace this value with Value_1
const int WaterValue{1300}; //you need to replace this value with Value_2
int soilMoistureValue{};
int soilmoisturepercent{};
#pragma endregion GlobaleVariabelen

#pragma region FunctieDeclaraties
void grondvochtigheidsSensorEnWaterSensor(void *parameters);
#pragma endregion FunctieDeclaraties

void setup()
{
  Serial.begin(9600);
  pinMode(waterlvlPIN, INPUT);
  pinMode(enableWaterlvl, OUTPUT);

  xTaskCreatePinnedToCore(
      grondvochtigheidsSensorEnWaterSensor,
      "grondvochtigheidsSensorEnWaterSensor",
      1024,
      NULL,
      1,
      NULL,
      1);
}

void loop()
{

}

void grondvochtigheidsSensorEnWaterSensor(void *parameters)
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
}
