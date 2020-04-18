#!/bin/bash
cd tests
if [[ ! -d "build-Release" ]]
then
  mkdir build-Release
fi
qmake tests.pro -o build-Release
cd build-Release
if [[ $config -eq "MacOS" ]]
then
  make
elif [[ $config -eq "Windows" ]]
then
  mingw32-make
fi
result=$?
if [[ $result -eq 0 ]]
then
  ./tests
  result=$?
fi
cd ../..
exit $result
