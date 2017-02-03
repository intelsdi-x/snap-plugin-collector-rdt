#!/usr/bin/env bash

set -e

sudo yum update
sudo yum install git cmake mc tmux
sudo yum install autoconf automake libtool curl make unzip wget
sudo yum install clang

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++

cd ../intel-cmt-cat/lib/
make SHARED=n
cp libpqos.a ../../../lib/

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

#curl $curlopts -O https://googlemock.googlecode.com/files/gmock-1.7.0.zip
cd third_party/protobuf
wget http://pkgs.fedoraproject.org/repo/pkgs/gmock/gmock-1.7.0.zip/073b984d8798ea1594f5e44d85b20d66/gmock-1.7.0.zip
unzip -q gmock-1.7.0.zip
rm gmock-1.7.0.zip
mv gmock-1.7.0 gmock

./autogen.sh
./configure
make -j2
#make check
sudo make install
sudo ldconfig # refresh shared library cache

cd ../grpc
cd grpc
git submodule update --init
make
sudo make install

cd ../snap-plugin-lib-cpp
mkdir build
./autogen.sh
./configure -prefix=`pwd`/build
make
sudo make install
cp build/lib/libsnap.a ../../lib

export LD_LIBRARY_PATH=/usr/local/lib
