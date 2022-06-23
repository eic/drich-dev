#!/bin/bash
set -e

# set compact file, depending on (optional) argument; default is full ecce
compactFile=$(
  case "$1" in
    ("d") echo "ecce_drich_only.xml"  ;;
    ("p") echo "ecce_pfrich_only.xml" ;;
    (*)   echo "ecce.xml"             ;;
  esac)
echo "compactFile = $compactFile"

# produce geometry root file
wdir=$(pwd)/geo
mkdir -p $wdir
pushd ecce
dd_web_display --export -o $wdir/detector_geometry.root $compactFile
popd
echo ""
echo "produced $(ls -t $wdir/*.root|head -n1)"
echo " -> open it with jsroot to view the geometry"
echo ""
