#ifndef STARGATE_USB_H
#define STARGATE_USB_H

#include "ArduinoClass.h"
#include <Arduino.h>
#include <memory>
#include <vector>
#include <map>
#include "usb/cdc_acm_host.h"
#include "usb/hid_host.h"
#include "usb/hid_usage_keyboard.h"
#include "usb/hid_usage_mouse.h"

class USB : public ArduinoClass {
public:
    USB();

    typedef struct {
        enum key_state {
            KEY_STATE_PRESSED = 0x00,
            KEY_STATE_RELEASED = 0x01
        } state;
        uint8_t modifier;
        uint8_t key_code;
    } key_event_t;

    void setup() override;
    void loop() override;

    void setPixel(int id, bool state) const;
    void clearPixels() const;

    void registerKeyDownCallback(std::function<void(uint8_t)> callback);

    static void handleEventHelper(const cdc_acm_host_dev_event_data_t *event, void *user_ctx) {
        ((USB *) user_ctx)->handleEvent(event);
    }

    static bool handleRxHelper(const uint8_t *data, size_t data_len, void *user_ctx) {
        return ((USB *) user_ctx)->handleRx(data, data_len);
    }

    static void usbLibTaskHelper(void *user_ctx) {
        return ((USB *) user_ctx)->usbLibTask();
    }

    static void usbHIDDriverEventHelper(hid_host_device_handle_t hidDeviceHandle,
                                     const hid_host_driver_event_t event,
                                     void *user_ctx) {
        ((USB *) user_ctx)->usbHIDDriverEvent(hidDeviceHandle, event);
    }

    static void usbHIDInterfaceEventHelper(hid_host_device_handle_t hidDeviceHandle,
                                     const hid_host_interface_event_t event,
                                     void *user_ctx) {
        ((USB *) user_ctx)->usbHIDInterfaceEvent(hidDeviceHandle, event);
    }

private:
    void openCDCDevice();
    void handleEvent(const cdc_acm_host_dev_event_data_t *event);
    bool handleRx(const uint8_t *data, size_t data_len);
    void usbLibTask();
    void usbHIDDriverEvent(hid_host_device_handle_t hidDeviceHandle, hid_host_driver_event_t event);
    void usbHIDInterfaceEvent(hid_host_device_handle_t hidDeviceHandle, hid_host_interface_event_t event);
    static bool keyFound(const uint8_t *src, uint8_t key, unsigned int length);
    static bool hidKeyboardIsModifierShift(uint8_t modifier);
    static bool hidKeyboardGetChar(uint8_t modifier, uint8_t key_code, unsigned char *key_char);
    void keyEventCallback(key_event_t *key_event);

    static void appendStringToMessage(const std::string& text, std::vector<uint8_t> &message) {
        for (char c : text) {
            message.emplace_back(c);
        }
    }

private:
    cdc_acm_dev_hdl_t *_cdcDev;
    static const char *_hidProtoNameStr[];
    static const uint8_t _keycode2ascii[57][2];
    static const std::map<uint8_t, int> _keycode2LEDIndex;
    std::function<void(uint8_t)> _keyDownCallback;
};


#endif //STARGATE_USB_H
