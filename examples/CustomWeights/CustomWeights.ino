/*
  CustomWeights - ARC8

  Every sensor has a weight: a number saying where it sits on the bar. The
  left-hand sensors get negative numbers, the right-hand ones get positive
  numbers, and readLine() mixes them together to say where the line is.

  The weights the library starts with are {-4,-3,-2,-1,1,2,3,4}, which are
  evenly spread. Here we use {-7,-5,-3,-1,1,3,5,7} instead. The gaps at the
  ends are bigger, so when the line reaches an outer sensor the position
  number shoots up further and the robot reacts harder before it loses the
  line completely.

  setWeights() also works out the new middle for you, so getError() still
  gives 0 when the line is dead centre.

  Open Tools > Serial Monitor and set the speed to 9600.
  Uses A0 to A7, so run this on an Arduino Nano.
*/

#include <ARC8.h>

ARC8 sensors;

int pins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int values[8];

int myWeights[8] = {-7, -5, -3, -1, 1, 3, 5, 7};

void setup() {
  Serial.begin(9600);
  sensors.begin(pins);

  sensors.setLine(BLACK);
  sensors.setWeights(myWeights);

  Serial.println(F("Sweep the sensors across the line now!"));
  unsigned long start = millis();
  while (millis() - start < 3000) {
    sensors.read(values);
    sensors.calibrate();
  }
  Serial.println(F("Done. Slide the bar across the line and watch."));
}

void loop() {
  sensors.read(values);

  // readLine() is what works the position out. printLine() only shows the
  // last one that was worked out, so call readLine() first.
  sensors.readLine();
  sensors.printLine();

  delay(200);
}
