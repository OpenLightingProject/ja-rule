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
cd googlemock
mkdir build
cd build
cmake ..
