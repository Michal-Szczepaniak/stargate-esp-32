#include "Main.h"

const std::set<std::vector<uint8_t>> Main::_validAddresses = {
        {17, 31, 15, 35, 25, 13, 34}, // ABYDOS
        {38, 5,  23, 27, 10, 35, 34}, // APOPHIS'S BASE
        {28, 21, 29, 37, 25, 12, 34}, // CASTIANA or SAHAL
        {28, 5,  33, 38, 22, 32, 34}, // SAHAL or CASTIANA
        {37, 2,  6,  15, 24, 38, 34}, // CHULAK
        {4,  22, 12, 23, 13, 31, 34}, // CLAVA THESSARA INFINITAS
        {7,  38, 14, 35, 6,  20, 34}, // CLAVA THESSARA INFINITAS2
        {21, 35, 12, 18, 10, 25, 34}, // DESTROYERS
        {11, 7,  36, 19, 23, 28, 34}, // EARTH
        {11, 27, 14, 37, 15, 21, 34}, // EDORA
        {13, 17, 37, 31, 5,  12, 34}, // EURONDA
        {28, 18, 5,  32, 20, 1,  34}, // JUNA
        {29, 12, 18, 21, 7,  1,  34}, // KALLANA
        {7,  14, 29, 18, 6,  4,  34}, // KHEB
        {5,  2,  13, 25, 7,  9,  34}, // K'TAU
        {30, 25, 35, 31, 23, 26, 34}, // MARTIN LLOYD'S HOMEWORLD
        {27, 11, 15, 14, 21, 33, 34}, // NID OFF-WORLD BASE
        {11, 18, 12, 9,  13, 16, 34}, // P2X-555
        {27, 37, 11, 15, 14, 21, 34}, // P34-353J
        {33, 18, 20, 24, 7,  12, 34}, // P3W-451
        {37, 7,  26, 24, 3,  22, 34}, // P3X-118
        {21, 11, 37, 14, 30, 15, 34}, // P3X-562
        {1,  18, 5,  26, 20, 32, 34}, // P9C-372
        {25, 19, 6,  5,  31, 17, 34}, // PB5-926
        {38, 2,  14, 18, 7,  15, 34}, // PX1 - 767
        {14, 21, 16, 28, 36, 3,  34}, // PRACLARUSH TAONAS
        {13, 33, 26, 37, 9,  5,  34}, // SANGREAL PLANET
        {9,  11, 6,  7,  12, 16, 34}, // TARTARUS
        {29, 9,  17, 24, 23, 5,  34}, // TOLLAN
        {20, 26, 18, 32, 5,  1,  34}, // TOLLANA
        {21, 18, 2,  25, 33, 13, 34}, // VAGON BREI
        {37, 5,  17, 15, 22, 19, 34}, // FINAL DESTINATION
};

Main::Main() : _leds(), _stepper(AccelStepper::DRIVER, GPIO_NUM_1, GPIO_NUM_2),
               _ioExpander(0x20, GPIO_NUM_21, GPIO_NUM_18),
               _chevronLEDs({{GPIO_NUM_5,  LOW},
                             {GPIO_NUM_6,  LOW},
                             {GPIO_NUM_7,  LOW},
                             {GPIO_NUM_8,  LOW},
                             {GPIO_NUM_9,  LOW},
                             {GPIO_NUM_10, LOW},
                             {GPIO_NUM_14, LOW}}),
               _chevronMots({{P01,  P02},
                             {P03,  P04},
                             {P05,  P06},
                             {P07,  P10},
                             {P11, P12}}) {

}

void Main::setup() {
    Serial.begin(115200);

    FastLED.addLeds<NEOPIXEL, GPIO_NUM_4>(_leds, 144);
    FastLED.clear(true);
    for (auto &_led: _leds) {
        _led = CRGB(0, 10, 255);
    }
    FastLED.show();
    delay(2000);
    for (auto &_led: _leds) {
        _led = CRGB(0, 00, 200);
    }
    FastLED.show();
    delay(2000);
    FastLED.clear(true);

    if (!FFat.begin(false, "", 1)) {
        Serial.println("FFat Mount Failed");
        return;
    }

    FFat.open("/");
    Serial.println("File system mounted");

    _audio.setPinout(GPIO_NUM_16, GPIO_NUM_15, GPIO_NUM_17);
    _audio.setVolume(15);
    _audio.forceMono(true);

    _stepper.setAcceleration(200.f);
    _stepper.setMaxSpeed(200.f);

    for (std::pair<int, int> led: _chevronLEDs) {
        pinMode(led.first, OUTPUT);
    }

    for (std::pair<int, int> led: _chevronMots) {
        _ioExpander.pinMode(led.first, OUTPUT);
        _ioExpander.pinMode(led.second, OUTPUT);
        _ioExpander.digitalWrite(led.first, HIGH);
        _ioExpander.digitalWrite(led.second, LOW);
    }

    _usb.registerKeyDownCallback([this](uint8_t id) { onKeyDown(id); });
    _usb.setup();
}

void Main::loop() {
    _usb.loop();
    _audio.loop();
    _stepper.run();

    for (std::pair<int, int> led: _chevronLEDs) {
        digitalWrite(led.first, HIGH);
    }
}

void Main::onKeyDown(uint8_t id) {
    if (std::find(_chevronsPressed.begin(), _chevronsPressed.end(), id) == _chevronsPressed.end()) {
        _chevronsPressed.emplace_back(id);
    }

    if (id > 0) {
        _stepper.move(1000.f);
    } else {
        _stepper.move(-1000.f);
    }

    if (_chevronsPressed.size() < 8) {
        if (id == 0) {
            _usb.clearPixels();
            _chevronsPressed.clear();
            play("cancel.wav");
            return;
        }

        _usb.setPixel(id, true);
        play("/DHD/dhd_usual_" + std::to_string(random(7) + 1) + ".wav");
        return;
    }

    if (false == _validAddresses.count(_chevronsPressed)) {
        Serial.println("Whoops invalid address, better luck next time… but continuing anyway");
        _usb.clearPixels();
        _chevronsPressed.clear();
        play("dial_fail_sg1.wav");
    }
}

void Main::play(std::string filePath) {
    _audio.stopSong();
    delay(100);
    _audio.connecttoFS(FFat, filePath.c_str());
}
