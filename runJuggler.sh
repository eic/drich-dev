#!/bin/bash
# run juggler
source environ.sh
pushd juggler
xenv -x $JUGGLER_INSTALL_PREFIX/Juggler.xenv \
  gaudirun.py JugPID/options/testIRT.py
popd
