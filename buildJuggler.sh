#!/bin/bash
# - memory hungry, build with `-j1` or `-j2`
# - kluges around a possible issue in Gaudi (see kluge.sh)

set -e

[[ $# -gt 0 ]] && clean=1 || clean=0 # clean build if any args
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi

cp -v kluge.sh juggler/

pushd juggler

if [ $clean -eq 1 ]; then
  echo "clean build dir..."
  mkdir -p build
  rm -rv build
fi

cmake -B build -S . \
  -DCMAKE_INSTALL_PREFIX=$JUGGLER_INSTALL_PREFIX \
  -DCMAKE_FIND_DEBUG_MODE=OFF
./kluge.sh # execute kluge, to fix build files
cmake --build build -j2 -- install

rm -v kluge.sh

popd
