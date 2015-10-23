Updating Firmware
================

[TOC]

# Overview {#update-overview}

The **Firmware** is the code that runs on the board.

The **Bootloader** is a mode that allows the firmware to be written to the board

# Application Firmware Update {#update-steps}

## Enter Bootloader Mode {#update-bootloader}
To enter the Bootloader mode press and hold the button on the board when powering up. If you are successful the top LED will be solid and the bottom three LEDS will flash together

## Flash Firmware {#update-flash}
Download the latest firmware package from [github] and note its location
### On Linux {#update-flash-linux}
Download the **dfu-util** application using

`sudo apt-get install dfu-util`

To update the firmware run the following command replacing "firmware.dfu" with the file path to the firmware you downloaded earlier

`sudo dfu-util -a 0 -R -D firmware.dfu`

### On Mac {#update-flash-mac}

### On Windows {#update-flash-windows}

# Bootloader Update {#bootloader-update}
In order to update the bootloader you will need a special set of hardware and software that can interface with the PIC32 microcontroller. 

## Hardware Required {#bootloader-update-hardware}
You will need a PicKit 3 programmer or similar

## Software Required {#bootloader-update-software}
MPLAB IPE

## Steps {#bootloader-update-steps}

[github]:	https://github.com/OpenLightingProject/ja-rule/releases	"github"
