## ESP32-S3 software for stargate board (in other repo)

This repo contains arduino platformio code for the esp32-s3
used in the stargate board I have available in the other repo.

Communication with DHD is achieved via usb host driver. 
Dont' forget to bridge the solder pads!

## Libs

I needed to copy the usb_host_cdc_acm and usb_host_hid from [github.com/espressif/idf-extra-components](https://github.com/espressif/idf-extra-components) 
in order to make it work

CmdMessenger parts from  [github.com/thijse/Arduino-CmdMessenger](https://github.com/thijse/Arduino-CmdMessenger)

## FATFS image

I added custom partitioning for bigger fatfs and boi do we need that extra space for audio.

Here are commands to recreate the fatfs image although it's attached.

Using [github.com/marcmerlin/esp32_fatfsimage](https://github.com/marcmerlin/esp32_fatfsimage)

```
fatfsimage -l5 img.ffat 11712 audio/
esptool.py --chip esp32-s3 --port /dev/ttyUSB0 --baud 921600 write_flash 0x490000 img.ffat
```

Voila.