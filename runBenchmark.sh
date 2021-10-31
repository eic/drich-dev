#!/bin/bash
# run benchmark
pushd benchmarks
#benchmarks/rich/run_irt.sh
benchmarks/rich/run_irt.sh basic 1 -t4 -d-1 -ppi+ -e20 -n1
#benchmarks/rich/run_irt.sh basic2 1 -t4 -d1 -ppi+ -e20 -n10
#benchmarks/rich/run_irt.sh basic 2 -t4 -d-1 -ppi+ -e20 -n10
popd
