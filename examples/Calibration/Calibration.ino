/*
  Calibration - ARC8

  Every sensor is slightly different, so the library has to learn what white
  and black look like for each one before its numbers mean anything.

  When the sketch starts you get 3 seconds. Sweep the sensor bar back and
  forth across the line so that every single sensor sees both the white
  floor and the black tape. After that, all 8 sensors are stretched onto the
  same 0 to 1000 scale: 0 is the whitest that sensor ever saw, 1000 is the
  blackest.

  Open Tools > Serial Monitor and set the speed to 9600.
  Uses A0 to A7, so run this on an Arduino Nano.
*/

#include <ARC8.h>

ARC8 sensors;

int pins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int values[8];

void setup() {
  Serial.begin(9600);
  sensors.begin(pins);

  Serial.println(F("Sweep the sensors across the line now!"));

  // Keep reading and learning for 3 seconds. calibrate() only looks at the
  // numbers read() just collected, so the two always go together.
  unsigned long start = millis();
  while (millis() - start < 3000) {
    sensors.read(values);
    sensors.calibrate();
  }

  Serial.println(F("Done. Here is what each sensor learned:"));
  for (int i = 0; i < 8; i++) {
    Serial.print(F("sensor "));
    Serial.print(i);
    Serial.print(F("  white="));
    Serial.print(sensors.getMin(i));
    Serial.print(F("  black="));
    Serial.println(sensors.getMax(i));
  }

  Serial.println(F("Now printing the stretched 0 to 1000 numbers:"));
}

void loop() {
  sensors.read(values);
  sensors.printCalibrated();

  delay(200);
}
