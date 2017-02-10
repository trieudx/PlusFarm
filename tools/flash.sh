#!/bin/bash

sudo python ./esptool.py --port /dev/ttyUSB0 --baud 460800 erase_flash
sleep 3
sudo python ./esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash --flash_mode qio --flash_freq 80m --flash_size 32m 0x00000 ../output/bin/sprinkler.flash.bin 0x10000 ../output/bin/sprinkler.irom0text.bin 0x3FC000 ../output/bin/esp_init_data_default.bin
