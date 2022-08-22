#!/bin/bash

set -e

[[ $# -gt 0 ]] && clean=1 || clean=0 # clean build if any args
if [ -z "$BUILD_NPROC" ]; then export BUILD_NPROC=1; fi
if [ -z "$DRICH_DEV" ]; then echo "ERROR: source environ.sh first"; exit 1; fi

pushd ip6

if [ $clean -eq 1 ]; then
  echo "clean build dir..."
  mkdir -p build
  rm -rv build
fi

cmake -B build -S . \
  -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX
cmake --build build -j$BUILD_NPROC -- install

popd
