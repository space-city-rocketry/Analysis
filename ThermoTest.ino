// this example is public domain. enjoy!
// https://learn.adafruit.com/thermocouple/

#include "max6675.h"

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
int thermoDO1 = 7;
int thermoCS1 = 8;
int thermoCLK1 = 9;
int thermoDO2 = 23;
int thermoCS2 = 22;
int thermoCLK2 = 21;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
MAX6675 thermocouple1(thermoCLK1, thermoCS1, thermoDO1);
MAX6675 thermocouple2(thermoCLK2, thermoCS2, thermoDO2);


void setup() {
  Serial.begin(115200);

  Serial.println("MAX6675 test");
  // wait for MAX chip to stabilize
  delay(500);
}

void loop() {
  // basic readout test, just print the current temp
  //delay(1);
  
   float sensor0 = thermocouple.readFahrenheit();
   float sensor1 = thermocouple1.readFahrenheit();
   float sensor2 = thermocouple2.readFahrenheit();
   Serial.print(sensor0);
      Serial.print(", ");
   Serial.print(sensor1);
         Serial.print(", ");
   Serial.println(sensor2);


   

   //delay(1);
//
//   Serial.print("sensor 1, F = ");
//   Serial.println(thermocouple1.readFahrenheit());
//   //delay(1);
//
//   Serial.print("sensor 2, F = ");
//   Serial.println(thermocouple2.readFahrenheit());
   // For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
  delay(500);
}
