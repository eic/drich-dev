#!/bin/bash
set -e

# set compact file, depending on (optional) argument; default is full epic
compactFile=$(
  case "$1" in
    ("d") echo "epic_drich_only.xml"  ;;
    ("p") echo "epic_pfrich_only.xml" ;;
    (*)   echo "epic.xml"             ;;
  esac)
echo "compactFile = $compactFile"

# produce geometry root file
wdir=$(pwd)/geo
mkdir -p $wdir
pushd epic
dd_web_display --export -o $wdir/detector_geometry.root $compactFile
popd
echo ""
echo "produced $(ls -t $wdir/*.root|head -n1)"
echo " -> open it with jsroot to view the geometry"
echo ""
