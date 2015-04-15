# Testing

This directory contains the tests & test infrastructure for the Ja Rule
platform. We use gmock / gtest.

## Initial setup

gmock / gtest should not be installed system-wide, see
https://code.google.com/p/googletest/wiki/FAQ .

Instead run install-gmock.sh in the top level directory to download and install
a copy.

Once that is complete, in this test directory run:

```
autoreconf -i
./configure
make
make check
```

## Directory Layout

**mocks**, The mocks for each module which are used for dependency injection.

**harmony**, The stubbed out harmony API. We stub / mock out all the harmony
calls we make so that we don't have to pull in harmony when running the tests.

**src**, The src code to test. Source files in this directory are symlinked
backed to /firmware/src,

**system_config**, The system config header files, including system_config.h &
system_definitions.h

**tests**, The unittests.
