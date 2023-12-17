#include "USB.h"

#include <esp_check.h>
#include "usb/cdc_acm_host.h"
#include "usb/usb_host.h"

USB::USB() : _cdcDev(nullptr), _sent(false) {

}

void USB::setup() {
    const usb_host_config_t host_config = {
            .skip_phy_setup = false,
            .intr_flags = ESP_INTR_FLAG_INTRDISABLED,
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));
    Serial.println("USB host installed");

    BaseType_t task_created = xTaskCreatePinnedToCore(USB::usbLibTaskHelper,
                                                      "usb_events",
                                                      4096,
                                                      xTaskGetCurrentTaskHandle(),
                                                      2, nullptr, 0);
    assert(task_created == pdTRUE);

    Serial.println("Installing CDC-ACM driver");
    ESP_ERROR_CHECK(cdc_acm_host_install(nullptr));
}

void USB::loop() {
    if (_cdcDev == nullptr) {
        openCDCDevice();
    }

    if (_cdcDev != nullptr && !_sent) {
        _sent = true;

        uint8_t data[] = "14,ÿ,ÿ,ÿ;19;";

        esp_err_t err = cdc_acm_host_data_tx_blocking(*_cdcDev, data, sizeof(data), 100);
        if (ESP_OK != err) {
            Serial.println("Error TXing");
        }

        err = cdc_acm_host_set_control_line_state(*_cdcDev, true, true);
        if (ESP_OK != err) {
            Serial.println("Error setting line state");
        }
    }
}

void USB::handleEvent(const cdc_acm_host_dev_event_data_t *event) {
    switch (event->type) {
        case CDC_ACM_HOST_ERROR:
            Serial.printf("CDC-ACM error has occurred, err_no = %d", event->data.error);
            break;
        case CDC_ACM_HOST_DEVICE_DISCONNECTED:
            Serial.printf("Device suddenly disconnected");
            _cdcDev = nullptr;
            break;
        case CDC_ACM_HOST_SERIAL_STATE:
            Serial.printf("Serial state notif 0x%04X", event->data.serial_state.val);
            break;
        case CDC_ACM_HOST_NETWORK_CONNECTION:
        default: break;
    }
}

bool USB::handleRx(const uint8_t *data, size_t data_len) {
    Serial.printf("%.*s", data_len, data);

    return true;
}

void USB::openCDCDevice() {
    const cdc_acm_host_device_config_t devConfig = {
            .connection_timeout_ms = 5000,
            .out_buffer_size = 512,
            .in_buffer_size = 512,
            .event_cb = USB::handleEventHelper,
            .data_cb = USB::handleRxHelper,
            .user_arg = this,
    };

    _cdcDev = new cdc_acm_dev_hdl_t;
    esp_err_t err = cdc_acm_host_open(0x1b4f, 0x9206, 0, &devConfig, _cdcDev);
    if (ESP_OK != err) {
        Serial.println("Error opening device");
        free(_cdcDev);
        _cdcDev = nullptr;
        return;
    }

    Serial.println("Opened CDC device");
}

void USB::usbLibTask() {
    while (true) {
        uint32_t event_flags;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            ESP_ERROR_CHECK(usb_host_device_free_all());
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            Serial.println("USB: All devices freed");
        }
    }
}
