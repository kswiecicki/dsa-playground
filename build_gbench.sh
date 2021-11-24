#!/usr/bin/env bash

GOOGLETEST_PATH=$(pwd)/googletest

cd googlebenchmark
rm -rf build
cmake -E make_directory "build"
cmake -E chdir "build" cmake -DGOOGLETEST_PATH=${GOOGLETEST_PATH} -DCMAKE_BUILD_TYPE=Release ../
cmake --build "build" --config Release
