Reference Hardware Design {#reference-design}
==============================

[TOC]

# Overview {#reference-overview}

The reference design is based on the
[PIC 32 Ethernet Starter Kit (DM320004-2)](http://www.microchipdirect.com/ProductSearch.aspx?Keywords=DM320004-2)
which uses a MX795F512L MCU.

The [PIC32 USB Starter Kit III
(DM320003-3)](http://www.microchipdirect.com/ProductSearch.aspx?Keywords=DM320003-3)
is cheaper and should also work.
The latter uses a MX470F512L MCU so you may need to change some settings.

You'll also need an I/O breakout board:
[PIC32 I/O Expansion Board (DM320002)]
(http://www.microchipdirect.com/ProductSearch.aspx?keywords=DM320002)

# Pin Assignments {#reference-pin-assignments}

On the DM320004-2 starter kit, with the DM320002 I/O board, the pin
assignments are as follows:

|  Use          | Pin Name    | J1 Connector | J11 Connector | Port  | Bit
| ------------- | ----------- | ------------ | ------------- | ----- | ---
| UART RX       | SCM1A/RF2   | 88           | 41            | -     | -
| UART TX       | SCM1B/RF8   | 90           | 43            | F     | 8
| TX Enable     | PMPD11/RF0  | 18           | 7             | F     | 0
| RX Enable     | PMPD10/RF1  | 16           | 8             | F     | 1
| Input Capture | IC2         | 54           | 26            | -     | -
| RDM Identify  | OC4/RD3     | 40           | 18            | D     | 3
| RDM Mute      | SS1/IC2/RD9 | 52           | 23            | D     | 10

If you use a different MCU, you can change the pin assignments in
system_settings.h

# Transceiver Hardware {#reference-transceiver-hardware}

To build a basic DMX / RDM Transceiver module, you'll need:

- 1 x
  [MAX481CPA]
  (http://datasheets.maximintegrated.com/en/ds/MAX1487-MAX491.pdf)
- 2 x 562 &Omega; resistor
- 1 x 133 &Omega; resistor
- 1 x 0.1 &mu;F capacitor
- 1 x 5 pin XLR connector
- 1 x Breakout board connector, or another means of attaching to J11 on the
- 2 LEDs (Mute & Identify status)
I/O expansion board.

@note This transceiver is not isolated.

@image html transceiver.png "The DMX / RDM Transceiver module"
