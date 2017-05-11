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

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++

pushd `pwd`
cd third_party/intel-cmt-cat/lib/
make SHARED=n
cp libpqos.a ../../../lib/
popd

if [ ! -d "./third_party/protobuf/gmock" ]
then
pushd `pwd`
cd ./third_party/protobuf
wget http://pkgs.fedoraproject.org/repo/pkgs/gmock/gmock-1.7.0.zip/073b984d8798ea1594f5e44d85b20d66/gmock-1.7.0.zip
unzip -q gmock-1.7.0.zip
rm gmock-1.7.0.zip
mv gmock-1.7.0 gmock
popd
fi

pushd `pwd`
cd ./third_party/protobuf
./autogen.sh
./configure
make -j2
make install
popd

pushd `pwd`
cd ./third_party/grpc
git checkout tags/v1.0.1
git submodule update --init
make -j2
make install
popd

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

pushd `pwd`
cd ./third_party/snap-plugin-lib-cpp
mkdir -p build
./autogen.sh
./configure -prefix=`pwd`/build
make -j2
make install
cp build/lib/libsnap.a ../../lib
popd

pushd `pwd`
cd ./third_party/googletest
cmake CMakeLists.txt
make -j2
make install
cp googlemock/libgmock.a ../../lib
cp googlemock/gtest/libgtest.a ../../lib
popd

# /usr/local/lib is usually not in LD_LIBRARY_PATH
#sudo cp /usr/local/lib/* /usr/lib
ldconfig -v
