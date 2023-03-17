#!/bin/bash

set -e

prefix=$(pwd)/prefix2
ncpu=4

mkdir -p $prefix

# override with local builds
export CMAKE_PREFIX_PATH=$prefix/install:$CMAKE_PREFIX_PATH

# build IRT
cmake -S irt -B $prefix/build_irt -DCMAKE_INSTALL_PREFIX=$prefix/install
cmake --build $prefix/build_irt -j$ncpu
cmake --install $prefix/build_irt

echo ""
echo "========================================"
echo "========================================"
echo "========================================"
echo ""

# build EICrecon
cmake -S EICrecon -B $prefix/build_eicrecon -DCMAKE_INSTALL_PREFIX=$prefix/install
cmake --build $prefix/build_eicrecon -j$ncpu
cmake --install $prefix/build_eicrecon
