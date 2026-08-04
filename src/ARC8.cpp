#include "ARC8.h"

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

ARC8::ARC8() {
  for (int i = 0; i < 8; i++) {
    _pins[i] = 0;
    _raw[i]  = 0;

    // Start the smallest at the highest possible reading and the biggest at
    // the lowest, so the very first real reading replaces both of them.
    _calMin[i] = 1023;
    _calMax[i] = 0;
  }

  // How much each sensor counts towards the line position.
  //
  // Sensor 0 is the one on the RIGHT of the array and sensor 7 is on the
  // LEFT. The right-hand half counts negative and the left-hand half
  // positive, so a NEGATIVE position means the line has gone right.
  //
  // That is the same way round as the Spirit robot firmware, which is why
  // the steering line reads speedLeft = base - correction.
  //
  // The numbers double as they go outwards instead of climbing evenly. The
  // outer sensors are the ones worth reacting hardest to, because once the
  // line reaches them it is about to leave the array altogether.
  _weights[0] = -8;
  _weights[1] = -4;
  _weights[2] = -2;
  _weights[3] = -1;
  _weights[4] =  1;
  _weights[5] =  2;
  _weights[6] =  4;
  _weights[7] =  8;

  _hasRead       = false;
  _calibrated    = false;
  _threshold     = 500;
  _setpoint      = 0;
  _lineIsBlack   = true;
  _lastLineValue = 0;

  _warnedPinTwice       = false;
  _warnedBadIndex       = false;
  _warnedThreshold      = false;
  _warnedNoRead         = false;
  _warnedNotCalibrated  = false;
  _warnedBadCalibration = false;
  _warnedLineMode       = false;
}

void ARC8::begin(int pins[8]) {
  for (int i = 0; i < 8; i++) {
    _pins[i] = pins[i];
    pinMode(pins[i], INPUT);
  }

  // Listing the same pin twice is a common typo, so say something about it.
  for (int i = 0; i < 8; i++) {
    for (int j = i + 1; j < 8; j++) {
      if (_pins[i] == _pins[j] && !_warnedPinTwice) {
        Serial.println(F("ARC8: pin used twice"));
        _warnedPinTwice = true;
      }
    }
  }
}

// ---------------------------------------------------------------------------
// Raw read
// ---------------------------------------------------------------------------

void ARC8::read(int values[8]) {
  for (int i = 0; i < 8; i++) {
    values[i] = analogRead(_pins[i]);
    _raw[i]   = values[i];
  }
  _hasRead = true;
}

int ARC8::read(int i) {
  if (_badIndex(i)) return -1;
  return analogRead(_pins[i]);
}

// ---------------------------------------------------------------------------
// Calibration
// ---------------------------------------------------------------------------

void ARC8::calibrate() {
  if (!_checkRead()) return;

  for (int i = 0; i < 8; i++) {
    if (_raw[i] < _calMin[i]) _calMin[i] = _raw[i];
    if (_raw[i] > _calMax[i]) _calMax[i] = _raw[i];
  }
  _calibrated = true;
}

void ARC8::readCalibrated(int values[8]) {
  if (!_checkRead()) {
    for (int i = 0; i < 8; i++) values[i] = 0;
    return;
  }

  for (int i = 0; i < 8; i++) {
    values[i] = _scale(_raw[i], i);
  }
}

int ARC8::readCalibrated(int i) {
  if (_badIndex(i)) return -1;
  return _scale(analogRead(_pins[i]), i);
}

int ARC8::getMin(int i) {
  if (_badIndex(i)) return -1;
  return _calMin[i];
}

int ARC8::getMax(int i) {
  if (_badIndex(i)) return -1;
  return _calMax[i];
}

// Stretches one sensor's raw reading onto a shared 0 to 1000 scale.
//
// Why bother? No two sensors are the same. One might read 120 on the white
// floor and 800 on the black tape, while the sensor right next to it reads
// 200 and 950. If we compared those numbers directly, the brighter sensor
// would always look like it was closer to the line, and the robot would
// steer towards it forever. Giving every sensor its own stretch, from the
// white it actually saw up to the black it actually saw, means all 8 end up
// speaking the same language and can be compared fairly.
int ARC8::_scale(int raw, int i) {
  if (_calMax[i] - _calMin[i] < 10) {
    // The gap between white and black is far too small to stretch. Dividing
    // by a tiny gap would blow small wobbles up into huge numbers, so we hand
    // the raw reading straight back instead of a broken scaled one.
    if (!_calibrated) {
      if (!_warnedNotCalibrated) {
        Serial.println(F("ARC8: not calibrated yet"));
        _warnedNotCalibrated = true;
      }
    } else {
      if (!_warnedBadCalibration) {
        Serial.println(F("ARC8: sensor not calibrated properly"));
        _warnedBadCalibration = true;
      }
    }
    return constrain(raw, 0, 1000);
  }

  // The (long) is not optional. raw - _calMin[i] can be about 1000, and
  // multiplying that by 1000 gives roughly a million, which does not fit in
  // the 16-bit int an Uno or Nano uses. Without the (long) the number wraps
  // around into nonsense and the robot steers the wrong way.
  long value = (long)(raw - _calMin[i]) * 1000 / (_calMax[i] - _calMin[i]);
  return constrain((int)value, 0, 1000);
}

// ---------------------------------------------------------------------------
// Digital / threshold
// ---------------------------------------------------------------------------

void ARC8::setThreshold(int value) {
  if (value < 0 || value > 1000) {
    if (!_warnedThreshold) {
      Serial.println(F("ARC8: threshold out of range"));
      _warnedThreshold = true;
    }
  }
  _threshold = constrain(value, 0, 1000);
}

void ARC8::readDigital(bool values[8]) {
  if (!_checkRead()) {
    for (int i = 0; i < 8; i++) values[i] = false;
    return;
  }

  for (int i = 0; i < 8; i++) {
    values[i] = _isOnLine(_scale(_raw[i], i));
  }
}

bool ARC8::onLine(int i) {
  if (_badIndex(i)) return false;
  return _isOnLine(_scale(analogRead(_pins[i]), i));
}

int ARC8::countOnLine() {
  if (!_checkRead()) return 0;

  int count = 0;
  for (int i = 0; i < 8; i++) {
    if (_isOnLine(_scale(_raw[i], i))) count++;
  }
  return count;
}

bool ARC8::_isOnLine(int calValue) {
  return _lineIsBlack ? (calValue > _threshold) : (calValue < _threshold);
}

// ---------------------------------------------------------------------------
// Line position
// ---------------------------------------------------------------------------

void ARC8::setLine(int mode) {
  if (mode != BLACK && mode != WHITE) {
    if (!_warnedLineMode) {
      Serial.println(F("ARC8: invalid line mode"));
      _warnedLineMode = true;
    }
    return;  // keep whatever was set before
  }
  _lineIsBlack = (mode == BLACK);
}

// Keep your weights inside about -32 to +32. The line position is worked out
// as weight x reading, readings go up to 1000, and the answer is stored in an
// int - so a weight of 33 or more can tip it over and flip the sign.
void ARC8::setWeights(int w[8]) {
  int smallest = w[0];
  int biggest  = w[0];

  for (int i = 0; i < 8; i++) {
    _weights[i] = w[i];
    if (w[i] < smallest) smallest = w[i];
    if (w[i] > biggest)  biggest  = w[i];
  }

  // Halfway between the smallest and biggest weight is what "dead centre"
  // looks like, so that is the number getError() measures against.
  _setpoint = (smallest + biggest) / 2;
}

// Works out where the line is by taking a weighted average of the sensors
// that can see it.
//
// Why weighted? Each sensor gets a number saying where it sits on the bar:
// the left-hand ones are negative, the right-hand ones are positive. A
// sensor that is only half over the tape gives a smaller reading than one
// sitting right on it, so multiplying each sensor's position by how strongly
// it sees black lets a sensor that is barely touching the line pull the
// answer only a little. That is what makes the position slide smoothly as
// the robot drifts, instead of jumping from one sensor to the next.
void ARC8::_updateLine() {
  if (countOnLine() == 0) return;  // nothing to see, keep the last position

  // sum is a long because the running total of weight x reading can easily
  // pass what an int holds. The finished answer still goes into
  // _lastLineValue, which is an int, so keep custom weights inside about
  // -32 to +32: 32 x 1000 is 32000, which just fits.
  long sum   = 0;
  int  count = 0;

  for (int i = 0; i < 8; i++) {
    int cal = _scale(_raw[i], i);
    if (_isOnLine(cal)) {
      sum += (long)_weights[i] * cal;
      count++;
    }
  }
  _lastLineValue = sum / count;
}

int ARC8::readLine() {
  if (!_checkRead()) return 0;
  _updateLine();
  return _lastLineValue;
}

int ARC8::getError() {
  if (!_checkRead()) return 0;
  _updateLine();
  return _lastLineValue - _setpoint;
}

bool ARC8::lineFound() {
  if (!_checkRead()) return false;
  return countOnLine() > 0;
}

// ---------------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------------

void ARC8::printRaw() {
  if (!_checkRead()) return;

  for (int i = 0; i < 8; i++) {
    Serial.print(_raw[i]);
    if (i < 7) Serial.print(' ');
  }
  Serial.println();
}

void ARC8::printCalibrated() {
  if (!_checkRead()) return;

  for (int i = 0; i < 8; i++) {
    Serial.print(_scale(_raw[i], i));
    if (i < 7) Serial.print(' ');
  }
  Serial.println();
}

void ARC8::printLine() {
  if (!_checkRead()) return;

  Serial.print(F("line: "));
  Serial.print(_lastLineValue);
  Serial.print(F(" error: "));
  Serial.println(_lastLineValue - _setpoint);
}

// ---------------------------------------------------------------------------
// Private checks
// ---------------------------------------------------------------------------

bool ARC8::_checkRead() {
  if (_hasRead) return true;

  if (!_warnedNoRead) {
    Serial.println(F("ARC8: call read() first"));
    _warnedNoRead = true;
  }
  return false;
}

bool ARC8::_badIndex(int i) {
  if (i >= 0 && i <= 7) return false;

  if (!_warnedBadIndex) {
    Serial.println(F("ARC8: bad sensor index"));
    _warnedBadIndex = true;
  }
  return true;
}
