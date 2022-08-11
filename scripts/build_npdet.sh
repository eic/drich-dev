#!/bin/bash
# - build a local copy of NPDet (for testing only)
# - assumes the repo is `./NPDet`

set -e

[[ $# -gt 0 ]] && clean=1 || clean=0 # clean build if any args
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi

pushd NPDet

if [ $clean -eq 1 ]; then
  echo "--- CLEAN: clean build dir..."
  mkdir -p build
  rm -rv build
  mkdir -p install
  rm -rv install
fi

cmake -B build -S . \
  -DCMAKE_INSTALL_PREFIX=install
cmake --build build -j$BUILD_NPROC -- install

popd
