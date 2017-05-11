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


if [ -z "$TEST_TYPE" ]; then
    echo "Please set TEST_TYPE environment variable!"
    exit 1
fi

rm ./build/test-${TEST_TYPE}
./build.sh

echo "--------------"
echo "RUNNING TESTS:"
echo "--------------"
./build/test-${TEST_TYPE}

echo "------------------------"
echo "CHECKING TESTS COVERAGE:"
echo "------------------------"
gcov -n ./build/CMakeFiles/test-medium.dir/src/${TEST_TYPE}_test.cpp.o | grep "${TEST_TYPE}_test.cpp" -A2
