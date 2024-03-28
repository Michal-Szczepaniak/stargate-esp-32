#ifndef STARGATE_MAIN_H
#define STARGATE_MAIN_H

#include <Arduino.h>
#include <set>
#include "ArduinoClass.h"
#include "USB.h"
#include "FFat.h"
#include <Audio.h>
#include <FastLED.h>
#include <DRV8834.h>

#define NOT_SEQUENTIAL_PINOUT 1
#define PCF8575_DEBUG 1
#include "PCF8575.h"

#include "../.pio/libdeps/esp32-s3-devkitc-1-n16r8v/ESP32-audioI2S/src/Audio.h"
#include "../.pio/libdeps/esp32-s3-devkitc-1-n16r8v/FastLED/src/FastLED.h"
#include "../.pio/libdeps/esp32-s3-devkitc-1-n16r8v/PCF8575 library/PCF8575.h"

class Main: public ArduinoClass {
public:
    Main();

    void setup() override;

    void loop() override;

private:
    void onKeyDown(uint8_t id);
    void play(std::string filePath);

private:
    USB _usb;
    std::vector<uint8_t> _chevronsPressed;
    static const std::set<std::vector<uint8_t>> _validAddresses;
    Audio _audio;
    CRGB _leds[144];
    DRV8834 _stepper;
    PCF8575 _ioExpander;
    std::vector<std::pair<int, int>> _chevronLEDs;
    std::vector<std::pair<int, int>> _chevronMots;
};


#endif //STARGATE_MAIN_H
