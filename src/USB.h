#ifndef STARGATE_USB_H
#define STARGATE_USB_H

#include "ArduinoClass.h"
#include <Arduino.h>
#include <memory>
#include "usb/cdc_acm_host.h"
#include "usb/hid_host.h"
#include "usb/hid_usage_keyboard.h"
#include "usb/hid_usage_mouse.h"

class USB : public ArduinoClass {
public:
    USB();

    void setup() override;
    void loop() override;

    static void handleEventHelper(const cdc_acm_host_dev_event_data_t *event, void *user_ctx) {
        ((USB *) user_ctx)->handleEvent(event);
    }

    static bool handleRxHelper(const uint8_t *data, size_t data_len, void *user_ctx) {
        return ((USB *) user_ctx)->handleRx(data, data_len);
    }

    static void usbLibTaskHelper(void *user_ctx) {
        return ((USB *) user_ctx)->usbLibTask();
    }

private:
    void openCDCDevice();
    void handleEvent(const cdc_acm_host_dev_event_data_t *event);
    bool handleRx(const uint8_t *data, size_t data_len);
    void usbLibTask();

private:
    cdc_acm_dev_hdl_t *_cdcDev;
    bool _sent;
};


#endif //STARGATE_USB_H
