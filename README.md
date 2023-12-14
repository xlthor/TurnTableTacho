# Record Player Tachometer

This is a sketch for an arduino project to track down and plot the "speed" of a vinyl record player (aka "turntable").
It is based on an arduino nano, a hall sensor, an 128x128 dots graphic OLED display, a four-digit number display and this sketch.

It shows the current RPM of a turntable on the four-digit 7-segment display and the last 28 rpm values tracked by a graph on the OLED display.

Usecase: if you don't know whether your vinyl record player is running at the precise speed or you want to check it's wow and flutter,
just attach some neodym magnets to the platter and visualize the effective RPM over time.

It is possible to start with at least just one single magnet on the platter but the rresolution will be a bit insufficient as deviations within one turn can't be tracked.

## Motivation

   The purpose of this small sketch is to track down the RPMs of a vinyl turntable (TT) in order to show the deviation over time.
   
  A vinyl turntable must meet two critical factors:
  * Target RPM: it should be exactly 33.33 or 45.0 RPMs
  * Stability:  the speed should not vary over time, at least not more than 0.3% wich is 0.01 RPM

   TT usually should have 33.33 rotations per minute. This is very slow, almost 2 seconds per rotation.
   The deviation is in 1/100 so it is for instance 33.35 or 33.28. 

   (If it is in an area above, you will defintevily hear it and do not need a graphical plotter, check yor TTs hardware then!)

   Target is to show the current RPM value on a four digit 7-segment display and plot a graph of the last 28 values to an OLED display of 128*128 px.
 
   Given a hall sensor and magnets attached to the platter of a turntable (either on the bottom or the edge) we can track the time between each sensor interrupt and calculate the
   effective revolutions per minute (RPM). Displaying both on a 4-digit display and a graphical plotter can visiualize the precision of the turntable regarding 
   flutter and effective target RPM.
   
## Basic principle

   Based on the Hall sensor and the magnets attached to the platter, we get [numMagnets] interrupts per one rotation. We track the time between two interrupts and calculate the according RPM. 
   This is displayed on the 7-segment display with a precision of two digits ("33:33"). The value is stored in an array which can hold 28 values. The last is dropped by shifting all values by one 
   and the new is inserted at the end of the array. On the OLED, these 28 values are plotted into a graph. Plotting is done in the main loop after an interrupt has been received from the sensor.
   The OLED has 128 columns but we need 18 cols for the Y-axis lables. The "width" of a RPM value is 4 dots. (128 - 18) / 4 = 27.5

## HW Setup

   Board is an Arduino Nano, ATMega, 32K / 2 K on a seeed/grove base shield
   We use a Hall sensor to track the rotations with small neodym disc-type magnets attached to the bottom of the TT platter

   Hall sensor is connected to D2

   With a button on D3 we can toggle the graph scaling on the OLED between 33 and 45, as we really stretch the scale and only show values between 32.0 and 34.0 RPMs. As said above, if
   the deviation is more than 1 RPM, the hardware setup of the TT should be checked or the motor, bearings, whatever ...
   
   The OLED is an SH1107 controlled 128x128 dot b/w OLED, connected on I2C of the nano.
   
   Additionally we have a 4-digit 7-segment display on D4, controlled by a TM1637
   
   As the usual seeed grove 4-pin connectors combine D2 and D3 within one connector, a kind of "fan-out" needs to be build: the Hall on D2, the button on D3. 
   This is necessary as only D2 and D3 trigger interrupts (0 and 1) and thus the Hall sensor and the button cannot be connected to any other pin.
   
 ## Remarks
   
   The ISR for the Hall sensor is critical. It must do some basic calculations but it should be as short as possible. The read-out of the micros must be the very first statement.
   If there is only one magnet, the flutter within one round can't be tracked. Especially if the belt is from varying thickness, this might cause fast speed changes which are not captured then.
   If there are many magnets (i.e 6 or 8), number of interrupts may be critical during update of the display.
   * Tried one magnet: Resolution to low
   * Tried six magnets: Position precision of the magnets is very critical, should be within few micrometers!
   * Tried three magnets: Best results with an Arduino Nano on a belt drive with three magnets attached to the bottom of the platter
   
##  Tests
 
   Tests with a standard TT show results between 33.32 and 33.34 RPMs constantly. The resolution of the OLED can be controlled by setting vmax and vmin accordingly in the OLDEPlotter.h structs.
   On microprocessor controlled TT with either a direct drive or a belt drive, the "swing-in phase" during start can nicely be seen until the target speed has been reached.
   
## Code
 
   Wherever possible, the use of global variables is omitted. Wherever possible, everything is encapsulated into classes. This helps saving memory and currently we are at 65% mem usage.
   The OLED functions are encapsulated into an include and wrap the graphics lib U8G2 entirely.
   This makes the sketch a bit more complicated to read, but for an Arduino savvy coder it shouldn't be a problem and all others might learn a bit.
   
## Idea
 
   Print the current RPM somwe-where on the OLED as well instead on the 7-segment display. Coud save some turnaorund time probably ...
  