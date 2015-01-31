#!/bin/sh
set -ex
svn checkout http://googlemock.googlecode.com/svn/trunk/ googlemock-read-only
pushd googlemock-read-only
autoreconf -fi
./configure
make
popd
