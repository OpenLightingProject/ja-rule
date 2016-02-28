Updating Firmware
================

[TOC]

# Overview {#update-overview}

The **Firmware** is the OLE code image that runs on the device during normal
operating conditions.

The **Bootloader** is a mode that allows the firmware to be written to the
device.

The bootloader uses the 
[Device Firmware Upgrade (DFU)]
(https://en.wikipedia.org/wiki/USB#DFU)
protocol to transfer the firmware image from the host. This is a standard
protocol for upgrading firmware over USB. Any software that allows DFU uploads
should work correctly with the device.

# Application Firmware Update {#update-steps}

## Flash Firmware {#update-flash}
Download the latest firmware package from [GitHub] and note its location.

### On Linux {#update-flash-linux}
Download the **dfu-util** application using

    $ sudo apt-get install dfu-util

To update the firmware run the following command replacing "firmware.dfu" with the file path to the firmware you downloaded earlier

    $ sudo dfu-util -a 0 -R -D firmware.dfu

### On Mac {#update-flash-mac}

The same instructions as Linux.

### On Windows {#update-flash-windows}

TODO: describe this.

# Manually Enter Bootloader Mode {#update-bootloader}

This is not necessary if the procedure above is used.

To force the device to enter the Bootloader mode, press and hold the button on
the device when connecting a power source. If the device enters bootloader mode
LEDs A, B & C will flash simultaneously.

[GitHub]:	https://github.com/OpenLightingProject/ja-rule/releases	"github"
