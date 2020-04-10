#!/bin/bash
mkdir build-Release
qmake SimpleScript.pro -o build-Release
cd build-Release
make
cd ..
