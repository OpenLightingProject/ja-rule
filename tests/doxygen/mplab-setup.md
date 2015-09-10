MPLAB X Development Environment {#mplabx}
============================

[TOC]

# Overview {#mplabx-overview}

This page describes how to setup [MPLAB X](http://www.microchip.com/mplabx)
as a development environment. MPLAB X can have some quirks which can trip up
first time users.

# Harmony {#mplabx-harmony}

[Harmony](http://www.microchip.com/mplabharmony/) is Microchip's
hardware abstraction & middleware libraries. With the exception of the USB
code, we use only the lower level hardware abstraction libraries (typically
PLIB_*). Harmony is still fairly new and after examining some of the code, I
(Simon) have concerns about code quality.

MPLAB X comes with a [Harmony Configurator Plugin (MHC)](
https://microchip.wikidot.com/harmony:mhc-installation)
which auto-generates the boiler plate code from a Harmony configuration.

When used with Harmony, the design of the MPLAB X build / configuration
system makes it difficult to move a project's directory to a new location on
the filesystem. In fact, by default, a project's configurations.xml file
uses relative paths!

This project is set up to assume Harmony 1.02 is installed in
/opt/microchip/harmony/v1_02 . If this is not the case on your system you
have two options:

- Add a symlink from /opt/microchip/harmony/v1_02 pointing to the
  installation path on your system.
- Edit all the project configurations files to fix the path. I don't
  recommend this option as it will result in large diffs when you try to
  merge your changes later.

# Building {#mplabx-building}

First of all, read the [MPLAB X Getting Started Guide](
http://ww1.microchip.com/downloads/en/DeviceDoc/50002027C.pdf)
Getting Started Guide</a>. From within MPLAB X, choose File -> Open Project,
and then select the firmware/ja-rule.X file. Once the project has loaded,
you can perform a build (Run -> Build Project) and program your device.
