#!/usr/bin/env bash
# Copyright (c) 2017 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


set -e

mkdir -p lib

export CC=/opt/rh/devtoolset-4/root/usr/bin/gcc
export CXX=/opt/rh/devtoolset-4/root/usr/bin/g++
export PATH=/opt/rh/devtoolset-4/root/usr/bin/:$PATH

gcc --version

pushd `pwd`
cd third_party/intel-cmt-cat/lib/
make SHARED=n
cp libpqos.a ../../../lib/
popd

pushd `pwd`
cd ./third_party/googletest
cmake CMakeLists.txt
make -j2
make install
cp googlemock/libgmock.a ../../lib
cp googlemock/gtest/libgtest.a ../../lib
popd

pushd `pwd`
cd ./third_party/grpc
git checkout tags/v1.0.1
git submodule update --init
make -j2
make install

pushd `pwd`
cd ./third_party/protobuf
make clean
./autogen.sh
./configure
make -j2
make install 
popd
popd

pushd `pwd`
cd ./third_party
wget -c 'http://sourceforge.net/projects/boost/files/boost/1.58.0/boost_1_58_0.tar.bz2/download'
tar xf download
rm download
cd ./boost_1_58_0
./bootstrap.sh --show-libraries
./bootstrap.sh
./b2 install --prefix=/usr -j2
popd 

pushd `pwd`
cd ./third_party
git clone https://github.com/gabime/spdlog.git
cd ./spdlog
cp -r include/spdlog /usr/include/
popd

pushd `pwd`
cd ./third_party
wget http://downloads.cpp-netlib.org/0.11.2/cpp-netlib-0.11.2-final.tar.gz
tar xf cpp-netlib-0.11.2-final.tar.gz
rm cpp-netlib-0.11.2-final.tar.gz
cd cpp-netlib-0.11.2-final/
cmake .
make -j2
make install 
popd

pushd `pwd`
cd ./third_party/snap-plugin-lib-cpp
mkdir -p build
./autogen.sh
./configure -prefix=`pwd`/build
make -j2
make install
cp build/lib/libsnap.a ../../lib
popd

# /usr/local/lib is usually not in LD_LIBRARY_PATH
#sudo cp /usr/local/lib/* /usr/lib
export LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib
ldconfig -v
