#!/usr/bin/env bash

mkdir -p build
cd build
cmake ..
make -j2
cd ..
