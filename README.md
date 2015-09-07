# Ja Rule
[![Build Status](https://travis-ci.org/OpenLightingProject/ja-rule.svg?branch=master)](https://travis-ci.org/OpenLightingProject/ja-rule) [![Coverity Scan Status](https://scan.coverity.com/projects/3938/badge.svg)](https://scan.coverity.com/projects/3938) [![Coverage Status](https://coveralls.io/repos/OpenLightingProject/ja-rule/badge.png?branch=master)](https://coveralls.io/r/OpenLightingProject/ja-rule?branch=master)

Ja Rule is an open source DMX512 / RDM stack for PIC32 microcontrollers. For
more information see the [Ja Rule
Documentation](https://docs.openlighting.org/ja-rule/doc/latest/).

## Directory Layout

*Bootloader* The bootloader code.

*common* Common code shared between the bootloader and the main application.

*firmware/src* the source code .h and .c files.

*firmware/ja-rule.X* is the MPLABX project.

*linker* Contains the linker scripts used to build the bootloader and
the main application.

*tests* contains the unit tests.

*tools* Tools used to upgrade firmware on the device.

## Licensing

The Ja Rule codebase is licensed under the
[LGPL](http://www.gnu.org/licenses/lgpl.html).

The unit-testing code & mocks are licenced under the
[GPL](http://www.gnu.org/licenses/gpl.html).

## Getting Started

The firmware/ja-rule.X project can be opened in MPLAB X.

To run the unit tests, you'll need:
 - [OLA](https://www.openlighting.org/ola/getting-started/)
 - [gmock](https://code.google.com/p/googlemock/) / gtest

gmock / gtest should not be installed system-wide, see
https://code.google.com/p/googletest/wiki/FAQ for the reasons.

The install-gmock.sh helper script will download and build gmock & gtest in the
local directory.

Once gmock has been built, change into the test/ directory run:

```
autoreconf -i
./configure
make
make check
```

## Documentation

The documentation can be found at
https://docs.openlighting.org/ja-rule/doc/latest/index.html . To generate the
documentation locally run `make doxygen-doc` from the tests/ directory.

## Dev Notes

A bulk-in transfer with a full 512 bytes of DMX data takes < 1ms on my mac
laptop. Given this, I felt that a simple request / response model with a double
buffer would suffice.

For DMX / RDM messages, the response message is sent when the transceiver
completes the transaction, so the host received positive acknowledgement.

