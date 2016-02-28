Connecting to a Host PC {#host}
=======================

[TOC]

# Overview {#usb-logging-overview}

To use the device in [Controller Mode](@ref controller) or to access the
[USB Console](@ref usb-console), the device must be connected to a host PC
using a USB cable.

# Drivers {#host-drivers}

The device enumerates as a [CDC](https://en.wikipedia.org/wiki/USB_communications_device_class)
and as a custom device class. The former is used for the USB Console while the
latter is used for DMX / RDM controller messages.

To use the device as a DMX/RDM controller, [libftdi](https://www.intra2net.com/en/developer/libftdi/)
will need to be installed. See the OLA installation instructions for more
details.

## Mac {#host-drivers-mac}

The CDC driver is pre-installed on Mac OS X.

## Linux {#host-drivers-linux}

The cdc_acm module is included in most distribution's kernels.

# Troubleshooting {#host-troubleshooting}

## Mac {#host-troubleshooting-mac}

Under the Apple menu select *About This Mac*. Then click on *System Report*.

As shown below, under the *USB* section you should see a device called 'Ja Rule' or 'OLE'.

@image html mac-usb.png "OLE device connected to a Mac"

## Linux {#host-troubleshooting-linux}

On Linux, the *lsusb* tool can be used to locate the device.

    $ lsusb -d 1209:aced
    Bus 004 Device 004: ID 1209:aced InterBiometrics

The device will report as a *InterBiometrics* device. Full details are
displayed by passing the -v option:

    $ lsusb -d 1209:aced -v
    Bus 004 Device 004: ID 1209:aced InterBiometrics 
    Device Descriptor:
      bLength                18
      bDescriptorType         1
      bcdUSB               2.00
      bDeviceClass          239 Miscellaneous Device
      bDeviceSubClass         2 ?
      bDeviceProtocol         1 Interface Association
      bMaxPacketSize0        64
      idVendor           0x1209 InterBiometrics
      idProduct          0xaced 
      bcdDevice            0.00
      iManufacturer           1 Open Lighting Project
      iProduct                2 Ja Rule
      iSerial                 3 2dc8:1eaaf990
