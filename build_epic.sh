#!/bin/bash

set -e

[[ $# -gt 0 ]] && clean=1 || clean=0 # clean build if any args
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi
if [ "$PRIMARY_PREFIX" = "" ]; then echo "ERROR: PRIMARY_PREFIX not set"; exit 1; fi

pushd epic

if [ $clean -eq 1 ]; then
  echo "--- CLEAN: clean build dir..."
  mkdir -p build
  rm -rv build
  echo "--- CLEAN: locally rendered top-level compact files..."
  rm -vf epic*.xml
  echo "--- CLEAN: transient files from scripts/vary_params.rb..."
  rm -vf epic_drich_variant*.xml
  rm -vf compact/drich_variant*.xml
  rm -vf ${DETECTOR_PATH}/epic_drich_variant*.xml
  rm -vf ${DETECTOR_PATH}/compact/drich_variant*.xml
fi

cmake -B build -S . \
  -DCMAKE_INSTALL_PREFIX=$PRIMARY_PREFIX \
  -DIRT_AUXFILE=ON
cmake --build build -j$BUILD_NPROC -- install

popd
