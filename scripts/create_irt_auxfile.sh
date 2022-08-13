#!/bin/bash
# - create IRT geometry config file
# - wraps src/create_irt_auxfile.py, filtering verbose `stdout` for IRT-specific printouts
outFile=geo/irt-drich.root
scripts/src/create_irt_auxfile.py -o $outFile | grep IRTLOG
echo ""
echo "produced $outFile"
echo ""
