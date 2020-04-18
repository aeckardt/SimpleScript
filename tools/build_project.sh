#!/bin/bash
mkdir build-Release
qmake SimpleScript.pro -o build-Release
cd build-Release
if [[ $CONFIG -eq "MacOS" ]] then make="make" fi
if [[ $CONFIG -eq "Windows" ]] then make="mingw32-make" fi
eval $make
result=$?
cd ..
exit $result
