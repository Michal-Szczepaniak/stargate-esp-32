#ifndef MIHAU_TEMP_WIFIHELPER_H
#define MIHAU_TEMP_WIFIHELPER_H

#include <WiFi.h>

class WiFiHelper {
public:
    static void connect() {
        WiFiClass::setHostname("esp32-stargate");
        WiFi.begin("ssid", "pass");
        if (WiFi.waitForConnectResult() != 3) {
            Serial.println("sleep");
            esp_deep_sleep(1 * 1000000);
        }
    }
};


#endif //MIHAU_TEMP_WIFIHELPER_H
