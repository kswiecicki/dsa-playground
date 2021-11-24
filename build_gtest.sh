#!/usr/bin/env bash

cd googletest
rm -rf build
mkdir build
cd build
cmake ..
make -j
