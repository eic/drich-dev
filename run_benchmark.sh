#!/bin/bash
### run benchmark
pushd reconstruction_benchmarks

### print usage for run_irt.sh, for guidance on its arguments
benchmarks/rich/run_irt.sh
echo "========================================================"

### full detector
############################################################################
# benchmarks/rich/run_irt.sh basic_pfrich 1 -t8 -d-1 -ppi+ -e20 -n3
# benchmarks/rich/run_irt.sh basic_drich  1 -t8 -d1  -ppi+ -e20 -n3
############################################################################

### proposal plots
############################################################################
# benchmarks/rich/run_irt.sh basic_pfrich 1 -t8 -d-1 -pe-  -e20 -n100 -s
# benchmarks/rich/run_irt.sh basic_drich  1 -t8 -d1  -ppi+ -e20 -n100 -s
############################################################################

### quick tests
mode=0 # npsim + juggler
# mode=1 # npsim only
# mode=2 # juggler only
############################################################################
# benchmarks/rich/run_irt.sh basic_pfrich $mode -t1 -d-1 -pe-  -e20 -n50 -s
benchmarks/rich/run_irt.sh basic_drich  $mode -t1 -d1  -ppi+ -e20 -n50 -s
############################################################################

popd
