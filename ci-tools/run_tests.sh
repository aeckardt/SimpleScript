#!/bin/bash
cd tests
if [[ ! -d "build-Release" ]]; then mkdir build-Release; fi;
qmake tests.pro -o build-Release
cd build-Release
"${make_cmd[@]}"
result=$?
if [[ $result -eq 0 ]]; then
  if [[ -f ./tests ]]; then
    ./tests;
  elif [[ -f ./release/tests ]]; then
    ./release/tests;
  fi;
  result=$?;
fi;
cd ../..
exit $result
