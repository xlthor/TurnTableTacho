/**
 * Record Player Tachometer
 * ========================
 */
/****************************************************************************/
#include "Debugger.h"
#include "OLEDPlotter.h"
#include "TM1637.h"

#define CLK 4  // pins definitions for TM1637 and can be changed to other ports
#define DIO 5
#define HALLSENSE_PIN 2

Debugger debugger;
OLEDPlotter oledPlotter;

TM1637 tm1637(CLK, DIO);

// each rotation adds the measured rpms, up to (numRpmValues) discrete values
// rpms must be float as we want to get at least 2 digit precision (33.33 RPM for a 12'' vinyl)
float rpmValues[28];
int numRpmValues = sizeof(rpmValues) / sizeof(rpmValues[0]);  // stretch 4

// number of magnets on platter
const unsigned int numMagnets = 3;

// whether to show an average or the effective current RPM value
const bool AVG = false;

// counter for catched interrupts from the hall sensor
volatile int hallInterruptCounter;

// loop counter
int loops = 0;

// time of the hall sensor interrupt;
unsigned long timeold;

// timeout after which we "measure" zero rotations
const unsigned long gap = 3000000;

int8_t digitalDisplay[] = { '0', '0', '0', '0' };

void setup() {

  // we shouldn't do any log messages as this causes delays
  debugger.setDebug(Debugger::QUIET);

  attachInterrupt(digitalPinToInterrupt(HALLSENSE_PIN), hallISR, RISING);  
  // Interrupt triggers on rising edge;

  // 7-segemnt display settimngs
  tm1637.set(BRIGHT_DARKEST);
  tm1637.init();
  tm1637.point(POINT_ON);
  tm1637.clearDisplay();
  TimeUpdate(0.0);

  hallInterruptCounter = 0;
  
  oledPlotter.init();
  oledPlotter.toggleScale(); // "standard" is 33.33

  // the ISR for the button cannot be a class member, so we need a wrapper here:
  pinMode(OLEDPlotter::button_pin, INPUT_PULLUP);
  u8g2Interrupt(true);

  //debugger.print(Debugger::TRACE, "numRpmValues ");
  //debugger.println(Debugger::TRACE, numRpmValues);

  // initalize the rpm vlaue array with a horizontal line at 33.33 RPM
  for (int i = 0; i < numRpmValues; i++) {
    rpmValues[i] = 33.33;
  }

}

/**
 * attach or detach hall sensor interrupt
 */
void u8g2Interrupt(bool set) {
  if (set) {
    attachInterrupt(digitalPinToInterrupt(OLEDPlotter::button_pin), toggleScale, RISING);
  } else {
    detachInterrupt(digitalPinToInterrupt(OLEDPlotter::button_pin));
  }
}

/**
 * processor loop
 */
void loop() {
  
  // plot the graph after every second loop
  if (loops % 2 == 0 || loops == 0) {

    u8g2Interrupt(false);
    oledPlotter.plotGraph(rpmValues, numRpmValues);
    hallInterruptCounter = 0;
    u8g2Interrupt(true);
    loops = 0;

  }

  // we cannot track "no revolution" ...
  // set to zero after <gap> seconds w/o any interrupt. Default 3 seconds.
  if (micros() - timeold >= gap) {

    TimeUpdate(0.0);
  
    // re-initalize revolution counter
    hallInterruptCounter = 0;
    storeToArray(0.0);

  } else {

    if ( AVG ) { // average over all values in array
      float avgRpm = 0.0;
      for ( int i = 0; i < numRpmValues; i++ ) {
        avgRpm += rpmValues[i];
      }
      TimeUpdate(avgRpm / numRpmValues);

    } else {

      TimeUpdate(rpmValues[numRpmValues - 1]); // last value in array

    }

  }

  delay(1000);
  loops ++;
}

/**
 * add current value to RPM values array
 */
void storeToArray(float rpms) {

  memmove(rpmValues, &rpmValues[1], sizeof(float) * (numRpmValues - 1));

  // add as new last value
  rpmValues[numRpmValues - 1] = rpms;
}

/**
 * Intterrupt Service Routine (ISR) for button press with debounce
 */
long lastPress = 0;
void toggleScale() {

  unsigned long now = millis();

  if (lastPress > 0 && now - lastPress > 300) {
    // debugger.println(Debugger::TRACE, "ISR Button");
    oledPlotter.toggleScale();
  }
  lastPress = now;
}

/**
 * ISR for Hall sensor
 */
void hallISR() {
  // immediately take the time and save to a tmp var
  unsigned long timeold_tmp = micros();

  // take the time lapse after second interrupt
  if (hallInterruptCounter++ > 0) {

    // calculate the rotation time
    unsigned long rotating_time = (micros() - timeold);

    float rpms = 60000000.0 / (rotating_time * numMagnets);

    storeToArray(rpms);
  }

  // save the time we have catched this interrupt
  timeold = timeold_tmp;
}

/**
 * update the 7-segment display
 */
void TimeUpdate(float rpms) {

  //  debugger.println(Debugger::TRACE, "----------------------------------");
  //  for ( int i = 0; i < numRpmValues; i++ ) {
  //    debugger.print(Debugger::TRACE, rpmValues[i]);
  //    debugger.print(Debugger::TRACE, ", ");
  //  }
  //  debugger.println(Debugger::TRACE, "----------------------------------");

  String fmt;
  int len = 5;

  if (rpms > 0.0) {
    fmt = String(rpms, 2);
    len = fmt.length();
    if (len < 5)
      fmt = "0" + fmt;
  } else
    fmt = "00.00";

  // debugger.print(Debugger::DEBUG, (int)hallInterruptCounter);
  // debugger.print(Debugger::DEBUG, ": ");
  // debugger.println(Debugger::DEBUG, fmt);

  digitalDisplay[0] = fmt[len - 5] - '0';
  digitalDisplay[1] = fmt[len - 4] - '0';
  // "3" is the dot ..
  digitalDisplay[2] = fmt[len - 2] - '0';
  digitalDisplay[3] = fmt[len - 1] - '0';
  tm1637.display(digitalDisplay);
}
