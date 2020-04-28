#!/bin/bash
cd tests
if [[ ! -d "build-Release" ]]; then mkdir build-Release; fi;
qmake tests.pro -o build-Release
cd build-Release
"${make_cmd[@]}" -f Makefile.Release
result=$?
if [[ $result -eq 0 ]]; then
  ./release/tests;
  result=$?;
fi;
cd ../..
exit $result
