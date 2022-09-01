#!/bin/bash
# - build a local copy of DD4hep
# - assumes the repo is `./DD4hep`

set -e

[[ $# -gt 0 ]] && clean=1 || clean=0 # clean build if any args
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi

pushd DD4hep

if [ $clean -eq 1 ]; then
  echo "--- CLEAN: clean build dir..."
  mkdir -p build
  rm -rv build
  mkdir -p install
  rm -rv install
fi

cmake -B build -S . \
  -D CMAKE_INSTALL_PREFIX=install \
  -D DD4HEP_USE_GEANT4=ON         \
  -D DD4HEP_USE_EDM4HEP=ON        \
  -D DD4HEP_USE_HEPMC3=ON         \
  -D Boost_NO_BOOST_CMAKE=ON      \
  -D DD4HEP_USE_LCIO=ON           \
  -D BUILD_TESTING=ON             \
  -D ROOT_DIR=$ROOTSYS            \
  -D CMAKE_BUILD_TYPE=Release
cmake --build build -j$BUILD_NPROC -- install

popd

printf "\nDone. To use, run:  source scripts/this_DD4hep.sh\n\n"
