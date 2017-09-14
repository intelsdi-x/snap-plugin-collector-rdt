#!/usr/bin/env bash

export PATH=/opt/rh/devtoolset-3/root/usr/bin/:$PATH
gcc --version
mkdir -p build
cd build
cmake ..
make -j2
cd ..
