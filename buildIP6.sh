#!/bin/bash

if [ $# -ge 1 ]; then clean=1; else clean=0; fi

pushd ip6

if [ $clean -eq 1 ]; then
  echo "clean build dir... done"
  mkdir -p build
  rm -rv build
fi

cmake -B build -S . \
  -DCMAKE_INSTALL_PREFIX=$ATHENA_PREFIX \
  -DCMAKE_CXX_STANDARD=17 \
  &&\
cmake --build build -j$(grep -c processor /proc/cpuinfo) -- install &&\
popd &&\
exit 0

popd
exit 1
