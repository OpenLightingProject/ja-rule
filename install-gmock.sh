#!/bin/sh
set -ex

GTEST_DIRECTORY=gtest-read-only

#svn checkout http://googlemock.googlecode.com/svn/trunk/ googlemock-read-only
git clone git@github.com:google/googletest.git $GTEST_DIRECTORY
cd $GTEST_DIRECTORY/googlemock
autoreconf -fi
./configure
make
