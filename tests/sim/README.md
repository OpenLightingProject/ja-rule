# PIC32 Simulator

This directory contains a very basic PIC32 simulator used for the Transceiever
unit tests.

The simulator implements certain peripherals which behave the same as the actual
hardware on the MCU. This allows us to run the Transceiever code and verify the
state machine operates as expected.

## Supported Peripherals

- Input Capture
- Timer
- USART, only 8N2 mode.

## Limitations

The key limitation is that we execute the Tasks() function once per virtual
clock cycle. This means that ISRs will only run between calls to Tasks().

As a result, we don't test iterleaving of ISRs with the main tasks function. I
thought about trying to do this but instruction re-ordering makes this
difficult (impossible?).

## Signal Generator

The Signal Generator allows us to create a series of input events for the UART
& IC modules. This simulates receiving a DMX / RDM signal.
