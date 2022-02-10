#!/bin/bash
# run benchmark
pushd benchmarks

# print usage
#benchmarks/rich/run_irt.sh

# full athena
#benchmarks/rich/run_irt.sh basic_pfrich 1 -t8 -d-1 -ppi+ -e20 -n3
#benchmarks/rich/run_irt.sh basic_drich 1 -t8 -d1 -ppi+ -e20 -n3

# proposal plots
#benchmarks/rich/run_irt.sh basic_pfrich 1 -t8 -d-1 -pe- -e20 -n100 -s
benchmarks/rich/run_irt.sh basic_drich 1 -t8 -d1 -ppi+ -e20 -n100 -s
popd
