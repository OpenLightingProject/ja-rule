#!/bin/sh
set -ex

GTEST_DIRECTORY=gtest-read-only
GTEST_REPO=https://github.com/google/googletest.git

git clone $GTEST_REPO $GTEST_DIRECTORY
cd $GTEST_DIRECTORY/googlemock
autoreconf -fi
./configure
make
