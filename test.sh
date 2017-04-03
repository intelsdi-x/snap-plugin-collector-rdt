#!/usr/bin/env bash
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
