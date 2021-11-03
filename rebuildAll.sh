#!/bin/bash
# rebuild all repos
set -e
./buildATHENA.sh
irt/bin/buildIRT.sh
./buildEICD.sh
./buildJuggler.sh
