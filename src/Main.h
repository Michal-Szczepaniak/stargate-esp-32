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
    static const std::map<int, int> _dhdButtonMapping;
    static const std::map<int, int> _dhdReverseButtonMapping;
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

public:
    const std::map<uint8_t, int> _keycode2LEDIndex = {
            {'A', 0},
            {'8', 1},
            {'C', 2},
            {'V', 3},
            {'U', 4},
            {'a', 5},
            {'3', 6},
            {'5', 7},
            {'S', 8},
            {'b', 9},
            {'K', 10},
            {'X', 11},
            {'Z', 12},
            {'?', 13},
            {'E', 14},
            {'P', 15},
            {'M', 16},
            {'D', 17},
            {'F', 18},
            {'7', 19},
            {'c', 20},
            {'W', 21},
            {'6', 22},
            {'G', 23},
            {'4', 24},
            {'B', 25},
            {'H', 26},
            {'R', 27},
            {'L', 28},
            {'2', 29},
            {'N', 30},
            {'Q', 31},
            {'9', 32},
            {'J', 33},
            {'0', 34},
            {'O', 35},
            {'T', 36},
            {'Y', 37},
            {'1', 38},
            {'I', 39},
    };
    const std::map<int, char> _indexToChar = {
            {0, 'A'},
            {1, 'A'},
            {2, 'B'},
            {3, 'C'},
            {4, 'D'},
            {5, 'E'},
            {6, 'F'},
            {7, 'G'},
            {8, 'H'},
            {9, 'I'},
            {10, 'J'},
            {11, 'K'},
            {12, 'L'},
            {13, 'M'},
            {14, 'N'},
            {15, 'O'},
            {16, 'P'},
            {17, 'Q'},
            {18, 'R'},
            {19, 'S'},
            {20, 'T'},
            {21, 'U'},
            {22, 'V'},
            {23, 'W'},
            {24, 'X'},
            {25, 'Y'},
            {26, 'Z'},
            {27, 'a'},
            {28, 'b'},
            {29, 'c'},
            {30, 'd'},
            {31, 'e'},
            {32, 'f'},
            {33, 'g'},
            {34, 'h'},
            {35, 'i'},
            {36, 'j'},
            {37, 'k'},
            {38, 'l'},
            {39, 'm'},
    };
};


#endif //STARGATE_MAIN_H
