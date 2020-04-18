#!/bin/bash
config=$1
mkdir build-Release
qmake SimpleScript.pro -o build-Release
cd build-Release
echo "config is: $config"
if [[ $config -eq "MacOS" ]]
then
  make
fi
if [[ $config -eq "Windows" ]]
then
  mingw32-make
fi
result=$?
cd ..
exit $result
