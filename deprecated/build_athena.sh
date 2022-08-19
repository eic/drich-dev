#!/bin/bash

set -e

[[ $# -gt 0 ]] && clean=1 || clean=0 # clean build if any args
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi
if [ "$PRIMARY_PREFIX" = "" ]; then echo "ERROR: PRIMARY_PREFIX not set"; exit 1; fi

pushd athena

# clean
if [ $clean -eq 1 ]; then
  echo "clean build dir..."
  mkdir -p build
  rm -rv build
fi

# symlink beamline compact files
printf "\nsymlink beamline to local...\n"
rm -vf ip6
ln -svf $BEAMLINE_PATH/ip6 ip6

# build athena
cmake -B build -S . \
  -DCMAKE_INSTALL_PREFIX=$PRIMARY_PREFIX \
  -DIRT_AUXFILE=ON
cmake --build build -j$BUILD_NPROC -- install

popd

# make legacy prefix compatible with EPIC expectations
printf "\ncompatibility updates for built targets...\n"
rm -vf $DETECTOR_PATH/ip6
ln -svf $BEAMLINE_PATH/ip6 $DETECTOR_PATH/ip6
ln -svf $DETECTOR_PATH/compact/subsystem_views/drich_only.xml $DETECTOR_PATH/${DETECTOR}_drich_only.xml
