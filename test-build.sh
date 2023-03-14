#!/bin/bash

set -e
mkdir -p prefix2

# build IRT
cmake -S irt -B prefix2/build_irt -DCMAKE_INSTALL_PREFIX=prefix2/install
cmake --build prefix2/build_irt -j8
cmake --install prefix2/build_irt

# overrides container version with local version
export IRT_ROOT=$pwd/prefix2/install

# build EICrecon
cmake -S EICrecon -B prefix2/build_eicrecon -DCMAKE_INSTALL_PREFIX=prefix2/install
cmake --build prefix2/build_eicrecon -j8
cmake --install prefix2/build_eicrecon
