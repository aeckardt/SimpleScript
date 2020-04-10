cd ~/tests
mkdir build-Release
qmake tests.pro -o build-Release
cd build-Release
make
./tests -v
