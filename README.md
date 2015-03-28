# Ja Rule
[![Build Status](https://travis-ci.org/OpenLightingProject/ja-rule.svg?branch=master)](https://travis-ci.org/OpenLightingProject/ja-rule) [![Coverity Scan Status](https://scan.coverity.com/projects/3938/badge.svg)](https://scan.coverity.com/projects/3938) [![Coverage Status](https://coveralls.io/repos/OpenLightingProject/ja-rule/badge.png?branch=master)](https://coveralls.io/r/OpenLightingProject/ja-rule?branch=master)

Skunkworks project.

## Development Environment Setup

When used with Harmony, the design of the MPLAB X build / configuration system makes it
difficult to move a project's directory to a new location on the filesystem. In fact, by
default, the project's configurations.xml file uses relative paths!

This project is set up to assume Harmony 1.02 is installed in /opt/microchip/harmony/v1_02 .
If this is not the case on your system you have two options:
* Add a symlink from /opt/microchip/harmony/v1_02 pointing to the installation path on your system.
* Edit all the project configurations files to fix the path. I don't recommend this option as it will result in large diffs when you try to merge your changes later.

## Dev Notes

A bulk-in transfer with a full 512 bytes of DMX data takes < 1ms on my mac
laptop. Given this, I felt that a simple request / response model with a double
buffer would suffice,

For DMX / RDM messages, the response message is sent when the transceiver
completes the transaction.


## TODO:

- Implement reset sequence
