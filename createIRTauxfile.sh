#!/bin/bash
# - create IRT geometry config file
# - wraps scripts/createIRTauxfile.py, filtering verbose `stdout` for IRT-specific printouts
outFile=geo/irt-drich.root
scripts/createIRTauxfile.py -o $outFile | grep IRTLOG
echo ""
echo "produced $outFile"
echo ""
