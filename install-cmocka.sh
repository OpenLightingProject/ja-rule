#!/bin/sh
set -ex
wget https://open.cryptomilk.org/attachments/download/42/cmocka-0.4.1.tar.xz
tar -xf cmocka-0.4.1.tar.xz
cd cmocka-0.4.1/src && cmake .. && make && sudo make install
