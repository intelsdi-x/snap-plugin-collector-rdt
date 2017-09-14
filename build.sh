#!/usr/bin/env bash

export CC=/opt/rh/devtoolset-4/root/usr/bin/gcc
export CXX=/opt/rh/devtoolset-4/root/usr/bin/g++
export PATH=/opt/rh/devtoolset-4/root/usr/bin/:$PATH
gcc --version
mkdir -p build
cd build
cmake ..
make -j2
cd ..
