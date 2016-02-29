Introduction     {#mainpage}
================

[TOC]

This guide describes how to get started with a device running the
[Open Lighting Embedded](https://www.openlighting.org/ole/) (OLE) software. For
developer information, including instructions on how to modify the OLE code,
see the [OLE Developer Guide](http://docs.openlighting.org/ole/doc/latest/).

Since the OLE software is customizable, the version running on a particular
device may have different functionality from what is described here. Consult
your manufacturer's documentation on which features are enabled on
each product.

The examples in this guide are from a
[Number1](https://www.openlighting.org/ole/number1/) since that was the first
and most widely available OLE device. Throughout the rest of this document, the
device running OLE is simply referred to as the 'device'.


# Overview {#main-overview}

The device can function as either an
[RDM](https://en.wikipedia.org/wiki/RDM_%28lighting%29) Responder or DMX / RDM
Controller. On boot, the device will start in Responder mode.

## Responder Mode {#main-responder}

In responder mode, the device can simulate different types of RDM models, for
example a moving light or a dimmer. This allows testing of RDM Controller
implementations against a known good RDM Responder implementation. Together the
different RDM Models implement all PIDs from the E1.20, E1.37-1 & E1.37-2
standards.

Responder mode is standalone, a host PC is not required but can be connected to
view the [USB Console](@ref usb-console) logs if desired.

## Controller Mode {#main-controller}

In controller mode, the device operates as a
[USB to DMX / RDM Controller](@ref controller). This mode requires a host
computer running the
[Open Lighting Architecture](https://www.openlighting.org/ola/) (OLA). The OLA
documentation outlines the requirements for the host computer.

When the device is configured as an output port within OLA it will
automatically switch into controller mode.

When operating in controller mode, the OLE device reports detailed timing
information about the RDM responses. The timing stats are reported when running
the [RDM Responder
Tests](https://www.openlighting.org/rdm-tools/rdm-responder-tests/).

# Powering the Device {#main-powering}

The device is powered over the USB port. If the device is connected to a host
PC no additional power source is required.

For standalone operation, the device can be powered by a standard USB cell
phone charger.

# Licensing {#main-licensing}

The OLE codebase is licensed under the
[LGPL](http://www.gnu.org/licenses/lgpl.html).

The unit-testing code & mocks are licenced under the
[GPL](http://www.gnu.org/licenses/gpl.html).
