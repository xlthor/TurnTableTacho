/**
 * Record Player Tachometer
 * ========================
 * 
 * Motivation
 * 
 *    The purpose of this small sketch is to track down the RPMs of a vinyl turntable (TT) in order to show the deviation over time.
 *    
 *    A vinyl turntable must meet two critical factors:
 *    * Target RPM: it should be exactly 33.33 or 45.0 RPMs
 *    * Stability:  the speed should not vary over time, at least not more than 0.3% wich is 0.01 RPM
 * 
 *    TT usually should have 33.33 rotations per minute. This is very slow, almost 2 seconds per rotation.
 *    The deviation is in 1/100 so it is for instance 33.35 or 33.28. 
 *    (If it is in an area above, you will defintevily hear it and do not need a graphical plotter, check yor TTs hardware then!)
 * 
 *    Target is to show the current RPM value on a four digit 7-segment display and plot a graph of the last 28 values to an OLED display of 128*128 px.
 *
 *    Given a hall sensor and magnets attached to the platter of a turntable (either on the bottom or the edge) we can track the time between each sensor interrupt and calculate the
 *    effective revolutions per minute (RPM). Displaying both on a 4-digit display and a graphical plotter can visiualize the precision if the turntable regarding 
 *    flutter and effective target RPM.
 *    
 * Basic principle
 * 
 *    Based on the Hall sensor and the magnets attached to the platter, we get [numMagnets] interrupts per one rotation. We track the time between two interrupts and calculate the according RPM. 
 *    This is displayed
 *    on the7-segment display with a precision of two digits ( "33:33"). The value is stored in an array which can hold 27 values. The last is dropped by shifting all values by one 
 *    and the new is inserted at the end of the array. On the OLED, these 27 values are plotted into a graph. Plotting is done in the main loop after an interrupt has been received from the sensor.
 *    The OLED has 128 columns but we need 18 cols for the Y-axis lebales. The "width" of a RPM value is 4 dots. (128 - 18) / 4 = 27.5
 * 
 * HW Setup
 *
 *    Board is an Arduino Nano, ATMega, 32K / 2 K on a seeed/grove base shield
 *    We use a Hall sensor to track the rotations with small neodym disc-type magnets attached to the bottom of the TT platter
 *    Hall is connected to D2
 *    With a button on D3 we can toggle the graph scaling on the OLED between 33 and 45, as we really stretch the scale and only show values between 32.0 and 34.0 RPMs. As said above, if
 *    the deviation is more than 1 RPM, the hardware setup of the TT should be checked or the motor, bearings, whatever ...
 *    
 *    The OLED is an SH1107 controlled 128x128 dot b/w OLED, connected on I2C of the nano.
 *    
 *    Additionally we have a 4-digit 7-segment display on D4, controlled by a TM1637
 *    
 *    As the usual seeed grove 4-pin connectors combine D2 and D3 within one connector, a kind of "fan-out" needs to be build: the Hall on D2, the button on D3. 
 *    This is necessary as only D2 and D3 trigger interrupts (0 and 1) and thus the Hall sensor and the button cannot be connected to any other pin.
 *    
 *  Remarks
 *    
 *    The ISR for the Hall sensor is critical. It must do some basic calculations but it should be as short as possible. The read-out of the micros must be the very first statement.
 *    If there is only one magnet, the flutter within one round can't be tracked. Especially if the belt is from varying thickness, this maight cause fast speed changes which are not captured then.
 *    If there are many magnets (i.e 6 or 8), number of interrupts may be critical during update of the display.
 *    Tried one magnet: resolution to low
 *    Tried six magnets: Position precision of the magnets is very critical, should be within few micrometers!
 *    Tried three magnets: Best results with an Arduino Nano on a belt drive with three magnets attached to the bottom ofthe platter
 *    
 *  Tests
 *  
 *    Tests with a standard TT show results between 33.32 and 33.34 RPMs constantly. The resolution of the OLED can be controlled by setting vmax and vmin accordingly in the OLDEPlotter.h structs.
 *    On microprocessor controlled TT with either a direct drive or a belt drive, the "swing-in phase" during start can nicely be seen until the target speed has been reached.
 *    
 *  Code
 *  
 *    Wherever possible, the use of global variables is omitted. Wherever possible, everything is encapsulated into classes. This helps saving memory and currently we are at 65% mem usage.
 *    The OLED functions are encapsulated into an include and wrap the graphics lib U8G2 entirely.
 *    This makes the sketch a bit more complicated to read, but for an Arduino savvy coder it shouldn't be a problem and all others might learn a bit.
 *    
 *  Idea
 *  
 *    Print the current RPM somwehere on the OLED as well instead on the 7-segment display. Coud save some turnaorund time probably ...
 *    
 *  
 * 
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

// counter for catched interrupts from the hall sensor
volatile int hallInterruptCounter;
bool update = false;

// time of the hall sensor interrupt;
unsigned long timeold;

// timeout after which we "measure" zero rotations
const unsigned long gap = 3000000;

int8_t digitalDisplay[] = { '0', '0', '0', '0' };

void setup() {
  // we shouldn't do any log messages as this causes delays
  debugger.setDebug(Debugger::TRACE);

  attachInterrupt(digitalPinToInterrupt(HALLSENSE_PIN), hallISR, RISING);  // Interrupt triggers on rising edge;
  // when the sensor turns off(the magnet leaves).

  tm1637.set(BRIGHT_DARKEST);
  tm1637.init();
  tm1637.point(POINT_ON);
  tm1637.clearDisplay();
  hallInterruptCounter = 0;
  TimeUpdate(0.0);

  oledPlotter.init();
  oledPlotter.toggleScale();

  // the ISR for the button cannot be a class member, so we need a wrapper here:
  pinMode(OLEDPlotter::button_pin, INPUT_PULLUP);
  u8g2Interrupt(true);

  debugger.print(Debugger::TRACE, "numRpmValues ");
  debugger.println(Debugger::TRACE, numRpmValues);

  for (int i = 0; i < numRpmValues; i++) {
    rpmValues[i] = 33.33;
  }
}

void u8g2Interrupt(bool set) {
  if (set) {
    attachInterrupt(digitalPinToInterrupt(OLEDPlotter::button_pin), toggleScale, RISING);
  } else {
    detachInterrupt(digitalPinToInterrupt(OLEDPlotter::button_pin));
  }
}

void loop() {

  TimeUpdate(rpmValues[numRpmValues - 1]);

  if (hallInterruptCounter % 2 == 0 || hallInterruptCounter == 0) {
    u8g2Interrupt(false);
    oledPlotter.plotGraph(rpmValues, numRpmValues);
    hallInterruptCounter = 0;
    u8g2Interrupt(true);
  }

  // set to zero after <gap> seconds w/o any interrupt. Default 3 seconds.
  if (micros() - timeold > gap) {
    TimeUpdate(0.0);
    timeold = micros();
    // re-initalize revolution counter
    hallInterruptCounter = 0;
    storeToArray(0.0);
    oledPlotter.plotGraph(rpmValues, numRpmValues);
  }

  delay(1000);
}

void storeToArray(float rpms) {

  memmove(rpmValues, &rpmValues[1], sizeof(float) * (numRpmValues - 1));

  // add as new last value
  rpmValues[numRpmValues - 1] = rpms;
}

/**
 * ISR for button press with debounce
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
  digitalDisplay[2] = fmt[len - 2] - '0';
  digitalDisplay[3] = fmt[len - 1] - '0';
  tm1637.display(digitalDisplay);
}
