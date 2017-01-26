#!/usr/bin/env bash

mkdir -p build
cd build
cmake ..
make -j4
cp snap-plugin-collector-rdt ..
cd ..