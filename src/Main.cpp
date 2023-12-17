#include "Main.h"

void Main::setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    _usb.setup();
}

void Main::loop() {
    _usb.loop();
}
