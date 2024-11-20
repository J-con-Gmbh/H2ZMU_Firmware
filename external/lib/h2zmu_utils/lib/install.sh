#!/bin/bash

[[ -d build ]] || mkdir build

cd build

cmake ..
make
sudo make install
