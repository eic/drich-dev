#!/bin/bash
# rebuild all modules, in order of dependence
# pass an argument to rebuild everything cleanly
set -e
./build.sh EDM4eic $*
./build.sh irt     $*
./build.sh epic    $*
./build.sh juggler $*
