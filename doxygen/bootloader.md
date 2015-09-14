Bootloader
================

[TOC]

# Overview {#bootloader-overview}

The bootloader is used to flash new firmware onto the Device, without having
to use extra hardware like a
[PICkit3](http://www.microchip.com/Developmenttools/ProductDetails.aspx?PartNO=PG164130)
. This makes it easier for end users to upgrade their devices.

The bootloader uses the 
[Device Firmware Upgrade (DFU)]
(http://www.usb.org/developers/docs/devclass_docs/DFU_1.1.pdf)
protocol. This is a standard protocol for upgrading firmware over USB.

@section bootloader_procedure Update Procedure

Generally the update procedure is:
- Download or create a new firmware image.
- Activate the bootloader mode on the device.
- Perform the update.
- Run the new code.
