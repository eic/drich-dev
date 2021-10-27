#!/bin/bash
# build EICD
pushd eicd
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=$ATHENA_PREFIX -DBUILD_DATA_MODEL=ON && \
cmake --build build -j2 -- install && \
popd && exit 0
popd
exit 1

