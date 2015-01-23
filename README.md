# Ja Rule
[![Build Status](https://travis-ci.org/OpenLightingProject/ja-rule.svg?branch=master)](https://travis-ci.org/OpenLightingProject/ja-rule)
Skunkworks project.

## Development Environment Setup

When used with Harmony, the design of the MPLAB X build / configuration system makes it
difficult to move a project's directory to a new location on the filesystem. In fact, by
default, the project's configurations.xml file uses relative paths!

This project is set up to assume Harmony 1.02 is installed in /opt/microchip/harmony/v1_02 .
If this is not the case on your system you have two options:
* Add a symlink from /opt/microchip/harmony/v1_02 pointing to the installation path on your system.
* Edit all the project configurations files to fix the path. I don't recommend this option as it will result in large diffs when you try to merge your changes later.
