# Ja Rule
[![Build Status](https://travis-ci.org/OpenLightingProject/ja-rule.svg?branch=master)](https://travis-ci.org/OpenLightingProject/ja-rule) [![Coverity Scan Status](https://scan.coverity.com/projects/3938/badge.svg)](https://scan.coverity.com/projects/3938) [![Coverage Status](https://coveralls.io/repos/OpenLightingProject/ja-rule/badge.png?branch=master)](https://coveralls.io/r/OpenLightingProject/ja-rule?branch=master)

Ja Rule is an open source DMX512 / RDM stack for PIC32 microcontrollers. The
software is developed as part of the
[Open Lighting Project](https://www.openlighting.org/).

## Documentation

The [Ja Rule Developer
Documentation](https://docs.openlighting.org/ja-rule/doc/latest/), is targeted
towards people who want to know more about the platform and how to modify it.

The Ja Rule User Guide is not available yet.

## Licensing

The Ja Rule codebase is licensed under the
[LGPL](http://www.gnu.org/licenses/lgpl.html).

The unit-testing code & mocks are licenced under the
[GPL](http://www.gnu.org/licenses/gpl.html).

The hardware designs and the documentation is licensed under the
[Creative Commons BY-SA](https://creativecommons.org/licenses/by-sa/3.0/us/).

## Directory Layout

```
├── Bootloader  # The DFU bootloader
│   └── firmware
│       ├── Bootloader.X  # Bootloader MPLAB X project
│       ├── src  # Bootloader source code
├── boardcfg     # Software configuration for each board
├── common  # Common code shared between the bootloader and application.
├── firmware  # The main DMX/RDM application
│   ├── ja-rule.X
│   └── src
├── linker       # linker scripts for the bootloader & application
├── tests        # Unit tests
├── tools        # tools to upgrade the firmware on the device.
└── user_manual  # The user manual
```

## Getting Started

The firmware/ja-rule.X project can be opened in MPLAB X.

To run the unit tests, you'll need:
 - [OLA](https://www.openlighting.org/ola/getting-started/)
 - [gmock](https://code.google.com/p/googlemock/) / gtest

gmock / gtest should not be installed system-wide, see
https://code.google.com/p/googletest/wiki/FAQ for the reasons.

The install-gmock.sh helper script will download and build gmock & gtest in the
local directory.

Once gmock has been built, run:

```
autoreconf -i
./configure
make
make check
```

## PLASA Identifiers & UIDs

The code by default uses the Open Lighting PLASA ID (0x7a70). This range is
owned by the Open Lighting Project and at this time we do not sub-license
ranges to anyone else. You *may not* ship product with the Open Lighting
PLASA ID.

Per https://wiki.openlighting.org/index.php/Open_Lighting_Allocations the
UIDs 7a70:fffffe00 to 7a70:fffffefe may be used for development (in house)
purposes.

## Dev Notes

A bulk-in transfer with a full 512 bytes of DMX data takes < 1ms on my mac
laptop. Given this, I felt that a simple request / response model with a double
buffer would suffice.

For DMX / RDM messages, the response message is sent when the transceiver
completes the transaction, so the host received positive acknowledgement.
