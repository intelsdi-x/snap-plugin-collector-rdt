#!/usr/bin/env bash

sudo yum install git cmake
sudo yum install autoconf automake libtool curl make gcc-g++ unzip wget

#curl $curlopts -O https://googlemock.googlecode.com/files/gmock-1.7.0.zip
wget http://pkgs.fedoraproject.org/repo/pkgs/gmock/gmock-1.7.0.zip/073b984d8798ea1594f5e44d85b20d66/gmock-1.7.0.zip
unzip -q gmock-1.7.0.zip
rm gmock-1.7.0.zip
mv gmock-1.7.0 gmock


#run
./autogen.sh
 ./configure
 make -j2
 #make check
 sudo make install
 sudo ldconfig # refresh shared library cache