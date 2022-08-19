#!/bin/bash
# - memory hungry, build with `-j1` or `-j2`

set -e

[[ $# -gt 0 ]] && clean=1 || clean=0 # clean build if any args
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi
if [ "$PRIMARY_PREFIX" = "" ]; then echo "ERROR: PRIMARY_PREFIX not set"; exit 1; fi

pushd juggler

if [ $clean -eq 1 ]; then
  echo "clean build dir..."
  mkdir -p build
  rm -rv build
fi

cmake -B build -S . \
  -DCMAKE_INSTALL_PREFIX=$JUGGLER_INSTALL_PREFIX \
  -DCMAKE_FIND_DEBUG_MODE=OFF
cmake --build build -j2 -- install

popd
