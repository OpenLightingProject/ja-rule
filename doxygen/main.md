Introduction     {#mainpage}
================

[TOC]

# Overview {#main-overview}

OLE is an Open Source DMX512 / RDM stack for PIC32 microcontrollers. It
contains code to function as either an DMX / RDM controller (transmitter) or as an
DMX receiver / RDM responder.

# Features {#main-features}

## USB to DMX / RDM Controller {#main-features-controller}

- Configurable transmit timing parameters, including:
 - Break duration
 - Mark duration
 - RDM response Timeout / Broadcast response timeout
- Response timing information, including:
 - Break time
 - Mark time
 - Response delay

## DMX512 / RDM Receiver {#main-features-responder}

- The ability to simulate many different types of RDM Models.
 Together, they support all PIDs from the E1.20, E1.37-1 & E1.37-2
 standards. This allows RDM controllers to be tested without having access
 to a large number of RDM devices. The code can act as:
 - An LED driver
 - A moving light
 - A proxy, which will ACK_TIMER all requests to the child devices.
 - A sensor only device (No DMX footprint)
 - A dimmer, with sub-devices including all PIDs from E1.37-1
 - A network device, including all PIDs from E1.37-2.
- Configurable RDM response delay, with an option to introduce jitter
- Identify & Mute status indicators.
- RGB pixel control using SPI (LPD8806 only for now).

## Common {#main-features-common}

- USB Logging, with adjustable log levels. Requires Linux or MAC OS X 10.7 or
  greater.
- Extensive @ref testing.

# Supported Platforms {#main-platforms}

This code is targeted at the PIC32 series MCUs. It uses the parts of the
[Harmony](http://www.microchip.com/pagehandler/en_us/devtools/mplabharmony/home.html)
framework, so it won't work on other platforms without some porting effort.

# Terminology {#main-terminology}

<dl>
<dt>Device</dt>
<dd>The embedded system running this code.</dd>
<dt>Host</dt>
<dd>The system that typically runs a full OS, and is the USB / SPI bus
    master.</dd>
<dt>%Message</dt>
<dd>A structured sequence of bytes exchanged between the Host and the
    Device.</dd>
<dt>Responder</dt>
<dd>An RDM Responder, depending on the configuration of a Device, it may
    act as an RDM Responder.</dd>
</dl>

# Getting Started {#main-getting-started}

See the \ref reference-design for the necessary hardware. Once you have the
hardware, the \ref mplabx page explains how to get started building
the firmware.

# Licensing {#main-licensing}

The Ja Rule codebase is licensed under the
[LGPL](http://www.gnu.org/licenses/lgpl.html).

The unit-testing code & mocks are licenced under the
[GPL](http://www.gnu.org/licenses/gpl.html).
