#!/bin/bash
# rebuild all modules, in order of dependence
# pass an argument to rebuild everything cleanly
set -e
./build_eicd.sh $*
./build_irt.sh $* # temporarily disabled
./build_ip6.sh $*
./build_epic.sh $*
