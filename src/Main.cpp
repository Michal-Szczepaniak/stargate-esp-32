#include "Main.h"

const std::set<std::vector<uint8_t>> Main::_validAddresses = {
        {17, 31, 15, 35, 25, 13, 34, 0}, // ABYDOS
        {38, 5,  23, 27, 10, 35, 34, 0}, // APOPHIS'S BASE
        {28, 21, 29, 37, 25, 12, 34, 0}, // CASTIANA or SAHAL
        {28, 5,  33, 38, 22, 32, 34, 0}, // SAHAL or CASTIANA
        {37, 2,  6,  15, 24, 38, 34, 0}, // CHULAK
        {4,  22, 12, 23, 13, 31, 34, 0}, // CLAVA THESSARA INFINITAS
        {7,  38, 14, 35, 6,  20, 34, 0}, // CLAVA THESSARA INFINITAS2
        {21, 35, 12, 18, 10, 25, 34, 0}, // DESTROYERS
        {11, 7,  36, 19, 23, 28, 34, 0}, // EARTH
        {11, 27, 14, 37, 15, 21, 34, 0}, // EDORA
        {13, 17, 37, 31, 5,  12, 34, 0}, // EURONDA
        {28, 18, 5,  32, 20, 1,  34, 0}, // JUNA
        {29, 12, 18, 21, 7,  1,  34, 0}, // KALLANA
        {7,  14, 29, 18, 6,  4,  34, 0}, // KHEB
        {5,  2,  13, 25, 7,  9,  34, 0}, // K'TAU
        {30, 25, 35, 31, 23, 26, 34, 0}, // MARTIN LLOYD'S HOMEWORLD
        {27, 11, 15, 14, 21, 33, 34, 0}, // NID OFF-WORLD BASE
        {11, 18, 12, 9,  13, 16, 34, 0}, // P2X-555
        {27, 37, 11, 15, 14, 21, 34, 0}, // P34-353J
        {33, 18, 20, 24, 7,  12, 34, 0}, // P3W-451
        {37, 7,  26, 24, 3,  22, 34, 0}, // P3X-118
        {21, 11, 37, 14, 30, 15, 34, 0}, // P3X-562
        {1,  18, 5,  26, 20, 32, 34, 0}, // P9C-372
        {25, 19, 6,  5,  31, 17, 34, 0}, // PB5-926
        {38, 2,  14, 18, 7,  15, 34, 0}, // PX1 - 767
        {14, 21, 16, 28, 36, 3,  34, 0}, // PRACLARUSH TAONAS
        {13, 33, 26, 37, 9,  5,  34, 0}, // SANGREAL PLANET
        {9,  11, 6,  7,  12, 16, 34, 0}, // TARTARUS
        {29, 9,  17, 24, 23, 5,  34, 0}, // TOLLAN
        {20, 26, 18, 32, 5,  1,  34, 0}, // TOLLANA
        {21, 18, 2,  25, 33, 13, 34, 0}, // VAGON BREI
        {37, 5,  17, 15, 22, 19, 34, 0}, // FINAL DESTINATION
};

const std::map<int, int> Main::_dhdButtonMapping = {
        { 0,  0 },
        { 25, 1 },
        { 2,  2 },
        { 17, 3 },
        { 14, 4 },
        { 18, 5 },
        { 23, 6 },
        { 26, 7 },
        { 39, 8 },
        { 33, 9 },
        { 10, 10 },
        { 28, 11 },
        { 16, 12 },
        { 30, 13 },
        { 35, 14 },
        { 15, 15 },
        { 31, 16 },
        { 27, 17 },
        { 8,  18 },
        { 36, 19 },
        { 4,  20 },
        { 3,  21 },
        { 21, 22 },
        { 11, 23 },
        { 37, 24 },
        { 12, 25 },
        { 34, 26 },
        { 38, 27 },
        { 29, 28 },
        { 6,  29 },
        { 24, 30 },
        { 7,  31 },
        { 22, 32 },
        { 19, 33 },
        { 1,  34 },
        { 32, 35 },
        { 5,  36 },
        { 9,  37 },
        { 20, 38 },
};

const std::vector<uint16_t> Main::_chevronOffsets = {
        0,
        STEPS_PER_CHEVRON * 1,
        STEPS_PER_CHEVRON * 2,
        STEPS_PER_CHEVRON * 3,
        STEPS_PER_CHEVRON * 6,
        STEPS_PER_CHEVRON * 7,
        STEPS_PER_CHEVRON * 8,
        0,
};

Main::Main() : _chevronsPressed({}), _ioExpander(0x20, GPIO_NUM_21, GPIO_NUM_18),
               _chevronLEDs({{GPIO_NUM_9,  LOW},
                             {GPIO_NUM_10, LOW},
                             {GPIO_NUM_14, LOW},
                             {GPIO_NUM_5,  LOW},
                             {GPIO_NUM_7,  LOW},
                             {GPIO_NUM_6,  LOW},
                             {GPIO_NUM_8,  LOW}}),
               _chevronMots({{P13, P10},
                             {P14, P15},
                             {P06,  P07},
                             {P02,  P03},
                             {P04,  P05},
                             {P00,  P01},
                             {P11, P12}}) {
}

void Main::setup() {
    Serial0.begin(115200);
    Serial0.println("start");

    _ioExpander.begin();
    for (std::pair<int, int> led: _chevronMots) {
        _ioExpander.pinMode(led.first, OUTPUT);
        _ioExpander.pinMode(led.second, OUTPUT);
        _ioExpander.digitalWrite(led.first, HIGH);
        _ioExpander.digitalWrite(led.second, HIGH);
    }


    FastLED.addLeds<NEOPIXEL, GPIO_NUM_4>(_leds, 144);
    FastLED.clear(true);

    if (!FFat.begin(false, "", 1)) {
        Serial0.println("FFat Mount Failed");
        return;
    }

    FFat.open("/");
    Serial0.println("File system mounted");

    _audio.setPinout(GPIO_NUM_16, GPIO_NUM_15, GPIO_NUM_17);
    _audio.setVolume(15);
    _audio.forceMono(true);

    _stepperEngine.init();
    _stepper = _stepperEngine.stepperConnectToPin(GPIO_NUM_1);
    _stepper->setDirectionPin(GPIO_NUM_2);
    _stepper->setAutoEnable(true);
    _stepper->setSpeedInHz(200*16);
    _stepper->setAcceleration(200*16);

    for (std::pair<int, int> led: _chevronLEDs) {
        pinMode(led.first, OUTPUT);
    }

    _usb.registerKeyDownCallback([&](uint8_t id) { onKeyDown(id); });
    _usb.setup();
}

void Main::loop() {
    _usb.loop();
    _audio.loop();

    if (_goHomeTimeout != 0 && (millis() - _goHomeTimeout > (1 * 1000))) {
        _goHomeTimeout = 0;
        _stepper->move(calculateMove(getCurrentPosition(), 0));
        _currentPosition = 0;
        _currentOffset = 0;
    }

    if (_wormholeEstablished && !_audio.isRunning()) {
        play("/wormhole-loop.wav");
    }

    if (_wormholeEstablished && millis() - _wormholeAnimationTimestamp > 100) {
        _wormholeAnimationTimestamp = millis();

        for (int i = 0; i < 144; i++) {
            if (i % 5 == _wormholeAnimationAlt) {
                _leds[i] = CRGB(5, 37, 247);
            } else {
                _leds[i] = CRGB(0, 10, 200);
            }
        }

        _wormholeAnimationAlt = (_wormholeAnimationAlt + 1) % 5;
        FastLED.show();
    }

    if (!_stepper->isRunning() && _chevronSelectionInProgress) {
        int currentChevron = getCurrentChevron();
        doChevronAnimation(currentChevron);

        _chevronSelectionInProgress = false;

        if (!_chevronsQueue.empty()) {
            uint8_t nextChevron = _chevronsQueue.front();
            _chevronsQueue.erase(_chevronsQueue.begin());
            _stepper->move(
                    calculateMove(
                            getCurrentPosition(),
                            ((nextChevron-1) * -1 * STEPS_PER_SYMBOL) + _chevronOffsets[currentChevron + 2]
                            ));
            _currentPosition = nextChevron-1;
            _chevronSelectionInProgress = true;
            _currentOffset = currentChevron + 2;
        }
    }
}

void Main::onKeyDown(uint8_t id) {
    if (_wormholeEstablished) {
        if (id > 0) return;

        _wormholeEstablished = false;
        play("/eh_usual_close.wav");

        unsigned long timestamp = millis();
        while (millis() - timestamp < 3000) {
            _audio.loop();
        }

        cancelDial();
        FastLED.clear(true);
        _wormholeAnimationTimestamp = 0;
        return;
    }

    if (std::find(_chevronsPressed.begin(), _chevronsPressed.end(), id) == _chevronsPressed.end()) {
        _chevronsPressed.emplace_back(id);
    } else {
        cancelDial();
        play("/cancel.wav");
    }

    if (_chevronsPressed.size() < 8) {
        if (id == 0) {
            cancelDial();
            play("/cancel.wav");
            return;
        }

        _usb.setPixel(_dhdButtonMapping.at(id), true);
        play("/DHD/dhd_usual_" + std::to_string(random(7) + 1) + ".wav");

        if (!_stepper->isRunning()) {
            int currentChevron = getCurrentChevron();
            _stepper->move(calculateMove(
                    getCurrentPosition(),
                    ((id-1) * -1 * STEPS_PER_SYMBOL) + _chevronOffsets[currentChevron+1]
                    ));
            _currentPosition = id-1;
            _chevronSelectionInProgress = true;
            _currentOffset = currentChevron+1;
        } else {
            _chevronsQueue.emplace_back(id);
        }

        return;
    }


    std::vector<uint8_t> pendingAddress;
    Serial0.print("Chevrons: ");
    for (uint8_t chevron : _chevronsPressed) {
        Serial0.print(_dhdButtonMapping.at(chevron));
        Serial0.print(", ");
        pendingAddress.emplace_back(_dhdButtonMapping.at(chevron));
    }
    Serial0.println();

    if (false == _validAddresses.count(pendingAddress)) {
        Serial0.println("Whoops invalid address, better luck next time…");

        cancelDial();
        play("/dial_fail_sg1.wav");
    } else {
        _usb.setPixel(0, true);

        play("/eh_usual_open.wav");

        unsigned long timestamp = millis();
        while (millis() - timestamp < 1500) {
            _audio.loop();
        }

        for (auto &_led: _leds) {
            _led = CRGB(0, 10, 255);
        }
        FastLED.setBrightness(50);
        FastLED.show();

        timestamp = millis();
        while (millis() - timestamp < 2000) {
            _audio.loop();
        }

        FastLED.setBrightness(20);
        FastLED.show();

        timestamp = millis();
        while (millis() - timestamp < 2500) {
            _audio.loop();
        }

        play("/wormhole-loop.wav");
        _wormholeAnimationTimestamp = millis();
        _wormholeEstablished = true;
    }
}

void Main::play(const std::string& filePath) {
    if (_audio.isRunning()) {
        _audio.stopSong();
    }
    _audio.connecttoFS(FFat, filePath.c_str());
}

void Main::cancelDial() {
    _usb.clearPixels();
    _chevronsPressed.clear();
    _chevronsQueue.clear();
    for (std::pair<int, int> &chevron : _chevronLEDs) {
        chevron.second = LOW;
    }
    updateChevrons();
    _goHomeTimeout = millis();
    _chevronSelectionInProgress = false;
}

int32_t Main::calculateMove(int32_t currentPos, int32_t targetPos) {
    const int32_t halfRotation = (STEPS_PER_ROTATION * MICROSTEPS) / 2;
    return ((currentPos - targetPos + (STEPS_PER_ROTATION * MICROSTEPS) + halfRotation) % (STEPS_PER_ROTATION * MICROSTEPS)) - halfRotation;
}

void Main::updateChevrons() {
    for (std::pair<int, int> chevron : _chevronLEDs) {
        digitalWrite(chevron.first, chevron.second);
    }
}

int Main::getCurrentChevron() {
    auto it = std::find_if(_chevronLEDs.begin(), _chevronLEDs.end(),
                           [&](std::pair<int, int> led){ return led.second == LOW; });

    return std::distance(_chevronLEDs.begin(), it);
}

int32_t Main::getCurrentPosition() {
    return (_currentPosition * STEPS_PER_SYMBOL * -1) + _chevronOffsets[_currentOffset];
}

void Main::doChevronAnimation(int chevron) {
    play("/chev_usual_1.wav");

    unsigned long timestamp = millis();
    while (millis() - timestamp < 300) {
        _audio.loop();
    }

    auto chevronMot = _chevronMots[chevron];
    _ioExpander.digitalWrite(chevronMot.first, HIGH);
    _ioExpander.digitalWrite(chevronMot.second, LOW);


    timestamp = millis();
    while (millis() - timestamp < 100) {
        _audio.loop();
    }

    _ioExpander.digitalWrite(chevronMot.second, HIGH);

    play("/chev_usual_3.wav");
    timestamp = millis();
    while (millis() - timestamp < 200) {
        _audio.loop();
    }

    _chevronLEDs[chevron].second = HIGH;
    updateChevrons();

    timestamp = millis();
    while (millis() - timestamp < 200) {
        _audio.loop();
    }

    play("/chev_usual_2.wav");
    timestamp = millis();
    while (millis() - timestamp < 100) {
        _audio.loop();
    }

    _ioExpander.digitalWrite(chevronMot.first, LOW);

    timestamp = millis();
    while (millis() - timestamp < 100) {
        _audio.loop();
    }

    _ioExpander.digitalWrite(chevronMot.first, HIGH);
}
