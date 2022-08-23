#!/bin/bash

set -e

[[ $# -gt 0 ]] && clean=1 || clean=0 # clean build if any args
if [ -z "$BUILD_NPROC" ]; then export BUILD_NPROC=1; fi
if [ -z "$DRICH_DEV" ]; then echo "ERROR: source environ.sh"; exit 1; fi

pushd eicd

if [ $clean -eq 1 ]; then
  echo "clean build dir..."
  mkdir -p build
  rm -rv build
fi

cmake -B build -S . \
  -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX \
  -DBUILD_DATA_MODEL=ON
cmake --build build -j$BUILD_NPROC -- install

popd
