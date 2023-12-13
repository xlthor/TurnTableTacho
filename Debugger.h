#ifndef Debugger_h
/*
 * Simple debugging class:
 * 
 * #include "Debugger.h"
 * 
 * Debugger debugger
 * ...
 * 
 * debugger.setDebug(Debugger.QUIET);  // turn off any output
 * 
 * debugger.println(Debugger.INFO, "Info String");
 * 
 * 2022/05/21 - Florian Amthor, Axel Amthor
 * 
 */
#define Debugger_h

class Debugger {
  public:
    enum debugLevel {QUIET = 0, INFO = 1, DEBUG = 2, TRACE = 3};
    void setDebug(debugLevel level);
    void print(debugLevel level, String line);
    void println(debugLevel level, String line);

    void print(debugLevel level, int line);
    void println(debugLevel level, int line);

    void print(debugLevel level, unsigned long line);
    void println(debugLevel level, unsigned long line);

    void print(debugLevel level, float line);
    void println(debugLevel level, float line);
    
    void print(debugLevel level, char line);
    void println(debugLevel level, char line);

  private:
    debugLevel debug;
};

void Debugger::setDebug(debugLevel level) {
  debug = level;
  if (  level > QUIET )
    Serial.begin(115200);
}

void Debugger::println(debugLevel level, String line) {
  if ( level > QUIET && level <= debug ) Serial.println(line);
}

void Debugger::print(debugLevel level, String line) {
  if ( level > QUIET && level <= debug ) Serial.print(line);
}

void Debugger::println(debugLevel level, int line) {
  if ( level > QUIET && level <= debug ) Serial.println(line);
}

void Debugger::print(debugLevel level, int line) {
  if ( level > QUIET && level <= debug ) Serial.print(line);
}

void Debugger::println(debugLevel level, float line) {
  if ( level > QUIET && level <= debug ) Serial.println(line);
}

void Debugger::print(debugLevel level, float line) {
  if ( level > QUIET && level <= debug ) Serial.print(line);
}

void Debugger::println(debugLevel level, unsigned long line) {
  if ( level > QUIET && level <= debug ) Serial.println(line);
}

void Debugger::print(debugLevel level, unsigned long line) {
  if ( level > QUIET && level <= debug ) Serial.print(line);
}

void Debugger::println(debugLevel level, char line) {
  if ( level > QUIET && level <= debug ) Serial.println(line);
}

void Debugger::print(debugLevel level, char line) {
  if ( level > QUIET && level <= debug ) Serial.print(line);
}


#endif
