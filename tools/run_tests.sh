#!/bin/bash
cd tests
mkdir build-Release
qmake tests.pro -o build-Release
cd build-Release
make
result=$?
if [[ $result -eq 0 ]]
then
  ./tests
  result=$?
fi
cd ../..
exit $result
