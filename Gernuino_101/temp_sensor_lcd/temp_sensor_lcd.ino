// demo of Starter Kit V2.0 - Grove Temperature Sensor and LCD
//
#include <CurieTime.h>
#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;
const int pinTemp = A0;      // pin of temperature sensor

float temperature, temp_c;
int B = 3975;                // B value of the thermistor
float resistance;
int i=0;
int colorR = 64;
int colorG = i;
int colorB = 64;


void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);
  setTime(16, 37, 00, 17, 05, 2017);

  lcd.setRGB(colorR, colorG, colorB);



}

void loop()
{
  for (int j=0;j<9;j++)
{
  i=2^j;
 }
 
  int val = analogRead(pinTemp);                               // get analog value
  resistance = (float)(1023 - val) * 10000 / val;              // get resistance
  temperature = 1 / (log(resistance / 10000) / B + 1 / 298.15) - 273.15; // calc temperature
  temp_c = (5.0 / 9) * (temperature - 32.0);
  Serial.println(temp_c);
  lcd.clear();
  lcd.print("Temp.is:");
  lcd.print(temp_c);
  lcd.print("oC");
  delay(1000);
  lcd.clear();
  lcd.print("The time is:");
  lcd.setCursor(1,2);
   lcd.print(hour());
  lcd.print(":");
  lcd.print(minute());
  lcd.print(":");
  lcd.print(second());
  delay(2000);
  lcd.clear();
  lcd.print(day());
  lcd.print("/");
  lcd.print(month());
  lcd.print("/");
  lcd.print(year());
  delay(1000);
  lcd.clear();
}

