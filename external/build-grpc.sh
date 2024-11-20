#!/bin/bash

if [[ ! -z "$1" && "$1" == "-h" ]]
then
    echo "--full-rebuild"
    exit
fi

workspaceFolder=$(pwd)
buildDirectory=${workspaceFolder}/build/cmake/external/grpc
installDirectory=${buildDirectory}/out
sourceDirectory=${workspaceFolder}/lib/grpc

if [[ ! -d ${sourceDirectory} ]]
then
  [[ ! -d ${sourceDirectory} ]] && mkdir "${workspaceFolder}/lib"
  cd "${workspaceFolder}/lib" || exit 1
  git clone --recurse-submodules -b "v1.45.2" --depth 1 --shallow-submodules https://github.com/grpc/grpc.git
  cd "${workspaceFolder}" || exit 1
fi

[[ ! -d "${buildDirectory}" ]] && mkdir -p "${buildDirectory}"
if [[ -d "${installDirectory}" ]] && [[ -n "$1" ]] && [[ "$1" == "--full-rebuild" ]]
then
    rm -r "${installDirectory}"
fi

cd "${buildDirectory}" || exit 1

cmake --install-prefix "${installDirectory}" -G Ninja -DCMAKE_BUILD_TYPE=Release -DgRPC_BUILD_TESTS=OFF -B"${buildDirectory}" -S"${sourceDirectory}"

cmake --build .
