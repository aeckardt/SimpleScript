#!/bin/bash
mkdir build-Release
qmake SimpleScript.pro -o build-Release
cd build-Release
if [[ $CONFIG -eq "MacOS" ]] then make_cmd=(make) fi
if [[ $CONFIG -eq "Windows" ]] then make_cmd=(mingw32-make) fi
"${make_cmd[@]}"
result=$?
cd ..
exit $result
