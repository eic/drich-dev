#!/bin/bash

set -e

[[ $# -gt 0 ]] && clean=1 || clean=0 # clean build if any args
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi
if [ "$PRIMARY_PREFIX" = "" ]; then echo "ERROR: PRIMARY_PREFIX not set"; exit 1; fi

pushd ip6/ip6
beamDir=$(pwd -P)
popd

pushd epic

touch ip6
rm ip6
ln -svf $beamDir ip6

if [ $clean -eq 1 ]; then
  echo "--- CLEAN: clean build dir..."
  mkdir -p build
  rm -rv build
  echo "--- CLEAN: remove rendered xml files..."
  rm -vf ecce*.xml
  echo "--- CLEAN: remove variant drich xml files..."
  rm -vf compact/drich_variant*.xml
fi

cmake -B build -S . \
  -DCMAKE_INSTALL_PREFIX=$PRIMARY_PREFIX
cmake --build build -j$BUILD_NPROC -- install

popd
