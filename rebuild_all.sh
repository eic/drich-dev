#!/bin/bash
# rebuild all modules, in order of dependence
# pass an argument to rebuild everything cleanly
set -e
./build.sh EDM4eic $*
./build.sh irt $*
./build.sh epic $*
source environ.sh
./build.sh EICrecon $*
./build.sh reconstruction_benchmarks $*
