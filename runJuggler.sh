#!/bin/bash
# run juggler

option=JugPID/options/testIRT.py
#option=JugPID/options/rich_reco.py

source environ.sh
pushd juggler
xenv -x $JUGGLER_INSTALL_PREFIX/Juggler.xenv gaudirun.py $option
popd
