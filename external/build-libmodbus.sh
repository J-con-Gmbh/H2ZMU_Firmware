#!/bin/bash

workspaceFolder=$(pwd)

buildDirectory=${workspaceFolder}/build/cmake/external/libmodbus
sourceDirectory=${workspaceFolder}/lib/libmodbus

[[ ! -d ${buildDirectory} ]] && mkdir -p ${buildDirectory}

if [[ ! -d ${sourceDirectory} ]]
then
  [[ ! -d ${sourceDirectory} ]] && mkdir "${workspaceFolder}/lib"
  cd "${workspaceFolder}/lib" || exit 1
  git clone -b "v3.1.10" https://github.com/stephane/libmodbus.git
  cd "${workspaceFolder}" || exit 1
fi

cd "${sourceDirectory}" || exit 1

[[ ! -f configure ]] && ./autogen.sh

./configure --host=arm-linux-gnueabihf --enable-static --prefix="${buildDirectory}" || exit 1

make || exit 1
make install || exit 1

exit 0
