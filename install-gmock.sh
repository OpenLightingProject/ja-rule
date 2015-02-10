#!/bin/sh
set -ex
svn checkout http://googlemock.googlecode.com/svn/trunk/ googlemock-read-only
cd googlemock-read-only
autoreconf -fi
./configure
make
cd ..
