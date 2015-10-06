Updating Firmware
================

[TOC]

# Overview {#update-overview}

The **Firmware** is the code that runs on the board.

The **Bootloader** is a mode that allows the firmware to be written to the board

# Application Firmware Update {#update-steps}

## Enter Bootloader Mode {#update-bootloader}
To enter the Bootloader mode hold the button closest to the usb port when powering up. If you are successful the top LED will be solid and the bottom three LEDS will flash together

## Flash Firmware {#update-flash}
Download the latest firmware package from [firmware] and note its location
### On Linux {#On-Linux}
Download the **dfu-util** application using

`sudo apt-get install dfu-util`

To update the firmware run the following command replacing "firmware.dfu" with the file path to the firmware you downloaded earlier

`sudo dfu-util -a 0 -R -D firmware.dfu`

### On Mac {#On-Mac}

### On Windows {#On-Windows}

# Bootloader Update {#Bootloader-Update}
In order to update the bootloader you will need a special set of hardware and software that can interface with the pic32 microcontroller. 

## Hardware Required {#Hardware-Required}
You will need a PicKit 3 programmer or similar

## Software Required {#Software-Required}
MPLAB IPE

## Steps {#Steps}

[firmware]:	http://dl.openlighting.org	"firmware"
