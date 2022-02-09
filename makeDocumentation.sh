#!/bin/bash
# build documentation from compact file `documentation` tags
source environ.sh
mkdir -p doc
docfile=$(pwd)/doc/detector.md
pushd $DRICH_DD4_ATHENA
bin/build_documentation | tee $docfile
popd
