#!/bin/bash
# run juggler

option=benchmarks/rich/options/testIRT.py
#option=benchmarks/rich/options/rich_reco.py

pushd benchmarks
xenv -x $JUGGLER_INSTALL_PREFIX/Juggler.xenv gaudirun.py $option
popd
