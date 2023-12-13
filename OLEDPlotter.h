/**
 * 
 *  Record Player Tachometer
 *  ========================
 *
 *  Helper functions to plot the cartesian graph on the OLED
 *
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

#ifndef OLEDPlotter_h
#define OLEDPlotter_h
#include "Debugger.h"

class OLEDPlotter {
  public:
    static constexpr int button_pin = 3;    // pin 3 is int 1 on nano
    void init();                            // initialize everything
    void toggleScale();                     // toggle scale between 33 and 45

    void plotGraph(float values[], int numVals);         // main method to plot rhe graph

    int value2Pixels(float value);          // calculates an RPM value into pixels forthe OLED

  private:
    void drawYAxis();                       // pretty Y axis

    // lib instance
    U8G2_SH1107_SEEED_128X128_2_HW_I2C *u8g2;

    // scaling
    enum scales {UNSET, THIRTYTHREE, FOURTYFIVE};

    // physical dimensions of the OLED
    const float oled_width = 128.0;
    const float oled_height = 128.0;

    // scaling definition
    struct t_scaling {
      float vmax;
      float vmin;
      float vtarget;
      float resolution;
      String labelTarget;
      String labelMin;
      String labelMax;
    };

    // Scale for 33
    const t_scaling scale_33 = { 36.0, 30.0, 33.33, oled_height / (36.0 - 30.0), "33", "30", "36" };

    // Scale for 45
    const t_scaling scale_45 = { 48.0, 42.0, 45.0, oled_height / (48.0 - 42.0), "45", "42", "48" };

    t_scaling scaleset = scale_45;

    scales scale;
    Debugger debugger;

    const int stretch = 4; // number of pixels on x-axis to increase on each value on y

};

void OLEDPlotter::init() {
  debugger.setDebug(Debugger::TRACE);

  // we init the toggle button on input pin 3 (2 and 3 support interrupts on a nano)
  pinMode(OLEDPlotter::button_pin, INPUT_PULLUP);

  /**
   *  init the OLED, we have a seeed Oled v2.1 with 128x128 pixels and an SH1107 controller
   *  connected to the I2C pin-out of the nano
   *  we use the u8g2 grafic library, see https://github.com/olikraus/u8g2/wiki/u8g2reference
   *
   */
  u8g2 = new U8G2_SH1107_SEEED_128X128_2_HW_I2C(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);;
  u8g2->begin();

  u8g2->setFont(u8g2_font_5x7_tf); //u8g2_font_7x14B_mr);
  u8g2->setFontMode(0);
  
}

int OLEDPlotter::value2Pixels(float value) {
  int px = (int)((value - scaleset.vmin) * scaleset.resolution);

  px = max( min(px, oled_height -1), 0);

  //debugger.print(Debugger::TRACE, "pxY ");
  //debugger.println(Debugger::TRACE, px);

  return px;
}

void OLEDPlotter::drawYAxis() {
  // first, plot the Y axis, we have 17 pix width
  u8g2->drawVLine(17, 0, 128);

  // invert as we have 0/0 in top left
  int pxY = oled_height - this->value2Pixels(scaleset.vtarget);

  u8g2->drawHLine(12, pxY, 5);
  u8g2->setCursor(0, pxY + 3);
  u8g2->print(scaleset.labelTarget);

  pxY = oled_height - this->value2Pixels(scaleset.vmin);
  u8g2->setCursor(0, pxY);
  u8g2->print(scaleset.labelMin);

  pxY = oled_height - this->value2Pixels(scaleset.vmax);
  u8g2->setCursor(0, pxY + 6);
  u8g2->print(scaleset.labelMax);
}

/**
 * plotGraph
 * redraws all current values to the display
 */
void OLEDPlotter::plotGraph(float values[], int numVals) {

  u8g2->firstPage();
  do {
    int height = oled_height - 1;
    
    u8g2->setDrawColor(1);
    drawYAxis();

    //debugger.print(Debugger::TRACE, "numVals ");
    //debugger.println(Debugger::TRACE, numVals); 

    int last = height - this->value2Pixels(values[0]);
    int pxX = 0;
    for ( int i = 0; i < numVals; i ++ ) {
      float nval = values[i];

      int pxY = height - this->value2Pixels(nval);

      u8g2->drawLine(oled_width - (pxX - stretch - 1), last, oled_width - pxX, pxY);
      
      last = pxY;
      pxX = pxX + stretch;
    }

  } while ( u8g2->nextPage() );
}

void OLEDPlotter::toggleScale() {
  if ( scale == THIRTYTHREE ) {
    scale = FOURTYFIVE;
    scaleset = scale_45;
  }
  else {
    scale = THIRTYTHREE;
    scaleset = scale_33;
  }

  // debugger.print(Debugger::INFO, "vmax ");
  // debugger.println(Debugger::INFO, scaleset.vmax);
  // debugger.print(Debugger::INFO, "vmin ");
  // debugger.println(Debugger::INFO, scaleset.vmin);
  // debugger.print(Debugger::INFO, "resolution ");
  // debugger.println(Debugger::INFO, scaleset.resolution);

}



#endif
