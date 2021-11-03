#!/bin/bash
# rebuild all repos
set -e
./buildIP6.sh
./buildATHENA.sh
irt/bin/buildIRT.sh
./buildEICD.sh
./buildJuggler.sh
