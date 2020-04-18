#!/bin/bash
mkdir build-Release
qmake SimpleScript.pro -o build-Release
cd build-Release
echo "config is: $CONFIG"
if [[ $CONFIG -eq "MacOS" ]] then make fi
if [[ $CONFIG -eq "Windows" ]] then mingw32-make fi
result=$?
cd ..
exit $result
