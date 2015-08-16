This directory contains host side programs that create firmware images for Ja
Rule devices.

You'll need to install [dfu-utils](http://dfu-util.sourceforge.net/) in
order to be able to flash the images to the device.

# Background

Ja Rule devices use the USB
[Device Firmware Upload](http://www.usb.org/developers/docs/devclass_docs/DFU_1.1.pdf)
protocol to transfer binary data to the device's program flash memory. A valid
.dfu file must end with a 16 byte DFU suffix. In addition, the Ja Rule
bootloader expects each DFU transfer to contain a 16-byte header. The programs
in this directory create .dfu files that satisfy these requirements.

With the exception of flash_hex.sh, usage information for each program can be
obtained by passing --help.

# Building the tools

To compile the tools run:

````
$ make
````

# Tools

## hex2dfu

This takes a .hex file, extracts the relevant information and creates the
firmware .dfu file.

````
$ hex2dfu firmware.hex
Wrote 112456 bytes of data to firmware.dfu
````

You can then verify the DFU file with _dfu-suffix_, from the dfu-utils package:

````
$ dfu-suffix -c firmware.dfu
dfu-suffix (dfu-util) 0.7

(C) 2011-2012 Stefan Schmidt
This program is Free Software and has ABSOLUTELY NO WARRANTY
Please report bugs to dfu-util@lists.gnumonks.org

Dfu suffix version 100
The file firmware.dfu contains a DFU suffix with the following properties:
BCD device:     0xFFFF
Product ID:     0xACEE
Vendor ID:      0x1209
BCD DFU:        0x0100
Length:         16
CRC:            0x1B34160B
````

From here you can use _dfu-util_ to flash firmware.dfu to the device:

````
$ dfu-util -D firmware.hex
````

## flash_hex.sh

This automates the running hex2dfu and dfu-util -D above.

## uid2dfu

The Ja Rule bootloader provides a DFU interface to writing the device's RDM
UID. The uid2dfu tool will create a .dfu image containing a UID:

````
$ uid2dfu -m 0x7a70 -d 00000001
UID: 7a70:00000001
Wrote 6 bytes of data to uid.dfu
````

From here you can use _dfu-suffix_ and _dfu-util_ to program the device,
similar to the example above.
