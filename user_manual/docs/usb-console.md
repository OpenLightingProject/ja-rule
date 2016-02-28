USB Console  {#usb-console}
================

[TOC]

# Overview {#usb-console-overview}

The device provides a serial console over the USB port. This allows a
[terminal emulator](
https://en.wikipedia.org/wiki/Terminal_emulation)
such as [minicom](http://linux.die.net/man/1/minicom) (Unix) or
[PuTTY](http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html)
(Windows) to communicate with the device.

The settings are 8N1, at 9600 baud.

The console provides different log levels. The +/- keys can be used to
increase / decrease the log level. Press the 'h' key to see a list of
commands.

# Connecting {#usb-console-connecting}

## Linux {#usb-console-linux}

On Linux, minicom can be used. The device will usually appear as an ACM device.

     $ minicom  -D /dev/ttyACM0

## Mac OS {#usb-console-mac}

On Mac OS X, minicom can be used. The device will appear as a usbmodem:

    $ minicom  -D /dev/tty.usbmodem14121

The device name may be different on your system.

## Windows {#usb-console-windows}

TODO: describe this.

# Commands {#usb-console-commands}

Key | Action
--- | -------------
c   | Show the receive frame counters.
h   | Show the help message.
m   | Show the mode of the device.
t   | Show the 485 transceiver timing settings.
u   | Show the UID of the device.
