#ifndef STARGATE_MAIN_H
#define STARGATE_MAIN_H

#include <Arduino.h>
#include "ArduinoClass.h"
#include "USB.h"

class Main: public ArduinoClass {
public:
    void setup() override;

    void loop() override;

private:
    USB _usb;
};


#endif //STARGATE_MAIN_H
