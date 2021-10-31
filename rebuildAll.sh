#!/bin/bash
# rebuild all repos
irt/bin/buildIRT.sh
./buildEICD.sh
./buildJuggler.sh
