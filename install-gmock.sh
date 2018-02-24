#!/bin/sh
set -ex

GTEST_DIRECTORY=gtest-read-only
GTEST_REPO=https://github.com/google/googletest.git

if [ -d $GTEST_DIRECTORY ]; then
  cd $GTEST_DIRECTORY;
  git pull;
else
  git clone $GTEST_REPO $GTEST_DIRECTORY
  cd $GTEST_DIRECTORY;
fi
# Ideally do some clever detection of which flavour of gmock people have, for
# now they'll just need to upgrade...
#cd googlemock
autoreconf -fi
./configure
make
