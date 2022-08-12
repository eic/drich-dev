#!/bin/bash

set -e

[[ $# -gt 0 ]] && clean=1 || clean=0 # clean build if any args
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi
if [ "$PRIMARY_PREFIX" = "" ]; then echo "ERROR: PRIMARY_PREFIX not set"; exit 1; fi

pushd irt

if [ $clean -eq 1 ]; then
  echo "clean build dir..."
  mkdir -p build
  rm -rv build
fi

cmake -B build -S . \
  -DCMAKE_INSTALL_PREFIX=$PRIMARY_PREFIX \
  -DEVALUATION=OFF
cmake --build build -j$BUILD_NPROC -- install

popd
