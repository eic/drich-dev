#!/bin/bash
# rebuild all modules, in order of dependence
# pass an argument to rebuild everything cleanly
set -e
./build_EDM4eic.sh $*
./build_irt.sh $*
./build_epic.sh $*
./build_juggler.sh $*
