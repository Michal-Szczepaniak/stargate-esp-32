#include "USB.h"

#include <esp_check.h>

#include <utility>
#include "usb/cdc_acm_host.h"
#include "usb/usb_host.h"
#include "CmdMessengerParts.h"

const char *USB::_hidProtoNameStr[] = {
        "NONE",
        "KEYBOARD",
        "MOUSE"
};

const uint8_t USB::_keycode2ascii[57][2] = {
        {0, 0}, /* HID_KEY_NO_PRESS        */
        {0, 0}, /* HID_KEY_ROLLOVER        */
        {0, 0}, /* HID_KEY_POST_FAIL       */
        {0, 0}, /* HID_KEY_ERROR_UNDEFINED */
        {'a', 'A'}, /* HID_KEY_A               */
        {'b', 'B'}, /* HID_KEY_B               */
        {'c', 'C'}, /* HID_KEY_C               */
        {'d', 'D'}, /* HID_KEY_D               */
        {'e', 'E'}, /* HID_KEY_E               */
        {'f', 'F'}, /* HID_KEY_F               */
        {'g', 'G'}, /* HID_KEY_G               */
        {'h', 'H'}, /* HID_KEY_H               */
        {'i', 'I'}, /* HID_KEY_I               */
        {'j', 'J'}, /* HID_KEY_J               */
        {'k', 'K'}, /* HID_KEY_K               */
        {'l', 'L'}, /* HID_KEY_L               */
        {'m', 'M'}, /* HID_KEY_M               */
        {'n', 'N'}, /* HID_KEY_N               */
        {'o', 'O'}, /* HID_KEY_O               */
        {'p', 'P'}, /* HID_KEY_P               */
        {'q', 'Q'}, /* HID_KEY_Q               */
        {'r', 'R'}, /* HID_KEY_R               */
        {'s', 'S'}, /* HID_KEY_S               */
        {'t', 'T'}, /* HID_KEY_T               */
        {'u', 'U'}, /* HID_KEY_U               */
        {'v', 'V'}, /* HID_KEY_V               */
        {'w', 'W'}, /* HID_KEY_W               */
        {'x', 'X'}, /* HID_KEY_X               */
        {'y', 'Y'}, /* HID_KEY_Y               */
        {'z', 'Z'}, /* HID_KEY_Z               */
        {'1', '!'}, /* HID_KEY_1               */
        {'2', '@'}, /* HID_KEY_2               */
        {'3', '#'}, /* HID_KEY_3               */
        {'4', '$'}, /* HID_KEY_4               */
        {'5', '%'}, /* HID_KEY_5               */
        {'6', '^'}, /* HID_KEY_6               */
        {'7', '&'}, /* HID_KEY_7               */
        {'8', '*'}, /* HID_KEY_8               */
        {'9', '('}, /* HID_KEY_9               */
        {'0', ')'}, /* HID_KEY_0               */
        {'\r', '\r'}, /* HID_KEY_ENTER           */
        {0, 0}, /* HID_KEY_ESC             */
        {'\b', 0}, /* HID_KEY_DEL             */
        {0, 0}, /* HID_KEY_TAB             */
        {' ', ' '}, /* HID_KEY_SPACE           */
        {'-', '_'}, /* HID_KEY_MINUS           */
        {'=', '+'}, /* HID_KEY_EQUAL           */
        {'[', '{'}, /* HID_KEY_OPEN_BRACKET    */
        {']', '}'}, /* HID_KEY_CLOSE_BRACKET   */
        {'\\', '|'}, /* HID_KEY_BACK_SLASH      */
        {'\\', '|'}, /* HID_KEY_SHARP           */  // HOTFIX: for NonUS Keyboards repeat HID_KEY_BACK_SLASH
        {';', ':'}, /* HID_KEY_COLON           */
        {'\'', '"'}, /* HID_KEY_QUOTE           */
        {'`', '~'}, /* HID_KEY_TILDE           */
        {',', '<'}, /* HID_KEY_LESS            */
        {'.', '>'}, /* HID_KEY_GREATER         */
        {'/', '?'} /* HID_KEY_SLASH           */
};

const std::map<uint8_t, int> USB::_keycode2LEDIndex = {
        { 'A', 0 },
        { 'B', 1 },
        { 'C', 2 },
        { 'D', 3 },
        { 'E', 4 },
        { 'F', 5 },
        { 'G', 6 },
        { 'H', 7 },
        { 'I', 8 },
        { 'J', 9 },
        { 'K', 10 },
        { 'L', 11 },
        { 'M', 12 },
        { 'N', 13 },
        { 'O', 14 },
        { 'P', 15 },
        { 'Q', 16 },
        { 'R', 17 },
        { 'S', 18 },
        { 'T', 19 },
        { 'U', 20 },
        { 'V', 21 },
        { 'W', 22 },
        { 'X', 23 },
        { 'Y', 24 },
        { 'Z', 25 },
        { '0', 26 },
        { '1', 27 },
        { '2', 28 },
        { '3', 29 },
        { '4', 30 },
        { '5', 31 },
        { '6', 32 },
        { '7', 33 },
        { '8', 34 },
        { '9', 35 },
        { 'a', 36 },
        { 'b', 37 },
        { 'c', 38 },
};

USB::USB() : _cdcDev(nullptr), _keyDownCallback(nullptr) {

}

void USB::setup() {
    const usb_host_config_t host_config = {
            .skip_phy_setup = false,
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));
    Serial.println("USB host installed");

    BaseType_t task_created = xTaskCreatePinnedToCore(USB::usbLibTaskHelper,
                                                      "usb_events",
                                                      4096,
                                                      this,
                                                      2, nullptr, 0);
    assert(task_created == pdTRUE);

    Serial.println("Installing CDC-ACM driver");
    ESP_ERROR_CHECK(cdc_acm_host_install(nullptr));

    Serial.println("Installing USB driver");
    const hid_host_driver_config_t hid_host_driver_config = {
            .create_background_task = true,
            .task_priority = 5,
            .stack_size = 4096,
            .core_id = 0,
            .callback = USB::usbHIDDriverEventHelper,
            .callback_arg = this
    };

    ESP_ERROR_CHECK(hid_host_install(&hid_host_driver_config));
}

void USB::loop() {
    if (_cdcDev == nullptr) {
        openCDCDevice();
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
//    Serial.printf("%.*s", data_len, data);

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
    delay(2000);
}

void USB::usbLibTask() {
    Serial.println("Usb lib task");
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

void USB::usbHIDDriverEvent(hid_host_device_handle_t hidDeviceHandle, const hid_host_driver_event_t event) {
    hid_host_dev_params_t dev_params;
    ESP_ERROR_CHECK(hid_host_device_get_params(hidDeviceHandle, &dev_params));

    switch (event) {
        case HID_HOST_DRIVER_EVENT_CONNECTED: {
            Serial.printf("HID Device, protocol '%s' CONNECTED\n",
                          _hidProtoNameStr[dev_params.proto]);

            const hid_host_device_config_t dev_config = {
                    .callback = USB::usbHIDInterfaceEventHelper,
                    .callback_arg = this
            };

            ESP_ERROR_CHECK(hid_host_device_open(hidDeviceHandle, &dev_config));
            if (HID_SUBCLASS_BOOT_INTERFACE == dev_params.sub_class) {
                ESP_ERROR_CHECK(hid_class_request_set_protocol(hidDeviceHandle, HID_REPORT_PROTOCOL_BOOT));
                if (HID_PROTOCOL_KEYBOARD == dev_params.proto) {
                    ESP_ERROR_CHECK(hid_class_request_set_idle(hidDeviceHandle, 0, 0));
                }
            }
            ESP_ERROR_CHECK(hid_host_device_start(hidDeviceHandle));
            break;
        }
        default:
            break;
    }
}

void USB::usbHIDInterfaceEvent(hid_host_device_handle_t hidDeviceHandle, const hid_host_interface_event_t event) {
    uint8_t data[64] = { 0 };
    size_t data_length = 0;
    hid_host_dev_params_t dev_params;
    ESP_ERROR_CHECK(hid_host_device_get_params(hidDeviceHandle, &dev_params));

    switch (event) {
        case HID_HOST_INTERFACE_EVENT_INPUT_REPORT: {
            ESP_ERROR_CHECK(hid_host_device_get_raw_input_report_data(hidDeviceHandle,
                                                                      data,
                                                                      64,
                                                                      &data_length));

            hid_keyboard_input_report_boot_t *kb_report = (hid_keyboard_input_report_boot_t *) data;

            if (data_length < sizeof(hid_keyboard_input_report_boot_t)) {
                return;
            }

            static uint8_t prev_keys[HID_KEYBOARD_KEY_MAX] = {0};
            key_event_t key_event;

            for (int i = 0; i < HID_KEYBOARD_KEY_MAX; i++) {

                // key has been released verification
                if (prev_keys[i] > HID_KEY_ERROR_UNDEFINED &&
                    !keyFound(kb_report->key, prev_keys[i], HID_KEYBOARD_KEY_MAX)) {
                    key_event.key_code = prev_keys[i];
                    key_event.modifier = 0;
                    key_event.state = key_event_t::KEY_STATE_RELEASED;
                    keyEventCallback(&key_event);
                }

                // key has been pressed verification
                if (kb_report->key[i] > HID_KEY_ERROR_UNDEFINED &&
                    !keyFound(prev_keys, kb_report->key[i], HID_KEYBOARD_KEY_MAX)) {
                    key_event.key_code = kb_report->key[i];
                    key_event.modifier = kb_report->modifier.val;
                    key_event.state = key_event_t::KEY_STATE_PRESSED;
                    keyEventCallback(&key_event);
                }
            }

            memcpy(prev_keys, &kb_report->key, HID_KEYBOARD_KEY_MAX);

            break;
        }
        case HID_HOST_INTERFACE_EVENT_DISCONNECTED:
            Serial.printf("HID Device, protocol '%s' DISCONNECTED",
                          _hidProtoNameStr[dev_params.proto]);
            ESP_ERROR_CHECK(hid_host_device_close(hidDeviceHandle));
            break;
        case HID_HOST_INTERFACE_EVENT_TRANSFER_ERROR:
            Serial.printf("HID Device, protocol '%s' TRANSFER_ERROR",
                          _hidProtoNameStr[dev_params.proto]);
            break;
        default:
            Serial.printf("HID Device, protocol '%s' Unhandled event",
                          _hidProtoNameStr[dev_params.proto]);
            break;
    }
}

bool USB::keyFound(const uint8_t *const src, uint8_t key, unsigned int length) {
    for (unsigned int i = 0; i < length; i++) {
        if (src[i] == key) {
            return true;
        }
    }
    return false;
}

bool USB::hidKeyboardIsModifierShift(uint8_t modifier) {
    return ((modifier & HID_LEFT_SHIFT) == HID_LEFT_SHIFT) || ((modifier & HID_RIGHT_SHIFT) == HID_RIGHT_SHIFT);
}

bool USB::hidKeyboardGetChar(uint8_t modifier, uint8_t key_code, unsigned char *key_char) {
    uint8_t mod = hidKeyboardIsModifierShift(modifier);

    if ((key_code >= HID_KEY_A) && (key_code <= HID_KEY_SLASH)) {
        *key_char = _keycode2ascii[key_code][mod];
    } else {
        return false;
    }

    return true;
}

void USB::keyEventCallback(USB::key_event_t *key_event) {
    unsigned char keyChar;

    if (key_event_t::KEY_STATE_PRESSED == key_event->state) {
        if (hidKeyboardGetChar(key_event->modifier, key_event->key_code, &keyChar)) {
            try {
                if (_keyDownCallback != nullptr) {
                    _keyDownCallback(_keycode2LEDIndex.at(keyChar));
                }
            } catch (std::out_of_range &e) {

            }
        }
    }
}

void USB::setPixel(int id, bool state) const {
    std::vector<uint8_t> message;
    appendStringToMessage("15,", message);
    std::vector<uint8_t> paramEncoded = CmdMessengerParts::writeBin<int>(id);
    message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());

    appendStringToMessage(",", message);

    if (!state) {
        paramEncoded = CmdMessengerParts::writeBin<int>(0);
        message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
        appendStringToMessage(",", message);

        paramEncoded = CmdMessengerParts::writeBin<int>(0);
        message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
        appendStringToMessage(",", message);

        paramEncoded = CmdMessengerParts::writeBin<int>(0);
        message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
    } else if (id == 0) {
        paramEncoded = CmdMessengerParts::writeBin<int>(250);
        message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
        appendStringToMessage(",", message);

        paramEncoded = CmdMessengerParts::writeBin<int>(0);
        message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
        appendStringToMessage(",", message);

        paramEncoded = CmdMessengerParts::writeBin<int>(0);
        message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
    } else {
        paramEncoded = CmdMessengerParts::writeBin<int>(250);
        message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
        appendStringToMessage(",", message);

        paramEncoded = CmdMessengerParts::writeBin<int>(117);
        message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
        appendStringToMessage(",", message);

        paramEncoded = CmdMessengerParts::writeBin<int>(0);
        message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
    }

    appendStringToMessage(";19;", message);

    esp_err_t err = cdc_acm_host_data_tx_blocking(*_cdcDev, message.data(), message.size(), 100);
    if (ESP_OK != err) {
        Serial.println("Error TXing");
    }
}

void USB::clearPixels() const {
    std::vector<uint8_t> message;
    appendStringToMessage("14,", message);
    std::vector<uint8_t> paramEncoded;

    paramEncoded = CmdMessengerParts::writeBin<int>(0);
    message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
    appendStringToMessage(",", message);

    paramEncoded = CmdMessengerParts::writeBin<int>(0);
    message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());
    appendStringToMessage(",", message);

    paramEncoded = CmdMessengerParts::writeBin<int>(0);
    message.insert(message.end(), paramEncoded.begin(), paramEncoded.end());

    appendStringToMessage(";19;", message);

    esp_err_t err = cdc_acm_host_data_tx_blocking(*_cdcDev, message.data(), message.size(), 100);
    if (ESP_OK != err) {
        Serial.println("Error TXing");
    }
}

void USB::registerKeyDownCallback(std::function<void(uint8_t)> callback) {
    _keyDownCallback = std::move(callback);
}

