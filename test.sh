#!/usr/bin/env bash
echo "RUNNING TESTS:"
./build/test-medium

echo "------------------"
echo "CHECKING TESTS COVERAGE:"
echo "------------------"
gcov -n ./build/CMakeFiles/test-medium.dir/src/medium_test.cpp.o | grep "medium_test.cpp" -A2
