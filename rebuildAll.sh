#!/bin/bash
# rebuild all modules, in order of dependence
# pass an argument to rebuild everything cleanly
set -e
./buildEICD.sh $*
./buildIRT.sh $*
./buildIP6.sh $*
./buildECCE.sh $*
./buildJuggler.sh $*
