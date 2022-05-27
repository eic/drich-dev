#!/bin/bash
# build documentation from compact file `documentation` tags
source environ.sh
mkdir -p doc
docfile=$(pwd)/doc/detector.md
pushd athena
bin/build_documentation | tee $docfile
popd
