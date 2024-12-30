#ifndef STARGATE_MAIN_H
#define STARGATE_MAIN_H

#include <Arduino.h>
#include <set>
#include "ArduinoClass.h"
#include "USB.h"
#include "FFat.h"
#include <Audio.h>
#include <FastLED.h>
#include "FastAccelStepper.h"

#define NOT_SEQUENTIAL_PINOUT 1
#define PCF8575_DEBUG 1

#include "PCF8575.h"
#include "AsyncWebSocket.h"

class Main : public ArduinoClass {
public:
    Main();

    void setup() override;

    void loop() override;

private:
    void onKeyDown(uint8_t id);

    void play(const std::string &filePath);

    void cancelDial();

    static int32_t calculateMove(int32_t currentPos, int32_t targetPos);

    void updateChevrons();

    int getCurrentChevron();

    int32_t getCurrentPosition();

    void doChevronAnimation(int chevron);

    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

    void openGate();

private:
    static const int32_t STEPS_PER_ROTATION = 1250;
    static const int32_t MICROSTEPS = 16;
    static const int32_t STEPS_PER_SYMBOL = (STEPS_PER_ROTATION * MICROSTEPS) / 39;
    static const int32_t STEPS_PER_CHEVRON = (STEPS_PER_ROTATION * MICROSTEPS) / 9;

private:
    USB _usb;
    std::vector<uint8_t> _chevronsPressed;
    std::vector<uint8_t> _chevronsQueue;
    static const std::set<std::vector<uint8_t>> _validAddresses;
    static const std::map<std::string, std::vector<uint8_t>> _savedAddresses;
    static const std::map<uint8_t, int> _keycode2LEDIndex;
    static const std::map<int, int> _dhdButtonMapping;
    static const std::map<int, int> _dhdReverseButtonMapping;
    static const std::map<int, char> _indexToChar;
    static const std::vector<uint16_t> _chevronOffsets;
    Audio _audio;
    CRGB _leds[144];
    PCF8575 _ioExpander;
    FastAccelStepperEngine _stepperEngine;
    FastAccelStepper *_stepper;
    std::vector<std::pair<int, int>> _chevronLEDs;
    std::vector<std::pair<int, int>> _chevronMots;
    uint64_t _goHomeTimeout = 0;
    uint16_t _currentPosition = 0;
    int _currentChevron = 0;
    bool _chevronSelectionInProgress = false;
    std::vector<bool> _chevronStatus = {false, false, false, false, false, false, false};
    uint8_t _currentOffset = 0;
    bool _wormholeEstablished = false;
    unsigned long _wormholeAnimationTimestamp = 0;
    uint8_t _wormholeAnimationAlt = 0;
    AsyncWebServer _server;
    AsyncWebSocket _ws;
};


#endif //STARGATE_MAIN_H
