/*
  BasicRead - ARC8

  Reads all 8 sensors and prints their numbers to the Serial Monitor.

  On the ARC8 a BIG number means the sensor is looking at black, and a
  SMALL number means it is looking at white. Slide a piece of black tape
  under the bar and watch which number jumps up.

  Open Tools > Serial Monitor and set the speed to 9600.

  Wiring: this sketch uses A0 to A7, which an Arduino Nano has. An Uno only
  has A0 to A5, so use a Nano if you want all 8 sensors.
*/

#include <ARC8.h>

ARC8 sensors;

int pins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int values[8];

void setup() {
  Serial.begin(9600);
  sensors.begin(pins);
}

void loop() {
  // read() fills values[] and also remembers the numbers inside the library,
  // which is what lets printRaw() below know what to print.
  sensors.read(values);
  sensors.printRaw();

  delay(200);  // slow the printing down so you can actually read it
}
