#!/bin/bash
# create IRT geometry config file
outFile=geo/irt-drich.root
python ecce/scripts/create_IRT_auxfile.py -o $outFile | grep IRTLOG
echo ""
echo "produced $outFile"
echo ""
