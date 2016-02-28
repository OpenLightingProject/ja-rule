RDM Responder  {#responder}
================

[TOC]

# Overview {#responder-overview}

The OLE software can simulate different types of RDM Device Models. This
allows RDM controllers to be tested without having to purchase a many
different types of RDM products.

Each Device Model is identified with a 16-bit Device Model ID. See Section
10.5.1 of the RDM Standard for more information on Device Model IDs.

The device can only operate as one model at a time.

Model ID (hex)  | Description
--------------- | -------------
0x0100          | LED Driver
0x0101          | Proxy, which ACK_TIMERs all requests to child devices.
0x0102          | Moving Light
0x0103          | Sensor only device (no DMX footprint)
0x0104          | E1.37-2 Network Device
0x0105          | Dimmer, with sub-devices & status messages.

## Selecting the Device Model ID {#responder-changing-models}

The RDM Device Model is controlled by Manufacturer Specific RDM PIDs. They can
be changed with any RDM Controller software that supports setting Manufacturer
Specific PIDs. These PIDs do not appear in SUPPORTED_PARAMETERS, so in a sense
they are 'hidden'.

### MODEL_ID_LIST

[MODEL_ID_LIST](http://rdm.openlighting.org/pid/display?manufacturer=31344&pid=32771)
will return a list of Device Model IDs. The request PDL is 0.

### MODEL_ID

[MODEL_ID](http://rdm.openlighting.org/pid/display?manufacturer=31344&pid=32770)
is used to Get or Set the current Device Model ID.

### OLA Commands

The PIDs aren't included in the 0.10.0 release of OLA, so you will need to
add [this file](https://raw.githubusercontent.com/OpenLightingProject/ja-rule/master/data/rdm/stellascapes_lightwidgets_number1.proto)
to your PID data directory (normally /usr/local/share/ola/pids or /usr/share/ola/pids) and restart olad.

Once you know the UID of your device. you can use  OLA CLI, to get a list of
available models:

    $ ola_rdm_get --universe 1 --uid 7a70:fffffe00 model_id_list

To change the current model:

    $ ola_rdm_set --universe 1 --uid 7a70:fffffe00 model_id 258

# LED Model {#responder-led}

The LED Model can be used to control SPI LED Pixels.

# Proxy Model {#responder-proxy}

This model simulates a wireless RDM proxy with 2 downstream RDM devices. As
such, all requests to the downstream devices will return an ACK_TIMER. The
response messages can be obtained with a GET QUEUED_MESSAGES.


# Moving Light Model {#responder-moving-light}

This model emulates an RDM enabled Moving Light.

## Supported Parameters

PID                         | Get | Set |
--------------------------- | ----|-----|
COMMS_STATUS                |  Y  |     |
SUPPORTED_PARAMETERS        |  Y  |     |
DEVICE_INFO                 |  Y  |     |
PRODUCT_DETAIL_ID_LIST      |  Y  |     |
DEVICE_MODEL_DESCRIPTION    |  Y  |     |
BOOT_SOFTWARE_VERSION_LABEL |  Y  |     |
DMX_PERSONALITY_DESCRIPTION |  Y  |     |

and more..

# Sensor Model {#responder-sensor}

This model has a DMX footprint of 0.

Sensor Index | Name           | Type, Units                     | Units   | Notes
-------------|----------------|---------------------------------|---------|---------
0            | Temperature    | Temperature, Centigrade         | Deci    | Data is from the onboard temp. sensor if present.
1            | Missing Sensor | Acceleration, meters / second^2 | Deci    | Always NACKs SENSOR_VALUE with NR_HARDWARE_FAULT
2            | Voltage        | Voltage, Volts                  | Milli   | -

# Network Model {#responder-network}

This model implements the PIDs from E1.37-2, IPv4 & DNS Configuration Messages.

Interface Index | Name  | DHCP Supported   |  Notes
----------------|-------|------------------|--------
1               | eth0  |  Yes             | -
3               | tun0  |  No              | -
4               | wlan0 |  Yes             | 1/3rd of DHCP renew requests will fail

# Dimmer {#responder-dimmer}

This model simulates a Dimmer. The root device has a DMX footprint of 0 and
there are 4 sub-devices present.

All PIDs from E1.37-1, Dimmer Message Sets are supported.

## Sub-devices

The dimmer has multiple sub-devices, that each take a single slot of DMX
data. The sub-devices are not contiguous.

DMX_BLOCK_ADDRESS can be used to set the DMX Start Address of all sub-devices in
a single operation.

## Dimmer Settings

Each sub-device of the dimmer implements the PIDs from Section 4 of E1.37-1.
To make things interesting, not all sub-devices support all the dimmer curves /
modulation frequencies.

## Presets & Scenes.

The root device provides 3 scenes. The first scene (index 1) is a factory
programed scene, which can't be modified. The 2nd and 3rd scenes can be
'updated' with capture preset.

DMX_FAIL_MODE and DMX_STARTUP_MODE can be used to change the on-failure and
on-startup scenes.

## Status Messages

Sub devices 1 & 3 will periodically queue status messages, which can be
collected using the STATUS_MESSAGE PID. Subdevice 3 uses a manufacturer
defined status_id, and the string can be retrieved with
STATUS_ID_DESCRIPTION. Status messages from the individual sub devices can
be suppressed with SUB_DEVICE_STATUS_REPORT_THRESHOLD.

## Lock PIN

The root device implements the LOCK_PIN, LOCK_STATE & LOCK_STATE_DESCRIPTION
PIDs. Besides the unlock state (default), there are two custom states. The
1st locks only the sub-devices, the 2nd locks both subdevices and the root
device.

The default lock pin is 0000.

## Self Tests

The root device supports two self tests. The 1st completes in 5s and
always passes. The 2nd takes 20s and always fails. When the test completes a
status message is queued.

