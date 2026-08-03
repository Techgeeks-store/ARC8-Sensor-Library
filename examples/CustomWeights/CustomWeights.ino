/*
  CustomWeights - ARC8

  Every sensor has a weight: a number saying where it sits on the array.
  readLine() mixes them together to say where the line is.

  Sensor 0 is the one on the RIGHT and sensor 7 is on the LEFT. The
  right-hand sensors get negative numbers and the left-hand ones positive.

  The weights the library starts with are {-8,-4,-2,-1,1,2,4,8}, which
  double as they go outwards. Here we use {-7,-5,-3,-1,1,3,5,7} instead,
  which is a gentler spread. Try this if the robot reacts too sharply as
  the line moves out towards the edges.

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
