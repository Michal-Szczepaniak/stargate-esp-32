#ifndef STARGATE_MAIN_H
#define STARGATE_MAIN_H

#include <Arduino.h>
#include <set>
#include "ArduinoClass.h"
#include "USB.h"

class Main: public ArduinoClass {
public:
    void setup() override;

    void loop() override;

private:
    void onKeyDown(uint8_t id);

private:
    USB _usb;
    std::vector<uint8_t> _chevronsPressed;
    static const std::set<std::vector<uint8_t>> _validAddresses;
};


#endif //STARGATE_MAIN_H
